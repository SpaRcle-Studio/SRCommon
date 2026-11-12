#include <Utils/Platform/AndroidSDK.h>
#include <Utils/Common/StringUtils.h>
#include <Utils/FileSystem/VFS.h>

namespace SR_UTILS_NS::Platform {
    bool AndroidSDK::CheckPackage(const Path& apk, StringView package) {
        const Path aapt = GetSDKPath().Concat("build-tools/35.0.0").Concat(GetExecutableName("aapt"));
        String metadata;
        if (!Command(aapt, {"dump", "badging", apk.ToString()}, &metadata)) {
            return false;
        }
        const String expected = "package: name='{}'"_format(package);
        if (!metadata.contains(expected)) {
            Message("Repacked APK package ID does not match the active build profile.", true);
            return false;
        }
        return true;
    }

    namespace {
        String TrimSDKField(StringView field) {
            while (!field.empty() && (field.front() == ' ' || field.front() == '\t' || field.front() == '\r')) {
                field.remove_prefix(1);
            }
            while (!field.empty() && (field.back() == ' ' || field.back() == '\t' || field.back() == '\r')) {
                field.remove_suffix(1);
            }
            return String(field);
        }
    }

    bool AndroidSDK::ListSystemImages(Vector<SystemImage>& images) {
        images.clear();
        String output;
        if (!JavaTool("sdkmanager", {"--sdk_root={}"_format(GetSDKPath()), "--list", "--channel=0"}, 180)) {
            return false;
        }
        // The package catalogue can exceed the bounded diagnostic log.
        auto file = VFS::Instance().OpenFile(GetLogPath(), FileMode::Read);
        if (!file || !file.Read(output)) {
            return false;
        }
        Vector<StringView> lines;
        StringUtils::Instance().SplitView(output, "\n", lines);
        for (const auto& line : lines) {
            const auto packageEnd = line.find('|');
            const String package = TrimSDKField(line.substr(0, packageEnd));
            if (!package.starts_with("system-images;") || !package.ends_with(";x86_64")) {
                continue;
            }
            bool duplicate = false;
            for (const auto& image : images) {
                if (image.package == package) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                continue;
            }
            const auto revisionEnd = packageEnd == StringView::npos ? StringView::npos : line.find('|', packageEnd + 1);
            const auto descriptionEnd = revisionEnd == StringView::npos ? StringView::npos : line.find('|', revisionEnd + 1);
            String description = package;
            if (revisionEnd != StringView::npos) {
                description = TrimSDKField(line.substr(revisionEnd + 1,
                    descriptionEnd == StringView::npos ? StringView::npos : descriptionEnd - revisionEnd - 1));
            }
            images.emplace_back(SystemImage{package, description, IsSystemImageInstalled(package)});
        }
        std::sort(images.begin(), images.end(), [](const SystemImage& left, const SystemImage& right) {
            return left.package < right.package;
        });
        Message(images.empty() ? "The SDK returned no compatible x86_64 system images." : "Android system image list updated.", images.empty());
        return !images.empty();
    }

    bool AndroidSDK::GetDevices(Vector<Device>& devices) {
        String output;
        if (!Command(GetADBPath(), {"devices", "-l"}, &output)) {
            return false;
        }
        devices.clear();
        Vector<StringView> lines;
        StringUtils::Instance().SplitView(output, "\n", lines);
        for (const auto& line : lines) {
            if (line.empty() || line.starts_with("List ") || line.starts_with('*')) {
                continue;
            }
            const auto serialEnd = String(line).find_first_of("\t ");
            if (serialEnd == StringView::npos) {
                continue;
            }
            SizeType stateStart = serialEnd;
            while (stateStart < line.size() && (line[stateStart] == ' ' || line[stateStart] == '\t')) {
                ++stateStart;
            }
            const auto stateEnd = String(line).find_first_of(" \r\t", stateStart);
            devices.emplace_back(Device{String(line.substr(0, serialEnd)),
                String(line.substr(stateStart, stateEnd == StringView::npos ? StringView::npos : stateEnd - stateStart)), String(line)});
        }
        Message(devices.empty() ? "No Android devices. Enable USB debugging, or start the emulator." : "Device list updated.");
        return true;
    }

    bool AndroidSDK::StartEmulator(Process& process, StringView image) {
        if (!HasEmulator(image)) {
            Message("Install the selected system image in Tools first.", true);
            return false;
        }
        const Path executable = GetSDKPath().Concat("emulator").Concat(GetExecutableName("emulator"));
        if (!Command(executable, {"-accel-check"}, nullptr, 30)) {
            Message("Hardware acceleration is unavailable. Configure the host hypervisor; see runtime log.", true);
            return false;
        }
        ProcessInfo info;
        info.executable = executable;
        info.arguments = {"-avd", GetVirtualDeviceName(image), "-port", "5580", "-no-snapshot-load"};
        info.environment = Environment();
        info.output = m_root.Concat("emulator.log");
        info.ownProcessTree = true;
        if (!process.Start(std::move(info))) {
            Message("Cannot start Android emulator. See emulator.log.", true);
            return false;
        }
        Message("Waiting for Android emulator to boot...");
        const auto started = std::chrono::steady_clock::now();
        while ((!isCancelled || !isCancelled()) && process.IsRunning() &&
            std::chrono::steady_clock::now() - started < std::chrono::minutes(3)) {
            String avd;
            String boot;
            if (Command(GetADBPath(), {"-s", "emulator-5580", "emu", "avd", "name"}, &avd, 15, {}, true) &&
                avd.starts_with(GetVirtualDeviceName(image)) &&
                Command(GetADBPath(), {"-s", "emulator-5580", "shell", "getprop", "sys.boot_completed"}, &boot, 15, {}, true) && boot.starts_with("1")) {
                Message("Emulator ready. Select it in the device list and run the game.");
                return true;
            }
            SR_HTYPES_NS::Thread::Sleep(1000);
        }
        Message("Emulator did not finish booting. See emulator.log.", true);
        return false;
    }

    bool AndroidSDK::InstallAndRun(StringView serial, const Path& arm64Apk, const Path& x64Apk) {
        if (serial.empty()) {
            Message("Select an authorized Android device first.", true);
            return false;
        }
        const String device(serial);
        Vector<Device> devices;
        if (!GetDevices(devices)) {
            return false;
        }
        bool ready = false;
        for (const auto& candidate : devices) {
            if (candidate.serial == device) {
                if (candidate.state != "device") {
                    Message("Android device {} is {}. Reconnect it and authorize USB debugging, then refresh devices."_format(device, candidate.state), true);
                    return false;
                }
                ready = true;
                break;
            }
        }
        if (!ready) {
            Message("Android device {} is disconnected. Refresh devices and select a connected device."_format(device), true);
            return false;
        }
        String abi;
        if (!Command(GetADBPath(), {"-s", device, "shell", "getprop", "ro.product.cpu.abilist"}, &abi)) {
            return false;
        }
        const Path apk = abi.contains("x86_64") ? x64Apk : abi.contains("arm64-v8a") ? arm64Apk : Path();
        if (apk.empty() || !apk.IsFile()) {
            Message("No APK matching the device ABI (arm64-v8a or x86_64).", true);
            return false;
        }
        const Path aapt = GetSDKPath().Concat("build-tools/35.0.0").Concat(GetExecutableName("aapt"));
        String metadata;
        if (!Command(aapt, {"dump", "badging", apk.ToString()}, &metadata)) {
            return false;
        }
        const auto nameBegin = metadata.find("package: name='");
        if (nameBegin == String::npos) {
            Message("Cannot read APK package ID.", true);
            return false;
        }
        const auto packageStart = nameBegin + StringView("package: name='").size();
        const auto packageEnd = metadata.find('\'', packageStart);
        if (packageEnd == String::npos) {
            Message("Invalid APK package metadata.", true);
            return false;
        }
        const String package = metadata.substr(packageStart, packageEnd - packageStart);
        m_lastPackage = package;
        Message("Installing APK on {}..."_format(device));
        if (!Command(GetADBPath(), {"-s", device, "install", "-r", apk.ToString()}, nullptr, 180)) {
            return false;
        }
        String activity;
        if (!Command(GetADBPath(), {"-s", device, "shell", "cmd", "package", "resolve-activity", "--brief",
            "-a", "android.intent.action.MAIN", "-c", "android.intent.category.LAUNCHER", package}, &activity)) {
            return false;
        }
        String component;
        Vector<StringView> lines;
        StringUtils::Instance().SplitView(activity, "\n", lines);
        for (const auto& line : lines) {
            const String candidate = TrimSDKField(line);
            if (candidate.starts_with(package + "/")) {
                component = candidate;
            }
        }
        if (component.empty()) {
            Message("APK has no resolved launcher activity.", true);
            return false;
        }
        String output;
        if (!Command(GetADBPath(), {"-s", device, "shell", "am", "start", "-W", "-n", component}, &output)) {
            return false;
        }
        if (output.contains("Error:") || output.contains("Exception")) {
            Message("Android activity could not be started. See runtime log.", true);
            return false;
        }
        Message("Android application started on {}."_format(device));
        return true;
    }
}
