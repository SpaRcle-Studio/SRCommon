//
// Created by Monika on 10.01.2022.
//

#ifndef SR_ENGINE_FEATURES_H
#define SR_ENGINE_FEATURES_H

#include <Utils/Common/Singleton.h>
#include <Utils/Types/StringAtom.h>
#include <Utils/FileSystem/Path.h>

namespace SR_UTILS_NS {
    namespace Xml {
        class Node;
    }
    class Features;

    class SR_COMMON_DLL_API FeatureGroup {
        friend class Features;
    public:
        SR_NODISCARD bool Enabled(StringAtom name) const;
        SR_NODISCARD bool Enabled(StringAtom name, bool def) const;

        ~FeatureGroup();

    private:
        bool Register(StringAtom name, bool value);

    private:
        std::unordered_map<StringAtom, bool> m_values;

    };

    class Features : public Singleton<Features> {
        SR_REGISTER_SINGLETON(Features);
    private:
        ~Features() override = default;

    public:
        bool Reload();

        void SetPath(const SR_UTILS_NS::Path& path);

        SR_NODISCARD bool Enabled(StringAtom name) const;
        SR_NODISCARD bool Enabled(StringAtom name, bool def) const;
        SR_NODISCARD bool Enabled(StringAtom group, StringAtom name) const;

    private:
        SR_NODISCARD const FeatureGroup& GetGroup(StringAtom name) const;
        bool Register(StringAtom group, StringAtom name, bool value);
        bool RegisterGroup(StringAtom group, const Xml::Node& node, const std::vector<StringAtom>& variants);

    private:
        bool m_isInitialized = false;
        std::unordered_map<StringAtom, FeatureGroup> m_features;
        Path m_path;

    };
}

#endif //SR_ENGINE_FEATURES_H
