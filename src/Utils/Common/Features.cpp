//
// Created by Monika on 13.01.2022.
//

#include <Utils/Common/Features.h>
#include <Utils/Common/CLIManager.h>
#include <Utils/Resources/Xml.h>
#include <Utils/Types/Thread.h>
#include <Utils/Profile/TracyContext.h>

namespace SR_UTILS_NS {
    static const StringAtom SR_FEATURE_COMMON_GROUP_ID = "Common";

    static const StringAtom SR_FEATURE_VARIANT_DEBUG_ID = "Debug";
    static const StringAtom SR_FEATURE_VARIANT_RELEASE_ID = "Release";
    static const StringAtom SR_FEATURE_VARIANT_VALIDATION_ID = "Validation";
    static const StringAtom SR_FEATURE_VARIANT_EMSCRIPTEN_ID = "Emscripten";
    static const StringAtom SR_FEATURE_VARIANT_WINDOWS_ID = "Windows";
    static const StringAtom SR_FEATURE_VARIANT_ANDROID_ID = "Android";
    static const StringAtom SR_FEATURE_VARIANT_LINUX_ID = "Linux";

    bool FeatureGroup::Register(StringAtom name, bool value) {
        if (m_values.count(name)) {
            SR_LOG("FeatureGroup::Register() : overriding feature \"{}\" from {} to {}!", name, m_values[name], value);
        }

        m_values[name] = value;

        return true;
    }

    bool FeatureGroup::Enabled(StringAtom name) const {
        if (auto&& pIt = m_values.find(name); pIt != m_values.end()) {
            return pIt->second;
        }

        SR_WARN("FeatureGroup::Enabled() : feature \"{}\" not found!", name);

        return false;
    }

    bool FeatureGroup::Enabled(StringAtom name, bool def) const {
        if (auto&& pIt = m_values.find(name); pIt != m_values.end()) {
            return pIt->second;
        }

        return def;
    }

    FeatureGroup::~FeatureGroup() = default;

    bool Features::Enabled(StringAtom group, StringAtom name) const {
        return GetGroup(group).Enabled(name);
    }

    void Features::SetPath(const SR_UTILS_NS::Path& path) {
        SR_LOCK_GUARD;

        if (path.IsEmpty() || !path.Exists()) {
            SR_ERROR("Features::Reload() : file not found! \n\tPath: {}", path);
            return;
        }

        m_path = path;
    }

    bool Features::Reload() {
        SR_LOCK_GUARD;
        SR_TRACY_ZONE;

        if (m_path.IsEmpty()) {
            SR_ERROR("Features::Reload() : path is empty!");
            return false;
        }

        m_features.clear();

        const auto& doc = SR_XML_NS::Document::Load(m_path);
        if (!doc) {
            SR_ERROR("Features::Reload() : failed to load document! \n\tPath: {}", m_path);
            return false;
        }

        std::vector<StringAtom> groups = { SR_FEATURE_COMMON_GROUP_ID };
        std::vector<StringAtom> variants = {
        #ifdef SR_DEBUG
            SR_FEATURE_VARIANT_DEBUG_ID,
        #endif
        #ifdef SR_RELEASE
            SR_FEATURE_VARIANT_RELEASE_ID,
        #endif
        #if defined(SR_EMSCRIPTEN)
            SR_FEATURE_VARIANT_EMSCRIPTEN_ID,
        #endif
        #if defined(SR_WIN32)
            SR_FEATURE_VARIANT_WINDOWS_ID,
        #endif
        #if defined(SR_ANDROID)
            SR_FEATURE_VARIANT_ANDROID_ID,
        #endif
        #if defined(SR_LINUX)
            SR_FEATURE_VARIANT_LINUX_ID,
        #endif
        };

        if (CLIManager::Instance().IsFlagPresent(CLIFlags::Validation)) {
            variants.push_back(SR_FEATURE_VARIANT_VALIDATION_ID);
        }

        if (const auto& groupsNode = doc.Root().GetNode("Features")) {
            for (const auto& groupName : groups) {
                RegisterGroup(groupName, groupsNode, variants);
            }
        }
        else {
            SR_ERROR("Features::Reload() : error while loading file! \n\tPath: {}", m_path);
            return false;
        }

        std::string featuresList;
        featuresList.reserve(1024);
        for (const auto& [groupName, group] : m_features) {
            featuresList += "\n\t[" + groupName.ToString() + "]";
            for (const auto& [featureName, featureValue] : group.m_values) {
                featuresList += "\n\t\t" + featureName.ToString() + " = " + (featureValue ? "true" : "false");
            }
        }
        SR_LOG("Features::Reload() : features are successfully reloaded! Features: {}", featuresList);

        m_isInitialized = true;

        return true;
    }

    bool Features::Enabled(StringAtom name) const {
        SR_TRACY_ZONE;
        return GetGroup(SR_FEATURE_COMMON_GROUP_ID).Enabled(name);
    }

    bool Features::Enabled(StringAtom name, bool def) const {
        SR_TRACY_ZONE;
        return GetGroup(SR_FEATURE_COMMON_GROUP_ID).Enabled(name, def);
    }

    bool Features::Register(StringAtom group, StringAtom name, bool value) {
        if (auto&& pIt = m_features.find(group); pIt == m_features.end()) {
            m_features.insert(std::make_pair(group, FeatureGroup()));
        }

        return m_features.at(group).Register(name, value);
    }

    const FeatureGroup& Features::GetGroup(StringAtom name) const {
        SR_LOCK_GUARD;

        if (auto pIt = m_features.find(name); pIt != m_features.end()) {
            return pIt->second;
        }

        static FeatureGroup defaultFeature = FeatureGroup();
        return defaultFeature;
    }

    bool Features::RegisterGroup(StringAtom group, const Xml::Node& node, const std::vector<StringAtom>& variants) {
        SR_TRACY_ZONE;

        static const std::string valueStr = "Value";
        static const std::string variantStr = "Variant";
        static const std::string nameStr = "Name";

        for (const auto& groupNode : node.GetNodes()) {
            if (groupNode.Name() != group) {
                continue;
            }

            for (const auto& featureNode : groupNode.GetNodes()) {
                if (featureNode.Name() == variantStr) {
                    continue;
                }
                Register(group, featureNode.Name(), featureNode.GetAttribute(valueStr).ToBool(false));
            }

            for (const auto& variant : variants) {
                for (const auto& variantNode : groupNode.GetNodes(variantStr)) {
                    if (variantNode.Name() != variantStr) {
                        continue;
                    }

                    if (variantNode.GetAttribute(nameStr).ToString() == variant) {
                        for (const auto& featureNode : variantNode.GetNodes()) {
                            Register(group, featureNode.Name(), featureNode.GetAttribute(valueStr).ToBool(false));
                        }
                    }
                }
            }

            return true;
        }
        return false;
    }
}