//
// Created by Monika on 02.03.2026.
//

#include <Utils/FileSystem/VirtualFS.h>
#include <Utils/Types/LockGuard.h>

namespace SR_UTILS_NS {
    bool VirtualFS::IsFileExist(const Path& path) const {
        SR_LOCK_GUARD;
        return m_files.contains(path);
    }

    bool VirtualFS::TryReadFile(const Path& path, std::string& buffer) const {
        SR_LOCK_GUARD;
        auto it = m_files.find(path);
        if (it != m_files.end()) {
            buffer = it->second;
            return true;
        }
        return false;
    }

    bool VirtualFS::WriteFile(const Path& path, std::string_view data) {
        SR_LOCK_GUARD;
        m_files[path] = data;
        return true;
    }
}