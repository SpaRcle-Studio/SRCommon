//
// Created by Monika on 02.03.2026.
//

#ifndef SR_ENGINE_COMMON_VIRTUAL_FS_H
#define SR_ENGINE_COMMON_VIRTUAL_FS_H

#include <Utils/Common/Singleton.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Types/FlatHashMap.h>

namespace SR_UTILS_NS {
    class VirtualFS : public Singleton<VirtualFS> {
        SR_REGISTER_SINGLETON(VirtualFS);
    public:
        SR_NODISCARD bool IsFileExist(const Path& path) const;
        SR_NODISCARD bool TryReadFile(const Path& path, std::string& buffer) const;
        SR_NODISCARD bool WriteFile(const Path& path, std::string_view data);

    private:
        SR_HTYPES_NS::FlatHashMap<Path, std::string> m_files;

    };
}

#endif //SR_ENGINE_COMMON_VIRTUAL_FS_H
