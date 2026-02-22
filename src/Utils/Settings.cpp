//
// Created by Monika on 17.06.2022.
//

#include <Utils/Settings.h>

#include <Codegen/Settings.generated.hpp>

namespace SR_UTILS_NS {
    bool Settings::Load() {
        Path path = GetResourcePath();
        if (!path.IsAbs()) {
            path = GetAssociatedPath().Concat(path);
        }

        m_document = SR_XML_NS::Document::Load(path);
        if (!m_document.Valid()) {
            SR_ERROR("Settings::Load() : file not found! \n\tPath: " + path.ToString());
            return false;
        }

        if (auto&& settings = m_document.Root().GetNode("Settings")) {
            LoadSettings(settings);
        }
        else {
            SR_ERROR("Settings::Load() : \"Settings\" node not found! \n\tPath: " + path.ToString());
            return false;
        }

        return IResource::Load();
    }

    bool Settings::Unload() {
        ClearSettings();

        return IResource::Unload();
    }

    Path Settings::GetAssociatedPath() const {
        return ResourceManager::Instance().GetResPath();
    }

    bool Settings::Destroy() {
        return IResource::Destroy();
    }

    SR_XML_NS::Document Settings::LoadDocument() const {
        Path path = GetResourcePath();
        if (!path.IsAbs()) {
            path = GetAssociatedPath().Concat(path);
        }

        return std::move(SR_XML_NS::Document::Load(path));
    }

    void Settings::Do(const SR_HTYPES_NS::Function<void(Settings*)> &fun) {
        fun(this);
    }

    void Settings::ClearSettings() {

    }

    bool Settings::LoadSettings(const Xml::Node& node) {
        return true;
    }

    const SR_XML_NS::Document& Settings::GetDocument() const {
        return m_document;
    }

    Settings::Settings() = default;

    Settings::~Settings() = default;
}

