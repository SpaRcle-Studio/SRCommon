//
// Created by Monika on 01.03.2026.
//

#ifndef SR_ENGINE_COMMON_FLAT_HASH_MAP_H
#define SR_ENGINE_COMMON_FLAT_HASH_MAP_H

#include <Utils/stdInclude.h>

namespace SR_HTYPES_NS {
    template<typename Key, typename Value> class FlatHashMap {
    #if defined(SR_EMSCRIPTEN)
        using Impl = std::unordered_map<Key, Value>;
    #else
        using Impl = ska::flat_hash_map<Key, Value>;
    #endif
    public:
        using Iterator = typename Impl::iterator;
        using ConstIterator = typename Impl::const_iterator;
        using value_type = typename std::pair<const Key, Value>;

    public:
        void clear() { m_map.clear(); }
        SR_NODISCARD Iterator find(const Key& key) { return m_map.find(key); }
        SR_NODISCARD ConstIterator find(const Key& key) const { return m_map.find(key); }
        SR_NODISCARD Iterator end() { return m_map.end(); }
        SR_NODISCARD ConstIterator end() const { return m_map.end(); }
        SR_NODISCARD Iterator begin() { return m_map.begin(); }
        SR_NODISCARD ConstIterator begin() const { return m_map.begin(); }
        SR_NODISCARD Value& operator[](const Key& key) { return m_map[key]; }
        SR_NODISCARD Value& operator[](Key&& key) { return m_map[std::move(key)]; }
        SR_NODISCARD Value& at(const Key& key) { return m_map.at(key); }
        SR_NODISCARD const Value& at(const Key& key) const { return m_map.at(key); }
        SR_NODISCARD uint64_t size() const { return m_map.size(); }
        SR_NODISCARD uint64_t count(const Key& key) const { return m_map.count(key); }
        SR_NODISCARD bool empty() const { return m_map.empty(); }
        SR_NODISCARD bool contains(const Key& key) const { return m_map.count(key) > 0; }
        bool erase(const Key& key) { return m_map.erase(key) > 0; }
        Iterator erase(ConstIterator pos) { return m_map.erase(pos); }
        Iterator erase(ConstIterator first, ConstIterator last) { return m_map.erase(first, last); }
        void reserve(size_t new_capacity) { m_map.reserve(new_capacity); }

        template<typename... Args> std::pair<Iterator, bool> emplace(Args&&... args) {
            return m_map.emplace(std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator emplace_hint(ConstIterator hint, Args&&... args) {
            return m_map.emplace_hint(hint, std::forward<Args>(args)...);
        }

        template<typename... Args> std::pair<Iterator, bool> try_emplace(const Key& key, Args&&... args) {
            return m_map.try_emplace(key, std::forward<Args>(args)...);
        }

        template<typename... Args> std::pair<Iterator, bool> try_emplace(Key&& key, Args&&... args) {
            return m_map.try_emplace(std::move(key), std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator try_emplace_hint(ConstIterator hint, const Key& key, Args&&... args) {
            return m_map.try_emplace_hint(hint, key, std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator try_emplace_hint(ConstIterator hint, Key&& key, Args&&... args) {
            return m_map.try_emplace_hint(hint, std::move(key), std::forward<Args>(args)...);
        }

        template<typename... Args> std::pair<Iterator, bool> insert(Args&&... args) {
            return m_map.insert(std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator insert(ConstIterator hint, Args&&... args) {
            return m_map.insert(hint, std::forward<Args>(args)...);
        }

        void max_load_factor(float factor) { m_map.max_load_factor(factor); }
        SR_NODISCARD float max_load_factor() const { return m_map.max_load_factor(); }

    private:
        Impl m_map{};

    };

    template<typename Key> class FlatHashSet {
    #if defined(SR_EMSCRIPTEN)
        using Impl = std::unordered_set<Key>;
    #else
        using Impl = ska::flat_hash_set<Key>;
    #endif
        using Iterator = typename Impl::iterator;
        using ConstIterator = typename Impl::const_iterator;
    public:
        void clear() { m_map.clear(); }
        SR_NODISCARD Iterator find(const Key& key) { return m_map.find(key); }
        SR_NODISCARD ConstIterator find(const Key& key) const { return m_map.find(key); }
        SR_NODISCARD Iterator end() { return m_map.end(); }
        SR_NODISCARD ConstIterator end() const { return m_map.end(); }
        SR_NODISCARD Iterator begin() { return m_map.begin(); }
        SR_NODISCARD ConstIterator begin() const { return m_map.begin(); }
        SR_NODISCARD uint64_t size() const { return m_map.size(); }
        SR_NODISCARD uint64_t count(const Key& key) const { return m_map.count(key); }
        SR_NODISCARD bool empty() const { return m_map.empty(); }
        SR_NODISCARD bool contains(const Key& key) const { return m_map.count(key) > 0; }
        bool erase(const Key& key) { return m_map.erase(key) > 0; }
        Iterator erase(ConstIterator pos) { return m_map.erase(pos); }
        Iterator erase(ConstIterator first, ConstIterator last) { return m_map.erase(first, last); }
        void reserve(size_t new_capacity) { m_map.reserve(new_capacity); }

        template<typename... Args> std::pair<Iterator, bool> emplace(Args&&... args) {
            return m_map.emplace(std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator emplace_hint(ConstIterator hint, Args&&... args) {
            return m_map.emplace_hint(hint, std::forward<Args>(args)...);
        }

        template<typename... Args> std::pair<Iterator, bool> try_emplace(const Key& key, Args&&... args) {
            return m_map.try_emplace(key, std::forward<Args>(args)...);
        }

        template<typename... Args> std::pair<Iterator, bool> try_emplace(Key&& key, Args&&... args) {
            return m_map.try_emplace(std::move(key), std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator try_emplace_hint(ConstIterator hint, const Key& key, Args&&... args) {
            return m_map.try_emplace_hint(hint, key, std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator try_emplace_hint(ConstIterator hint, Key&& key, Args&&... args) {
            return m_map.try_emplace_hint(hint, std::move(key), std::forward<Args>(args)...);
        }

        template<typename... Args> std::pair<Iterator, bool> insert(Args&&... args) {
            return m_map.insert(std::forward<Args>(args)...);
        }

        template<typename... Args> Iterator insert(ConstIterator hint, Args&&... args) {
            return m_map.insert(hint, std::forward<Args>(args)...);
        }

        void max_load_factor(float factor) { m_map.max_load_factor(factor); }
        SR_NODISCARD float max_load_factor() const { return m_map.max_load_factor(); }

    private:
        Impl m_map{};

    };
}

#endif //SR_ENGINE_COMMON_FLAT_HASH_MAP_H
