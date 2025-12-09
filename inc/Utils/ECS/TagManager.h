//
// Created by Monika on 01.01.2023.
//

#ifndef SR_ENGINE_TAG_MANAGER_H
#define SR_ENGINE_TAG_MANAGER_H

#include <Utils/Settings.h>

namespace SR_UTILS_NS {
    class TagManager : public GlobalSettings<TagManager> {
        SR_REGISTER_SINGLETON(TagManager)
        friend class GlobalSettings<TagManager>;
        using Super = GlobalSettings<TagManager>;
        using Hash = uint64_t;

        struct TagMap {
            static constexpr uint64_t EMPTY = 0; // Нельзя использовать 0 как хеш

            uint64_t table[64] = {};   // хранит хеши
            uint8_t indexOf[64] = {};  // хеш → индекс 0..N-1
            uint8_t size = 0;         // N
            uint8_t mask = 0;         // M = 1,3,7,15,31,63 (степень двойки-1)
            uint64_t mult = 0;        // multiplier
            uint8_t shift = 0;        // shift
        };

    public:
        SR_INLINE_STATIC const StringAtom UNTAGGED = "Untagged"; /** NOLINT */

    public:
        SR_NODISCARD uint64_t TagToMask(StringAtom tag) const;
        SR_NODISCARD StringAtom GetTagByIndex(uint16_t index) const;
        SR_NODISCARD static StringAtom GetDefaultTag();
        SR_NODISCARD uint16_t GetTagIndex(StringAtom tag) const;
        SR_NODISCARD const std::vector<StringAtom>& GetTags() const { return m_tags; }

        SR_NODISCARD SR_UTILS_NS::Path GetSettingsPath() const override;

    protected:
        void ClearSettings() override;
        bool LoadSettings(const SR_XML_NS::Node& node) override;
        bool BuildTagMap();

        void RegisterTag(StringAtom tag);

    private:
        TagMap m_tagMap;
        std::map<StringAtom, uint16_t> m_indices;
        std::vector<StringAtom> m_tags;

    };
}

#endif //SR_ENGINE_TAGMANAGER_H
