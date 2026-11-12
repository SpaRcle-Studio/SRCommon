//
// Created by OpenAI Codex on 12.11.2026.
//

#include <Utils/FileSystem/ArchiveExtract.h>
#include <Utils/FileSystem/Archive.h>
#include <Utils/FileSystem/VFS.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS {
    bool ExtractZip(const Path& source, const Path& destination, const ArchiveExtractOptions& options) {
        auto&& vfs = VFS::Instance();
        auto archive = Archive::Load(vfs.OpenFile(source, FileMode::Read));
        if (!archive || !vfs.CreateDirectories(destination.ToStringView())) {
            return false;
        }
        bool success = true;
        archive->Enumerate([&](const Archive::Entry& entry) {
            if (!success || entry.name.empty()) {
                return;
            }
            StringView name = entry.name;
            if (options.stripRoot) {
                const auto slash = name.find('/');
                if (slash == StringView::npos) {
                    return;
                }
                name.remove_prefix(slash + 1);
            }
            if (!options.stripPrefix.empty()) {
                if (!name.starts_with(options.stripPrefix)) {
                    return;
                }
                name.remove_prefix(options.stripPrefix.size());
            }
            if (name.empty()) {
                return;
            }
            if (name.starts_with('/') || name.contains("\\") || name.contains(":") || name.find('\0') != StringView::npos) {
                success = false;
                return;
            }
            SizeType component = 0;
            for (SizeType i = 0; i <= name.size(); ++i) {
                if (i == name.size() || name[i] == '/') {
                    if (name.substr(component, i - component) == "..") {
                        success = false;
                        return;
                    }
                    component = i + 1;
                }
            }
            const Path target = destination.Concat(name);
            if (entry.isDirectory) {
                success = vfs.CreateDirectories(target.ToStringView());
                return;
            }
            if (!vfs.CreateDirectories(target.GetPrevious().ToStringView())) {
                success = false;
                return;
            }
            auto file = vfs.OpenFile(target, FileMode::Write);
            success = file && file.Write(entry.data.data(), entry.data.size()) == entry.data.size();
            file = {};
            if (success && entry.isExecutable) {
                Platform::CopyPermissions(Platform::GetApplicationPath(), target);
            }
        });
        return success;
    }
}
