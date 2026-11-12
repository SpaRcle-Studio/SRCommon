#include <Utils/Platform/AndroidSDK.h>
#include <Utils/Platform/Platform.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/FileSystem/ArchiveExtract.h>
#include <Utils/Network/Download.h>
#include <Utils/Common/Hashes.h>

namespace SR_UTILS_NS::Platform {
    AndroidSDK::AndroidSDK(Path root)
        : m_root(std::move(root))
    { }

    Path AndroidSDK::GetSDKPath() const {
        return m_root.Concat("Android/sdk");
    }

    Path AndroidSDK::GetHomePath() const {
        return m_root.Concat("Android/home");
    }

    Path AndroidSDK::GetJavaPath(StringView tool) const {
        return m_root.Concat("Java/current/bin").Concat(GetExecutableName(tool));
    }

    Path AndroidSDK::GetADBPath() const {
        return GetSDKPath().Concat("platform-tools").Concat(GetExecutableName("adb"));
    }

    Path AndroidSDK::GetLogPath() const {
        return m_root.Concat("runtime.log");
    }

    String AndroidSDK::GetSelectedImage() const {
        String result;
        FileSystem::ReadFile(m_root.Concat("Android/system-image.txt"), result);
        return result;
    }

    bool AndroidSDK::HasTools() const {
        return GetADBPath().IsFile() && GetJavaPath("java").IsFile() &&
            GetSDKPath().Concat("build-tools/35.0.0/lib/apksigner.jar").IsFile();
    }

    String AndroidSDK::GetVirtualDeviceName(StringView image) const {
        return "SREngine_{:016x}"_format(HashArrayRepresentation(image.data(), image.size()));
    }

    Path AndroidSDK::GetVirtualDevicePath(StringView image) const {
        return GetHomePath().Concat("avd").Concat("{}.avd"_format(GetVirtualDeviceName(image)));
    }

    bool AndroidSDK::HasEmulator(StringView image) const {
        return !image.empty() && GetHomePath().Concat("avd").Concat("{}.ini"_format(GetVirtualDeviceName(image))).IsFile() &&
            GetVirtualDevicePath(image).Concat("config.ini").IsFile() &&
            GetSDKPath().Concat("emulator").Concat(GetExecutableName("emulator")).IsFile();
    }

    bool AndroidSDK::IsSystemImageInstalled(StringView image) const {
        String relative(image);
        for (auto& value : relative) {
            if (value == ';') {
                value = '/';
            }
        }
        return GetSDKPath().Concat(relative).Concat("package.xml").IsFile();
    }

    void AndroidSDK::Message(String text, bool error) const {
        if (onMessage) {
            onMessage(std::move(text), error);
        }
    }

    Vector<String> AndroidSDK::Environment() const {
        return {
            "ANDROID_HOME={}"_format(GetSDKPath()),
            "ANDROID_USER_HOME={}"_format(GetHomePath()),
            "ANDROID_AVD_HOME={}"_format(GetHomePath().Concat("avd")),
            "JAVA_HOME={}"_format(m_root.Concat("Java/current"))
        };
    }

    bool AndroidSDK::Command(const Path& program, const Vector<String>& arguments, String* output,
                            uint32_t timeoutSeconds, StringView input, bool quiet) {
        Process process;
        ProcessInfo info;
        info.executable = program;
        info.arguments = arguments;
        info.environment = Environment();
        const Path commandLog = quiet ? m_root.Concat("runtime-poll.log") : GetLogPath();
        info.output = commandLog;
        info.input = input;
        info.backgroundOutput = !quiet;
        if (!process.Start(std::move(info))) {
            if (!quiet) {
                Message("Cannot start {}. See {}."_format(program, GetLogPath()), true);
            }
            return false;
        }
        const auto started = std::chrono::steady_clock::now();
        if (!quiet && onProgress) {
            onProgress(-1.f);
        }
        uint32_t ticks = 0;
        while (process.IsRunning()) {
            if ((isCancelled && isCancelled()) || std::chrono::steady_clock::now() - started > std::chrono::seconds(timeoutSeconds)) {
                process.Stop();
                if (!quiet) {
                    Message("Command cancelled or timed out. See runtime log.", true);
                }
                return false;
            }
            if (++ticks % 5 == 0 && !quiet && onCommandOutput) {
                const String text = ReadCommandOutput(commandLog);
                const auto percent = text.rfind('%');
                if (onProgress && percent != String::npos) {
                    SizeType begin = percent;
                    while (begin > 0 && ((text[begin - 1] >= '0' && text[begin - 1] <= '9') || text[begin - 1] == '.')) {
                        --begin;
                    }
                    if (begin < percent) {
                        const String value = text.substr(begin, percent - begin);
                        const float_t fraction = strtof(value.c_str(), nullptr) / 100.f;
                        if (fraction >= 0.f && fraction <= 1.f) {
                            onProgress(fraction);
                        }
                    }
                }
                onCommandOutput(text);
            }
            SR_HTYPES_NS::Thread::Sleep(100);
        }
        process.FinishOutput();
        String text = ReadCommandOutput(commandLog);
        if (output) {
            *output = text;
        }
        if (!quiet && onCommandOutput) {
            onCommandOutput(std::move(text));
        }
        if (process.GetExitCode() != 0 || process.HasOutputError()) {
            if (!quiet) {
                Message("Command failed (exit {}). See runtime log."_format(process.GetExitCode()), true);
            }
            return false;
        }
        return true;
    }

    String AndroidSDK::ReadCommandOutput(const Path& path) const {
        String text;
        auto file = VFS::Instance().OpenFile(path, FileMode::Read);
        if (file) {
            if (file.GetSize() > 128 * 1024) {
                file.Seek(-128 * 1024, SeekOrigin::End);
            }
            char buffer[16384];
            while (const uint64_t count = file.Read(buffer, sizeof(buffer))) {
                text.append(buffer, count);
            }
        }
        return text;
    }

    bool AndroidSDK::JavaTool(StringView tool, const Vector<String>& arguments, uint32_t timeout, StringView input) {
        const Path classpath = GetSDKPath().Concat("cmdline-tools/latest/lib").Concat("{}-classpath.jar"_format(tool));
        const StringView toolsProperty = tool == "sdkmanager" ? "com.android.sdklib.toolsdir" : "com.android.sdkmanager.toolsdir";
        Vector<String> javaArguments = {"-D{}={}"_format(toolsProperty, classpath.GetPrevious().GetPrevious()),
            "-cp", classpath.ToString(), tool == "sdkmanager"
                ? "com.android.sdklib.tool.sdkmanager.SdkManagerCli" : "com.android.sdklib.tool.AvdManagerCli"};
        for (const auto& argument : arguments) {
            javaArguments.emplace_back(argument);
        }
        return Command(GetJavaPath("java"), javaArguments, nullptr, timeout, input);
    }

    bool AndroidSDK::Download(StringView url, const Path& destination) {
        SR_NETWORK_NS::DownloadRequest request;
        request.url = url;
        request.destination = destination;
        request.isCancelled = isCancelled;
        request.transferProgress = onDownloadProgress;
        request.progress = [this, destination](uint64_t bytes) {
            Message("Downloading {}... {} MB"_format(destination.GetBaseNameView(), bytes / (1024 * 1024)));
        };
        if (!SR_NETWORK_NS::Download(request)) {
            Message("Download failed or cancelled. See download log.", true);
            return false;
        }
        return true;
    }

    bool AndroidSDK::Install(bool withEmulator, bool acceptedLicense, StringView image) {
        const auto host = GetType();
        if ((host != PlatformType::Windows && host != PlatformType::Linux) || GetProcessArchitecture() != "x64") {
            Message("Managed Android tooling requires a Windows or Linux x86_64 editor.", true);
            return false;
        }
        if (!acceptedLicense) {
            Message("Read and accept the Android SDK terms before installation.", true);
            return false;
        }
        if (withEmulator && (!image.starts_with("system-images;android-") || !image.ends_with(";x86_64"))) {
            Message("Select a compatible x86_64 system image from the SDK package list.", true);
            return false;
        }
        auto&& vfs = VFS::Instance();
        if (!vfs.CreateDirectories(GetHomePath().Concat("avd").ToStringView())) {
            return false;
        }
        const bool windows = host == PlatformType::Windows;
        const Path downloads = m_root.Concat("Downloads");
        const Path javaReady = m_root.Concat("Java/current/.installed");
        if (!javaReady.IsFile() || !Command(GetJavaPath("java"), {"-version"}, nullptr, 30)) {
            const String os = windows ? "windows" : "linux";
            const Path archive = downloads.Concat(windows ? "jdk-25-windows.zip" : "jdk-25-linux.tar.gz");
            if (!Download("https://api.adoptium.net/v3/binary/latest/25/ga/{}/x64/jdk/hotspot/normal/eclipse"_format(os), archive)) {
                return false;
            }
            const Path javaRoot = m_root.Concat("Java/current");
            Message("Extracting Java runtime...");
            if (windows) {
                if (!ExtractZip(archive, javaRoot, {.stripRoot = true})) {
                    Message("Failed to unpack Java runtime.", true);
                    return false;
                }
            }
            else {
                if (!ExtractTarGzip(archive, javaRoot, {.stripRoot = true})) {
                    Message("Failed to unpack Java runtime.", true);
                    return false;
                }
            }
            if (!Command(GetJavaPath("java"), {"-version"}, nullptr, 30) || !FileSystem::WriteToFile(javaReady.ToStringView(), "Temurin 25\n")) {
                return false;
            }
        }
        const Path toolsReady = GetSDKPath().Concat("cmdline-tools/latest/.installed");
        if (!toolsReady.IsFile()) {
            const String sdkHost = windows ? "win" : "linux";
            const Path archive = downloads.Concat("commandlinetools-{}-15859902.zip"_format(sdkHost));
            if (!Download("https://dl.google.com/android/repository/commandlinetools-{}-15859902_latest.zip"_format(sdkHost), archive)) {
                return false;
            }
            Message("Extracting Android command-line tools...");
            if (!ExtractZip(archive, toolsReady.GetPrevious(), {.stripPrefix = "cmdline-tools/"}) ||
                !JavaTool("sdkmanager", {"--version"}, 30) || !FileSystem::WriteToFile(toolsReady.ToStringView(), "15859902\n")) {
                Message("Failed to install Android command-line tools.", true);
                return false;
            }
        }
        Message("Installing Android tools. This can take several minutes...");
        Vector<String> packages = {"--sdk_root={}"_format(GetSDKPath()), "platform-tools", "build-tools;35.0.0"};
        if (withEmulator) {
            packages.emplace_back("emulator");
            packages.emplace_back(image);
        }
        String answers;
        for (uint32_t i = 0; i < 32; ++i) {
            answers += "y\n";
        }
        if (!JavaTool("sdkmanager", packages, 3600, answers)) {
            return false;
        }
        if (withEmulator) {
            if (!HasEmulator(image)) {
                Message("Creating Android virtual device...");
                if (!JavaTool("avdmanager", {"create", "avd", "--force", "--name", GetVirtualDeviceName(image),
                    "--package", String(image), "--path", GetVirtualDevicePath(image).ToString()}, 120, "no\n")) {
                    return false;
                }
            }
            if (!FileSystem::WriteToFile(m_root.Concat("Android/system-image.txt").ToStringView(), image)) {
                return false;
            }
        }
        if (!HasTools() || (withEmulator && !HasEmulator(image))) {
            Message("Android installation is incomplete. See runtime log.", true);
            return false;
        }
        Message(withEmulator ? "Android tools and selected virtual device are ready." : "Android tools are ready.");
        return true;
    }

    bool AndroidSDK::Sign(const Path& apk, const Path& keyStore) {
        const Path signer = GetSDKPath().Concat("build-tools/35.0.0/lib/apksigner.jar");
        if (!GetJavaPath("java").IsFile() || !signer.IsFile()) {
            Message("Install Android tools in the Tools tab before exporting APKs.", true);
            return false;
        }
        if (!keyStore.IsFile()) {
            if (!VFS::Instance().CreateDirectories(keyStore.GetPrevious().ToStringView()) ||
                !Command(GetJavaPath("keytool"), {"-genkeypair", "-noprompt", "-keystore", keyStore.ToString(),
                    "-storepass", "android", "-keypass", "android", "-alias", "androiddebugkey",
                    "-dname", "CN=Android Debug,O=Android,C=US", "-keyalg", "RSA", "-keysize", "2048", "-validity", "10000"})) {
                return false;
            }
        }
        const Path aligned = apk.ConcatExt("aligned");
        const Path aligner = GetSDKPath().Concat("build-tools/35.0.0").Concat(GetExecutableName("zipalign"));
        bool success = Command(aligner, {"-P", "16", "-f", "4", apk.ToString(), aligned.ToString()});
        if (success) {
            success = Command(aligner, {"-c", "-P", "16", "4", aligned.ToString()});
        }
        if (success) {
            success = Command(GetJavaPath("java"), {"-jar", signer.ToString(), "sign", "--ks", keyStore.ToString(),
                "--ks-key-alias", "androiddebugkey", "--ks-pass", "pass:android", "--key-pass", "pass:android",
                "--out", apk.ToString(), aligned.ToString()});
        }
        if (success) {
            success = Command(GetJavaPath("java"), {"-jar", signer.ToString(), "verify", "--verbose", apk.ToString()});
        }
        VFS::Instance().Delete(aligned.ToStringView());
        return success;
    }
}
