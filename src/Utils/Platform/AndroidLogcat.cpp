//
// Created by OpenAI Codex on 12.11.2026.
//

#include <Utils/Platform/AndroidSDK.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Common/StringUtils.h>

namespace SR_UTILS_NS::Platform {
    bool AndroidSDK::ReadLogcat(StringView serial, StringView package, StringView priority, StringView buffer, String& output) {
        if (serial.empty()) {
            output = "Select an Android device in the Run tab.";
            return false;
        }
        Vector<String> arguments = {"-s", String(serial), "logcat", "-d", "-v", "threadtime", "-t", "1000"};
        if (buffer == "crash") {
            arguments.emplace_back("-b");
            arguments.emplace_back("crash");
        }
        else {
            for (const char* name : {"main", "system", "crash"}) {
                arguments.emplace_back("-b");
                arguments.emplace_back(name);
            }
        }
        if (!package.empty()) {
            String packages;
            if (!Command(GetADBPath(), {"-s", String(serial), "shell", "cmd", "package", "list", "packages", "-U", String(package)},
                &packages, 10, {}, true)) {
                output = "Cannot query application UID. Check the device connection and USB authorization.";
                return false;
            }
            const String prefix = "package:{} uid:"_format(package);
            const auto found = packages.find(prefix);
            if (found == String::npos) {
                output = "Application package not found on this device. Enter the package ID or disable the application filter.";
                return false;
            }
            const auto begin = found + prefix.size();
            auto end = begin;
            while (end < packages.size() && packages[end] >= '0' && packages[end] <= '9') {
                ++end;
            }
            if (end == begin) {
                output = "Cannot read application UID.";
                return false;
            }
            arguments.emplace_back("--uid={}"_format(packages.substr(begin, end - begin)));
        }
        arguments.emplace_back("*:{}"_format(priority));
        if (!Command(GetADBPath(), arguments, &output, 15, {}, true)) {
            return false;
        }
        String records;
        Vector<StringView> lines;
        StringUtils::Instance().SplitView(output, "\n", lines);
        for (auto line : lines) {
            if (!line.empty() && line.back() == '\r') {
                line.remove_suffix(1);
            }
            if (line.empty() || line.starts_with("--------- beginning of ") || line == "(null)") {
                continue;
            }
            records += line;
            records += '\n';
        }
        output = std::move(records);
        if (output.empty()) {
            return true;
        }
        if (!FileSystem::WriteToFile(m_root.Concat("logcat.txt").ToStringView(), output)) {
            output = "Cannot save the logcat snapshot to RuntimeTools/logcat.txt.";
            return false;
        }
        return true;
    }

    bool AndroidSDK::ClearLogcat(StringView serial) {
        return !serial.empty() && Command(GetADBPath(), {"-s", String(serial), "logcat", "-b", "main", "-b", "system", "-b", "crash", "-c"});
    }
}
