//
// Created by Monika on 01.01.2023.
//

#include <Utils/ECS/TagManager.h>
#include <Utils/Common/StringAtomLiterals.h>

namespace SR_UTILS_NS {
    static uint64_t MixTag(uint64_t x, uint64_t mult, uint64_t shift) {
        return (x * mult) >> shift;
    }

    void TagManager::RegisterTag(StringAtom tag) {
        SR_LOCK_GUARD;

        if (m_indices.count(tag) == 0) {
            m_tags.emplace_back(tag);
            m_indices[tag] = m_indices.size();
        }
    }

    SR_UTILS_NS::Path TagManager::GetSettingsPath() const {
        return "Engine/Configs/TagManagerSettings.xml";
    }

    void TagManager::ClearSettings() {
        SR_LOCK_GUARD;

        m_tags.clear();
        m_indices.clear();
        m_tagMap = {};

        Super::ClearSettings();
    }

    bool TagManager::LoadSettings(const Xml::Node &node) {
        SR_LOCK_GUARD;

        m_tags.clear();
        m_indices.clear();

        RegisterTag(UNTAGGED);

        if (auto&& tagsNode = node.GetNode("Tags")) {
            for (auto&& tagNode : tagsNode.GetNodes()) {
                RegisterTag(tagNode.Name());
            }
        }
        BuildTagMap();

        return Super::LoadSettings(node);
    }

    uint16_t TagManager::GetTagIndex(StringAtom tag) const {
        SR_LOCK_GUARD;

        if (tag.empty()) {
            return 0;
        }

        auto&& pIt = m_indices.find(tag);

        if (pIt == m_indices.end()) {
            SRHalt("TagManager::GetTagIndex() : unknown tag!");
            return SR_ID_INVALID;
        }

        return pIt->second;
    }

    StringAtom TagManager::GetTagByIndex(uint16_t index) const {
        SR_LOCK_GUARD;

        if (index >= m_tags.size()) {
            SRHalt("TagManager::GetTagByIndex() : out of range!");
            static const StringAtom def;
            return def;
        }

        return m_tags.at(index);
    }

    StringAtom TagManager::GetDefaultTag() {
        auto&& instance = TagManager::Instance();

        if (instance.m_tags.empty()) {
            return "Default";
        }

        return instance.m_tags[0];
    }

    uint64_t TagManager::TagToMask(StringAtom tag) const {
        if (tag.empty()) {
            tag = GetDefaultTag();
        }

        const uint64_t hash = tag.GetHash();
        uint64_t pos = MixTag(hash, m_tagMap.mult, m_tagMap.shift) & m_tagMap.mask;

        if (m_tagMap.table[pos] == hash)
            return 1ull << m_tagMap.indexOf[pos];

        SRHalt("TagManager::TagToMask() : unknown tag \"{}\"!", tag);
        return 0;
    }

    bool TagManager::BuildTagMap() {
        m_tagMap = {};

        SR_LOG("TagManager::BuildTagMap() : building perfect hash for {} tags...", m_tags.size());

        m_tagMap.size = (uint8_t)m_tags.size();

        // Выбираем размер таблицы — ближайшая степень двойки
        uint8_t M = 1;
        while (M < m_tagMap.size) M <<= 1;
        m_tagMap.mask = M - 1;

        // Пытаемся найти perfect hash
        for (uint64_t mult = 1; mult < (1ull << 32); mult += 2) {
            for (uint8_t shift = 0; shift < 16; ++shift) {
                bool ok = true;
                std::fill(std::begin(m_tagMap.table), std::begin(m_tagMap.table) + M, TagMap::EMPTY);

                for (uint8_t i = 0; i < m_tagMap.size; ++i) {
                    uint64_t h = MixTag(m_tags[i], mult, shift) & m_tagMap.mask;

                    if (m_tagMap.table[h] != TagMap::EMPTY) {
                        ok = false;
                        break;
                    }

                    m_tagMap.table[h] = m_tags[i];
                    m_tagMap.indexOf[h] = i;
                }

                if (ok) {
                    m_tagMap.mult = mult;
                    m_tagMap.shift = shift;
                    return true;
                }
            }
        }

        std::string tagsList;
        for (uint64_t i = 0; i < m_tags.size(); ++i) {
            tagsList += "\t[{}] {}\n"_format(i, m_tags[i]);
        }
        SRHalt("TagManager::BuildTagMap() : failed to build perfect hash for tags! It's extremely unlikely case. Try to change your tags list:\n{}", tagsList);

        return false;
    }
}