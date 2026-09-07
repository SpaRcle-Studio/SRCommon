//
// Created by Monika on 08.09.2026.
//

#include <Utils/Network/GitHubDownloader.h>
#include <Utils/Platform/Platform.h>
#include <Utils/Common/LexicalCast.h>

#include <nlohmann/json.hpp>

namespace SR_NETWORK_NS {
    bool GitHubRepo::IsValid() const {
        return !owner.empty() && !repo.empty();
    }

    GitHubRepo GitHubRepo::Parse(String url) {
        constexpr StringView https = "https://";
        constexpr StringView http  = "http://";

        if (url.starts_with(https))
            url.erase(0, https.size());
        else if (url.starts_with(http))
            url.erase(0, http.size());

        if (url.starts_with("www."))
            url.erase(0, 4);

        constexpr StringView github = "github.com/";
        if (!url.starts_with(github))
            return {};

        url.erase(0, github.size());

        const size_t ownerEnd = url.find('/');

        if (ownerEnd == String::npos || ownerEnd == 0)
            return {};

        String owner = url.substr(0, ownerEnd);

        const size_t repoStart = ownerEnd + 1;
        if (repoStart >= url.size())
            return {};

        const size_t repoEnd = url.find_first_of("/?#", repoStart);
        String repo = url.substr(
            repoStart,
            repoEnd == String::npos ? String::npos : repoEnd - repoStart
        );

        constexpr StringView gitSuffix = ".git";
        if (repo.ends_with(gitSuffix))
            repo.erase(repo.size() - gitSuffix.size());

        if (repo.empty())
            return {};

        return { std::move(owner), std::move(repo) };
    }

    GitHubDownloader::GitHubDownloader(StringView url)
        : m_url(url)
    {
        m_repo = GitHubRepo::Parse(url);
        m_pClient = CreateHTTPClient();
        if (!m_pClient) {
            SRHalt("GitHubDownloader::GitHubDownloader() : failed to create HTTP client!");
            return;
        }
    }

    const Vector<String>& GitHubDownloader::GetBranches() const {
        SR_TRACY_ZONE;

        if (!m_branches.empty()) {
            return m_branches;
        }

        SR_NETWORK_NS::HTTPRequest request {
            .method = SR_NETWORK_NS::HTTPMethod::GET,
            .url = "https://api.github.com/repos/{}/{}/branches"_format(m_repo.owner, m_repo.repo),
            .headers = {
                { "Accept", "application/vnd.github+json" },
                { "User-Agent", SR_PLATFORM_NS::GetApplicationName() }
            }
        };

        m_responseBody.clear();
        SR_NETWORK_NS::HTTPResponse response;
        const bool success = m_pClient->Send(request, response, [this](std::span<const std::byte> data) {
            m_responseBody.append(reinterpret_cast<const char*>(data.data()), data.size());
            return true;
        });

        if (!success || response.statusCode != 200) {
            SR_ERROR("GitHubDownloader::GetBranches() : failed to get branches! Status code: {}", response.statusCode);
            return m_branches;
        }

        try {
            auto json = nlohmann::json::parse(m_responseBody.view());
            for (const auto& branch : json) {
                if (branch.contains("name")) {
                    m_branches.emplace_back(branch["name"].get<std::string_view>());
                }
            }
        }
        catch (const std::exception& e) {
            SR_ERROR("GitHubDownloader::GetBranches() : failed to parse JSON response! Error: {}", e.what());
        }

        return m_branches;
    }

    StringView GitHubDownloader::GetDefaultBranch() const {
        SR_TRACY_ZONE;

        if (!m_defaultBranch.empty()) {
            return m_defaultBranch;
        }

        SR_NETWORK_NS::HTTPRequest request {
            .method = SR_NETWORK_NS::HTTPMethod::GET,
            .url = "https://api.github.com/repos/{}/{}"_format(m_repo.owner, m_repo.repo),
            .headers = {
                { "Accept", "application/vnd.github+json" },
                { "User-Agent", SR_PLATFORM_NS::GetApplicationName() }
            }
        };

        m_responseBody.clear();

        SR_NETWORK_NS::HTTPResponse response;
        const bool success = m_pClient->Send(request, response, [this](std::span<const std::byte> data) {
            m_responseBody.append(reinterpret_cast<const char*>(data.data()), data.size());
            return true;
        });

        if (!success || response.statusCode != 200) {
            SR_ERROR("GitHubDownloader::GetDefaultBranch() : failed to get repository! Status code: {}", response.statusCode);
            return {};
        }

        try {
            auto json = nlohmann::json::parse(m_responseBody.view());
            if (json.contains("default_branch")) {
                m_defaultBranch = json["default_branch"].get<std::string>();
            }
        }
        catch (const std::exception& e) {
            SR_ERROR("GitHubDownloader::GetDefaultBranch() : failed to parse JSON response! Error: {}", e.what());
        }

        return m_defaultBranch;
    }

    const GitHubRepo& GitHubDownloader::GetRepo() const {
        return m_repo;
    }

    const GitHubDownloader::Tree& GitHubDownloader::GetTree(StringView branch) const {
        SR_TRACY_ZONE;

        if (!m_tree.empty()) {
            return m_tree;
        }

        if (branch.empty()) {
            branch = GetDefaultBranch();
            if (branch.empty()) {
                SR_ERROR("GitHubDownloader::GetTree() : default branch is empty!");
                return m_tree;
            }
        }

        SR_NETWORK_NS::HTTPRequest request {
            .method = SR_NETWORK_NS::HTTPMethod::GET,
            .url = "https://api.github.com/repos/{}/{}/git/trees/{}?recursive=1"_format(m_repo.owner, m_repo.repo, branch),
            .headers = {
                { "Accept", "application/vnd.github+json" },
                { "User-Agent", SR_PLATFORM_NS::GetApplicationName() }
            }
        };

        m_responseBody.clear();
        SR_NETWORK_NS::HTTPResponse response;
        const bool success = m_pClient->Send(request, response,[this](std::span<const std::byte> data) {
            m_responseBody.append(reinterpret_cast<const char*>(data.data()), data.size());
            return true;
        });

        if (!success || response.statusCode != 200) {
            SR_ERROR("GitHubDownloader::GetTree() : failed to get repository tree! Status code: {}", response.statusCode);
            return m_tree;
        }

        try {
            const auto json = nlohmann::json::parse(m_responseBody.view());
            if (!json.contains("tree") || !json["tree"].is_array()) {
                SR_ERROR("GitHubDownloader::GetTree() : invalid GitHub tree response!");
                return m_tree;
            }

            for (const auto& entry : json["tree"]) {
                if (!entry.contains("type") || !entry.contains("path") || !entry.contains("sha") || !entry.contains("mode")) {
                    continue;
                }

                GitHubEntryInfo info;

                info.sha = entry["sha"].get<std::string_view>();
                info.mode = LexicalCast<uint32_t>(entry["mode"].get<std::string_view>());
                StringView type = entry["type"].get<std::string_view>();

                if (type == "blob") {
                    info.type = GitHubEntryType::Blob;
                }
                else if (type == "tree") {
                    info.type = GitHubEntryType::Tree;
                }
                else if (type == "commit") {
                    info.type = GitHubEntryType::Commit;
                }
                else {
                    info.type = GitHubEntryType::Unknown;
                }

                if (entry.contains("size") && !entry["size"].is_null()) {
                    info.size = entry["size"].get<uint64_t>();
                }

                m_tree.emplace(entry["path"].get<std::string_view>(), std::move(info));
            }
        }
        catch (const std::exception& e) {
            SR_ERROR("GitHubDownloader::GetTree() : failed to parse JSON response! Error: {}", e.what());
        }

        return m_tree;
    }
}
