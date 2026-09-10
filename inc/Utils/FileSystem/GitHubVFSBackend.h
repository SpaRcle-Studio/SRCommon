//
// Created by Monika on 10.09.2026.
//

#ifndef SR_ENGINE_COMMON_GIT_HUB_VFS_BACKEND_H
#define SR_ENGINE_COMMON_GIT_HUB_VFS_BACKEND_H

#include <Utils/FileSystem/IVFSBackend.h>
#include <Utils/Types/FlatHashMap.h>

namespace SR_NETWORK_NS {
    class GitHubDownloader;
}

namespace SR_UTILS_NS {
    /// Бекенд, монтирующий репозиторий GitHub только на чтение. Дерево файлов запрашивается один
    /// раз через GitHubDownloader, содержимое файлов скачивается по требованию и кешируется в памяти.
    /// Реальные пути имеют вид ":github:/Engine/Shaders/...", где ":github:" - корень репозитория.
    class GitHubVFSBackend : public IVFSBackend {
        using Cache = SR_HTYPES_NS::FlatHashMap<String, String>;
    public:
        static constexpr StringView GITHUB_PREFIX = ":github:";

    public:
        /// url - ссылка на репозиторий, например "https://github.com/owner/repo".
        /// branch - ветка репозитория, по умолчанию используется ветка по умолчанию репозитория.
        explicit GitHubVFSBackend(StringView url, StringView branch = StringView());
        ~GitHubVFSBackend() override;

    public:
        SR_NODISCARD bool ReadSupports() const override { return true; }
        SR_NODISCARD bool WriteSupports() const override { return false; }
        SR_NODISCARD FSItemType GetType(StringView path) const override;
        SR_NODISCARD File OpenFile(StringView path, FileMode mode) const override;

        void Delete(StringView path) const override;
        void Enumerate(StringView directory, const EnumerateCallback& callback, bool recursive) const override;
        void ResolveVirtualPath(StringView virtualPath, String& outRealPath) const override;

        SR_NODISCARD StringView GetBranch() const { return m_branch; }

        /// Сбрасывает кеш скачанных файлов. Дерево репозитория остается прежним.
        void ClearCache();

    private:
        /// Преобразует виртуальный путь в путь относительно корня репозитория (без ведущего слеша).
        void ResolveRepositoryPath(StringView virtualPath, String& outRepositoryPath) const;

        SR_NODISCARD bool IsTreeLoaded() const;

    private:
        RawPointerHolder<SR_NETWORK_NS::GitHubDownloader> m_pDownloader;
        String m_branch;
        mutable Cache m_cache;

    };
}

#endif //SR_ENGINE_COMMON_GIT_HUB_VFS_BACKEND_H
