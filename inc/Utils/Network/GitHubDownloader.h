//
// Created by Monika on 08.09.2026.
//

#ifndef SR_ENGINE_NETWORKING_GIT_HUB_DOWNLOADER_H
#define SR_ENGINE_NETWORKING_GIT_HUB_DOWNLOADER_H

#include <Utils/Network/IHTTPClient.h>

namespace SR_NETWORK_NS {
    struct GitHubRepo {
        String owner;
        String repo;

        SR_NODISCARD bool IsValid() const;
        SR_NODISCARD static GitHubRepo Parse(String url);
    };

    enum class GitHubEntryType {
        Unknown,
        Blob,
        Tree,
        Commit
    };

    struct GitHubEntryInfo {
        String sha;
        GitHubEntryType type = GitHubEntryType::Unknown;
        uint32_t mode = 0;
        uint64_t size = 0;
    };

    class GitHubDownloader : public NonCopyable {
        using Tree = SR_HTYPES_NS::FlatHashMap<String, GitHubEntryInfo>;
    public:
        explicit GitHubDownloader(StringView url);

    public:
        SR_NODISCARD const Vector<String>& GetBranches() const;
        SR_NODISCARD StringView GetDefaultBranch() const;
        SR_NODISCARD const GitHubRepo& GetRepo() const;
        SR_NODISCARD const Tree& GetTree(StringView branch = StringView()) const;

        /// Скачивает содержимое файла по пути внутри репозитория (raw.githubusercontent.com).
        /// Ветка по умолчанию берется из GetDefaultBranch().
        SR_NODISCARD bool DownloadFile(StringView path, String& outData, StringView branch = StringView()) const;

    private:
        mutable SR_UTILS_NS::String m_responseBody;
        mutable Vector<String> m_branches;
        mutable String m_defaultBranch;
        mutable Tree m_tree;

        String m_url;
        GitHubRepo m_repo;
        RawPointerHolder<IHTTPClient> m_pClient;

    };
}

#endif //SR_ENGINE_NETWORKING_GIT_HUB_DOWNLOADER_H
