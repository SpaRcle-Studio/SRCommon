//
// Created by Monika on 15.08.2024.
//

#include <Utils/Profile/TracyContext.h>
#include <Utils/Web/HTML/HTML.h>

namespace SR_UTILS_NS::Web {
    int HTMLContainerInterface::pt_to_px(int pt) const {
        const double_t dpi = SR_PLATFORM_NS::GetScreenDPI();
        return static_cast<int>(static_cast<double>(pt) * dpi / 72.0);
    }

    int HTMLContainerInterface::get_default_font_size() const {
        return pt_to_px(12);
    }

    const char* HTMLContainerInterface::get_default_font_name() const {
        return "Times New Roman";
    }

    void HTMLContainerInterface::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) {
        baseurl = url;

        /// try found file in resource path
        SR_UTILS_NS::Path fullPath = SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(baseurl);
        if (!fullPath.IsFile()) {
            /// try found file in page folder
            fullPath = GetPath().GetFolder().Concat(baseurl);
        }

        if (!fullPath.IsFile()) {
            SR_ERROR("HTMLContainerInterface::import_css() : file not found: {}", fullPath.c_str());
            return;
        }

        text = SR_UTILS_NS::FileSystem::ReadBinaryAsString(fullPath.View());
        if (text.empty()) {
            SR_ERROR("HTMLContainerInterface::import_css() : failed to read file: {}", fullPath.c_str());
        }
        else {
            AddPath(fullPath);
        }
    }

    void HTMLContainerInterface::AddPath(const SR_UTILS_NS::Path& path) {
        for (const SR_UTILS_NS::Path& p : m_paths) {
            if (p == path) {
                return;
            }
        }
        if (SRVerify2(path.IsFile(), "HTMLContainerInterface::AddPath() : path is not a file: {}", path.c_str())) {
            m_paths.emplace_back(path);
        }
    }

    const SR_UTILS_NS::Path& HTMLContainerInterface::GetPath() const {
        static SR_UTILS_NS::Path empty;
        return m_paths.empty() ? empty : m_paths.front();
    }

    HTMLPage::HTMLPage()
        : Super(this, SR_UTILS_NS::SharedPtrPolicy::Automatic)
    { }

    HTMLPage::~HTMLPage() {
        /// строгий порядок удаления
        m_document.reset();
        m_container.AutoFree();
    }

    HTMLPage::Ptr HTMLPage::Load(const SR_UTILS_NS::Path& path, const HTMLContainerInterface::Ptr& pContainer) {
        HTMLPage::Ptr pPage = new HTMLPage();

        const auto& fullPath = SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat(path);
        auto&& data = SR_UTILS_NS::FileSystem::ReadBinaryAsString(fullPath.View());
        if (data.empty()) {
            SR_ERROR("HTMLPage::Load() : failed to read file: {}", fullPath.c_str());
            return nullptr;
        }

        pPage->m_container = pContainer;
        if (!pPage->m_container) {
            pPage->m_container = new HTMLContainerInterface();
        }

        pPage->m_container->SetPage(pPage.Get());
        pPage->m_container->AddPath(fullPath);

        //static std::string masterStyles = "html,div,body { display: block; } head,style { display: none; }";
        pPage->m_document = litehtml::document::createFromString(data, pPage->m_container.Get(), litehtml::master_css);

        return pPage;
    }

    const std::vector<SR_UTILS_NS::Path>& HTMLPage::GetPaths() const {
        static std::vector<SpaRcle::Utils::Path> empty;
        return m_container ? m_container->GetPaths() : empty;
    }
}
