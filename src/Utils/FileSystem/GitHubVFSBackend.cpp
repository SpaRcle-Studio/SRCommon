//
// Created by Monika on 10.09.2026.
//

#include <Utils/FileSystem/GitHubVFSBackend.h>
#include <Utils/FileSystem/File.h>
#include <Utils/Network/GitHubDownloader.h>

namespace SR_UTILS_NS {
    GitHubVFSBackend::GitHubVFSBackend(StringView url, StringView branch)
        : IVFSBackend(GITHUB_PREFIX)
        , m_branch(branch)
    {
        m_pDownloader = new SR_NETWORK_NS::GitHubDownloader(url);

        if (!m_pDownloader->GetRepo().IsValid()) {
            SR_ERROR("GitHubVFSBackend::GitHubVFSBackend() : failed to parse repository url \"{}\"!", url);
            return;
        }

        if (m_branch.empty()) {
            m_branch = m_pDownloader->GetDefaultBranch();
        }

        SR_LOG("GitHubVFSBackend::GitHubVFSBackend() : mounting repository \"{}/{}\" (branch: {})...",
            m_pDownloader->GetRepo().owner, m_pDownloader->GetRepo().repo, m_branch);
    }

    GitHubVFSBackend::~GitHubVFSBackend() = default;

    bool GitHubVFSBackend::IsTreeLoaded() const {
        return m_pDownloader && !m_pDownloader->GetTree(m_branch).empty();
    }

    void GitHubVFSBackend::ClearCache() {
        m_cache.clear();
    }

    void GitHubVFSBackend::ResolveRepositoryPath(StringView virtualPath, String& outRepositoryPath) const {
        /// путь может быть уже реальным (":github:/Engine/...") либо виртуальным ("Resources/Engine/...")
        if (virtualPath.starts_with(GITHUB_PREFIX)) {
            virtualPath.remove_prefix(GITHUB_PREFIX.size());
        }
        else if (!GetVirtualPath().empty() && virtualPath.starts_with(GetVirtualPath())) {
            virtualPath.remove_prefix(GetVirtualPath().size());
        }

        while (virtualPath.starts_with('/')) {
            virtualPath.remove_prefix(1);
        }

        outRepositoryPath = virtualPath;

        /// в дереве репозитория пути не имеют завершающего слеша
        while (!outRepositoryPath.empty() && outRepositoryPath.ends_with('/')) {
            outRepositoryPath.remove_suffix(1);
        }
    }

    void GitHubVFSBackend::ResolveVirtualPath(StringView virtualPath, String& outRealPath) const {
        static String repositoryPathBuffer;
        ResolveRepositoryPath(virtualPath, repositoryPathBuffer);

        outRealPath = GITHUB_PREFIX;
        if (!repositoryPathBuffer.empty()) {
            outRealPath += '/';
            outRealPath += repositoryPathBuffer;
        }
    }

    FSItemType GitHubVFSBackend::GetType(StringView path) const {
        SR_TRACY_ZONE;

        if (!IsTreeLoaded()) {
            return FSItemType::Undefined;
        }

        static String repositoryPathBuffer;
        ResolveRepositoryPath(path, repositoryPathBuffer);

        if (repositoryPathBuffer.empty()) {
            return FSItemType::Folder; /// корень репозитория
        }

        auto&& tree = m_pDownloader->GetTree(m_branch);

        auto&& pIt = tree.find(repositoryPathBuffer);
        if (pIt == tree.end()) {
            return FSItemType::Undefined;
        }

        switch (pIt->second.type) {
            case SR_NETWORK_NS::GitHubEntryType::Blob:
                return FSItemType::File;
            /// подмодули (commit) с точки зрения VFS являются папками, содержимое которых недоступно
            case SR_NETWORK_NS::GitHubEntryType::Tree:
            case SR_NETWORK_NS::GitHubEntryType::Commit:
                return FSItemType::Folder;
            default:
                return FSItemType::Undefined;
        }
    }

    File GitHubVFSBackend::OpenFile(StringView path, FileMode mode) const {
        SR_TRACY_ZONE;

        if (SR_MATH_NS::IsMaskIncludedSubMask(mode, FileMode::Write)) {
            SRHalt("GitHubVFSBackend::OpenFile() : repository is read-only!");
            return File();
        }

        if (!IsTreeLoaded()) {
            return File();
        }

        static String repositoryPathBuffer;
        ResolveRepositoryPath(path, repositoryPathBuffer);

        if (repositoryPathBuffer.empty()) {
            SR_ERROR("GitHubVFSBackend::OpenFile() : resolved repository path is empty! Path: {}", path);
            return File();
        }

        auto&& tree = m_pDownloader->GetTree(m_branch);

        auto&& pTreeIt = tree.find(repositoryPathBuffer);
        if (pTreeIt == tree.end() || pTreeIt->second.type != SR_NETWORK_NS::GitHubEntryType::Blob) {
            SR_WARN("GitHubVFSBackend::OpenFile() : file not found!\n\tPath: {}", repositoryPathBuffer);
            return File();
        }

        auto&& pCacheIt = m_cache.find(repositoryPathBuffer);
        if (pCacheIt == m_cache.end()) {
            String data;
            SR_LOG("GitHubVFSBackend::OpenFile() : downloading file \"{}\" from repository \"{}/{}\" (branch: {})...", repositoryPathBuffer, m_pDownloader->GetRepo().owner, m_pDownloader->GetRepo().repo, m_branch);
            if (!m_pDownloader->DownloadFile(repositoryPathBuffer, data, m_branch)) {
                SR_ERROR("GitHubVFSBackend::OpenFile() : failed to download file!\n\tPath: {}", repositoryPathBuffer);
                return File();
            }
            pCacheIt = m_cache.emplace(repositoryPathBuffer, std::move(data)).first;
        }

        /// содержимое копируется из кеша, чтобы файл пережил его сброс
        auto&& pImpl = new MemoryFileImpl(String(pCacheIt->second));
        if (auto&& file = File(pImpl, repositoryPathBuffer, mode)) {
            return file;
        }
        return File();
    }

    void GitHubVFSBackend::Enumerate(StringView directory, const IVFSBackend::EnumerateCallback& callback, bool recursive) const {
        SR_TRACY_ZONE;

        if (!IsTreeLoaded()) {
            return;
        }

        String rootPath;
        ResolveRepositoryPath(directory, rootPath);

        static String fullPathBuffer;
        static String relativePathBuffer;

        /// дерево запрашивается рекурсивно, поэтому ключи - это полные пути от корня репозитория
        for (auto&& [repositoryPath, entry] : m_pDownloader->GetTree(m_branch)) {
            if (entry.type == SR_NETWORK_NS::GitHubEntryType::Unknown) {
                continue;
            }

            StringView relative = repositoryPath;

            if (!rootPath.empty()) {
                if (!relative.starts_with(rootPath) || relative.size() <= rootPath.size()) {
                    continue;
                }
                relative.remove_prefix(rootPath.size());
                if (!relative.starts_with('/')) {
                    continue; /// совпал только префикс имени, но не сама директория
                }
                relative.remove_prefix(1);
            }

            if (relative.empty()) {
                continue;
            }

            if (!recursive && relative.find('/') != StringView::npos) {
                continue;
            }

            fullPathBuffer = GITHUB_PREFIX;
            fullPathBuffer += '/';
            fullPathBuffer += repositoryPath;

            relativePathBuffer = repositoryPath;
            if (!GetVirtualPath().empty()) {
                relativePathBuffer.insert(0, GetVirtualPath());
                relativePathBuffer.insert(GetVirtualPath().size(), "/");
            }

            StringView name = repositoryPath;
            if (auto&& slashPos = name.find_last_of('/'); slashPos != StringView::npos) {
                name.remove_prefix(slashPos + 1);
            }

            StringView extension = name;
            if (auto&& dotPos = extension.find_last_of('.'); dotPos != StringView::npos) {
                extension.remove_prefix(dotPos + 1);
            }
            else {
                extension = {};
            }

            VFSEntry vfsEntry;
            vfsEntry.name = name;
            vfsEntry.extension = extension;
            vfsEntry.fullPath = fullPathBuffer;
            vfsEntry.relativePath = relativePathBuffer;
            vfsEntry.type = entry.type == SR_NETWORK_NS::GitHubEntryType::Blob ? FSItemType::File : FSItemType::Folder;

            callback(vfsEntry);
        }
    }

    void GitHubVFSBackend::Delete(StringView path) const {
        SRHalt("GitHubVFSBackend::Delete() : not supported!");
    }
}
