//
// Created by Monika on 23.04.2024.
//

#ifndef SR_ENGINE_UTILS_OBJECT_POOL_H
#define SR_ENGINE_UTILS_OBJECT_POOL_H

#include <Utils/Types/Stack.h>
#include <Utils/Types/Function.h>
#include <Utils/Types/FastMemoryArray.h>

namespace SR_HTYPES_NS {
    template<typename T, typename Index = uint64_t> class ObjectPool : public SR_UTILS_NS::NonCopyable {
    public:
        ObjectPool() = default;

        ObjectPool(ObjectPool&& other) noexcept
            : m_objects(SR_EXCHANGE(other.m_objects, { }))
            , m_freeIndices(SR_EXCHANGE(other.m_freeIndices, { }))
        { }

        ObjectPool& operator=(ObjectPool&& other) noexcept {
            m_objects = SR_EXCHANGE(other.m_objects, { });
            m_freeIndices = SR_EXCHANGE(other.m_freeIndices, { });
            return *this;
        }

        SR_NODISCARD uint32_t GetAliveCount() const {
            return m_objects.size() - m_freeIndices.size();
        }

        SR_NODISCARD uint32_t GetFreeCount() const {
            return m_freeIndices.size();
        }

        SR_NODISCARD uint32_t GetCapacity() const {
            return m_objects.size();
        }

        SR_NODISCARD const std::vector<std::pair<bool, T>>& GetObjects() const {
            return m_objects;
        }

        Index Add(T&& object) {
            Index index;
            if (m_freeIndices.empty()) SR_UNLIKELY_ATTRIBUTE {
                index = m_objects.size();
                m_objects.emplace_back(true, std::move(object));
            }
            else SR_LIKELY_ATTRIBUTE {
                index = m_freeIndices.back();
                m_freeIndices.pop_back();
                m_objects[index] = { true, std::move(object) };
            }
            return index;
        }

        Index Add(const T& object) {
            Index index;
            if (m_freeIndices.empty()) SR_UNLIKELY_ATTRIBUTE {
                index = m_objects.size();
                m_objects.emplace_back(true, object);
            }
            else SR_LIKELY_ATTRIBUTE {
                index = m_freeIndices.back();
                m_freeIndices.pop_back();
                m_objects[index] = { true, object };
            }
            return index;
        }

        T* GetFirstAlive() {
            for (auto&& [isAlive, object] : m_objects) {
                if (isAlive) SR_LIKELY_ATTRIBUTE {
                    return &object;
                }
            }
            return nullptr;
        }

        T& At(Index index) {
            auto&& object = m_objects[index];
            if (!object.first) SR_UNLIKELY_ATTRIBUTE {
                SRHaltTerminate("ObjectPool::At() : object is invalid!");
            }

            return object.second;
        }

        const T& At(Index index) const {
            auto&& object = m_objects[index];
            if (!object.first) SR_UNLIKELY_ATTRIBUTE {
                SRHaltTerminate("ObjectPool::At() : object is invalid!");
            }

            return object.second;
        }

        T& SR_FASTCALL AtUnchecked(Index index) {
            return m_objects[index].second;
        }

        const T& SR_FASTCALL AtUnchecked(Index index) const {
            return m_objects[index].second;
        }

        void Clear() {
            m_objects.clear();
            m_freeIndices.clear();
        }

        void Reserve(uint64_t size) {
            m_objects.reserve(size);
        }

        void ShrinkToFit() {
            if (m_freeIndices.size() == m_objects.size()) SR_UNLIKELY_ATTRIBUTE {
                Clear();
                m_objects.shrink_to_fit();
                return;
            }

            m_objects.shrink_to_fit();

            uint64_t count = 0;
            uint64_t index = m_objects.size() - 1;

            while (!m_objects[index].first) {
                ++count;
                if (index == 0) SR_UNLIKELY_ATTRIBUTE {
                    break;
                }
                --index;
            }

            if (count == 0) SR_UNLIKELY_ATTRIBUTE {
                return;
            }

            m_objects.resize(m_objects.size() - count);
        }

        SR_NODISCARD bool IsEmpty() const {
            return m_objects.size() == m_freeIndices.size();
        }

        SR_NODISCARD bool SR_FASTCALL IsAlive(Index index) const {
            if (index >= m_objects.size()) SR_UNLIKELY_ATTRIBUTE {
                return false;
            }

            return m_objects[index].first;
        }

        T RemoveByIndex(Index index) {
            if (index >= m_objects.size()) SR_UNLIKELY_ATTRIBUTE {
                SRHaltTerminate("ObjectPool::Remove() : index out of bounds!");
            }

            if (!m_objects[index].first) SR_UNLIKELY_ATTRIBUTE {
                SRHaltTerminate("ObjectPool::Remove() : object already removed!");
            }

            m_objects[index].first = false;
            m_freeIndices.emplace_back(index);

            return std::move(m_objects[index].second);
        }

        void RemoveByObject(const T& object) {
            for (uint64_t i = 0; i < m_objects.size(); ++i) {
                if (m_objects[i].second == object) {
                    if (!m_objects[i].first) SR_UNLIKELY_ATTRIBUTE {
                        SRHaltTerminate("ObjectPool::Remove() : object already removed!");
                    }

                    m_objects[i].first = false;
                    SR_MAYBE_UNUSED T temp = std::move(m_objects[i].second);

                    m_freeIndices.emplace_back(i);
                    return;
                }
            }
        }

        template<typename Pred, typename OnRemove> void RemoveIf(Pred&& pred, OnRemove&& onRemove) {
            SR_TRACY_ZONE;

            Index index = 0;
            for (auto&& [isAlive, object] : m_objects) {
                if (isAlive) SR_LIKELY_ATTRIBUTE {
                    if (pred(index, object)) {
                        isAlive = false;
                        onRemove(std::move(object));
                        m_freeIndices.emplace_back(index);
                    }
                }
                ++index;
            }
        }

        void ForEach(const SR_HTYPES_NS::Function<void(Index, T&)>& func) {
            if (IsEmpty()) SR_UNLIKELY_ATTRIBUTE {
                return;
            }

            Index index = 0;
            for (auto&& [isAlive, object] : m_objects) {
                if (isAlive) SR_LIKELY_ATTRIBUTE {
                    func(index, object);
                }
                ++index;
            }
        }

        void ForEach(const SR_HTYPES_NS::Function<void(Index, const T&)>& func) const {
            if (IsEmpty()) SR_UNLIKELY_ATTRIBUTE {
                return;
            }

            Index index = 0;
            for (auto&& [isAlive, object] : m_objects) {
                if (isAlive) SR_LIKELY_ATTRIBUTE {
                    func(index, object);
                }
                ++index;
            }
        }

    private:
        std::vector<std::pair<bool, T>> m_objects;
        SR_HTYPES_NS::FastMemoryArray<Index> m_freeIndices;

    };
}

#endif //SR_ENGINE_UTILS_OBJECT_POOL_H
