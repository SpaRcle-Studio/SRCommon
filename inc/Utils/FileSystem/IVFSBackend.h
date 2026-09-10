//
// Created by Monika on 10.09.2026.
//

#ifndef SR_ENGINE_COMMON_I_VFS_BACKEND_H
#define SR_ENGINE_COMMON_I_VFS_BACKEND_H

#include <Utils/Common/PassKey.h>
#include <Utils/Types/String.h>
#include <Utils/Types/Set.h>
#include <Utils/FileSystem/FileMode.h>

namespace SR_UTILS_NS {
    class File;
    class VFS;

    struct VFSEntry {
        StringView name;
        StringView extension;
        StringView fullPath;
        StringView relativePath;
        FSItemType type = FSItemType::Undefined;
    };

    struct VFSMount;

    class IVFSBackend : public NonCopyable {
    public:
        IVFSBackend() = default;
        IVFSBackend(StringView realPath)
            : m_realPath(realPath)
        { }

    public:
        using EnumerateCallback = SR_HTYPES_NS::Function<void(const VFSEntry&)>;

        SR_NODISCARD virtual bool ReadSupports() const = 0;
        SR_NODISCARD virtual bool WriteSupports() const = 0;
        SR_NODISCARD virtual FSItemType GetType(StringView path) const = 0;
        SR_NODISCARD virtual File OpenFile(StringView path, FileMode mode) const = 0;

        virtual void Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const = 0;
        virtual void ResolveVirtualPath(StringView virtualPath, String& outRealPath) const = 0;
        virtual void Delete(StringView path) const = 0;

        SR_NODISCARD virtual bool IsApplicable(StringView path) const;

        void SetVirtualPath(PassKey<VFS>, StringView virtualPath) { m_virtualPath = virtualPath; }
        SR_NODISCARD StringView GetVirtualPath() const { return m_virtualPath; }

        SR_NODISCARD StringView GetRealPath() const { return m_realPath; }

        void AddIgnoredExtension(StringView extension) { m_ignoredExtensions.insert(extension); }

    protected:
        String m_virtualPath;
        String m_realPath;
        Set<String> m_ignoredExtensions;

    };
}

#endif //SR_ENGINE_COMMON_I_VFS_BACKEND_H
