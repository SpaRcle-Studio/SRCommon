//
// Created by Monika on 10.09.2026.
//

#ifndef SR_ENGINE_COMMON_ANDROID_VFS_BACKEND_H
#define SR_ENGINE_COMMON_ANDROID_VFS_BACKEND_H

#include <Utils/FileSystem/IVFSBackend.h>

namespace SR_UTILS_NS {
    /// Бекенд для доступа к ресурсам, упакованным в apk (папка assets).
    /// Ресурсы упаковываются вместе с папкой "Resources", поэтому пути внутри assets полностью
    /// совпадают с виртуальными путями движка и никакого особого префикса не требуется.
    /// Бекенд монтируется в корень, так же как папка приложения на остальных платформах.
    class AndroidVFSBackend : public IVFSBackend {
    public:
        AndroidVFSBackend() = default;

    public:
        SR_NODISCARD bool ReadSupports() const override { return true; }
        SR_NODISCARD bool WriteSupports() const override { return false; }
        SR_NODISCARD FSItemType GetType(StringView path) const override;
        SR_NODISCARD File OpenFile(StringView path, FileMode mode) const override;

        void Delete(StringView path) const override;
        void Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const override;
        void ResolveVirtualPath(StringView virtualPath, String& outRealPath) const override;

    private:
        /// Преобразует виртуальный путь в путь относительно корня assets (без ведущего слеша).
        void ResolveAssetPath(StringView virtualPath, String& outAssetPath) const;

    };
}

#endif //SR_ENGINE_COMMON_ANDROID_VFS_BACKEND_H
