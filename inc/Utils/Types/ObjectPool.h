//
// Created by Monika on 23.04.2024.
//

#ifndef SR_ENGINE_UTILS_OBJECT_POOL_H
#define SR_ENGINE_UTILS_OBJECT_POOL_H

#include <Utils/Types/Stack.h>
#include <Utils/Types/Function.h>

namespace SR_HTYPES_NS {
    template<typename T, typename Index = uint64_t> class ObjectPool : SR_UTILS_NS::NonCopyable {
    public:
        ObjectPool() = default;

        SR_NODISCARD uint32_t GetAliveCount() const {
            return m_objects.size() - m_freeIndices.Size();
        }

        Index Add(T&& object) {
            Index index;
            if (m_freeIndices.IsEmpty()) SR_UNLIKELY_ATTRIBUTE {
                index = m_objects.size();
                m_objects.emplace_back(true, std::move(object));
            }
            else SR_LIKELY_ATTRIBUTE {
                index = m_freeIndices.Pop();
                m_objects[index] = { true, std::move(object) };
            }
            return index;
        }

        Index Add(const T& object) {
            Index index;
            if (m_freeIndices.IsEmpty()) SR_UNLIKELY_ATTRIBUTE {
                index = m_objects.size();
                m_objects.emplace_back(true, object);
            }
            else SR_LIKELY_ATTRIBUTE {
                index = m_freeIndices.Pop();
                m_objects[index] = { true, object };
            }
            return index;
        }

        T& At(Index index) {
            auto&& object = m_objects[index];
            if (!object.first) SR_UNLIKELY_ATTRIBUTE {
                SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("ObjectPool::At() : object is invalid!\n{}\n", SR_UTILS_NS::GetStacktrace()));
                SR_PLATFORM_NS::Terminate();
            }

            return object.second;
        }

        const T& At(Index index) const {
            auto&& object = m_objects[index];
            if (!object.first) SR_UNLIKELY_ATTRIBUTE {
                SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("ObjectPool::At() : object is invalid!\n{}\n", SR_UTILS_NS::GetStacktrace()));
                SR_PLATFORM_NS::Terminate();
            }

            return object.second;
        }

        void Clear() {
            m_objects.clear();
            m_freeIndices.Clear();
        }

        void Reserve(uint64_t size) {
            m_objects.reserve(size);
        }

        void ShrinkToFit() {
            if (m_freeIndices.Size() == m_objects.size()) SR_UNLIKELY_ATTRIBUTE {
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
            return m_objects.size() == m_freeIndices.Size();
        }

        SR_NODISCARD bool IsAlive(Index index) const {
            if (index >= m_objects.size()) SR_UNLIKELY_ATTRIBUTE {
                return false;
            }

            return m_objects[index].first;
        }

        T RemoveByIndex(Index index) {
            if (index >= m_objects.size()) SR_UNLIKELY_ATTRIBUTE {
                SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("ObjectPool::Remove() : index out of bounds!\n{}\n", SR_UTILS_NS::GetStacktrace()));
                SR_PLATFORM_NS::Terminate();
            }

            if (!m_objects[index].first) SR_UNLIKELY_ATTRIBUTE {
                SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("ObjectPool::Remove() : object already removed!\n{}\n", SR_UTILS_NS::GetStacktrace()));
                SR_PLATFORM_NS::Terminate();
            }

            m_objects[index].first = false;
            m_freeIndices.Push(index);

            return std::move(m_objects[index].second);
        }

        void RemoveByObject(const T& object) {
            for (uint64_t i = 0; i < m_objects.size(); ++i) {
                if (m_objects[i].second == object) {
                    if (!m_objects[i].first) SR_UNLIKELY_ATTRIBUTE {
                        SR_PLATFORM_NS::WriteConsoleError(SR_FORMAT("ObjectPool::Remove() : object already removed!\n{}\n", SR_UTILS_NS::GetStacktrace()));
                        SR_PLATFORM_NS::Terminate();
                    }

                    m_objects[i].first = false;
                    SR_MAYBE_UNUSED T temp = std::move(m_objects[i].second);

                    m_freeIndices.Push(i);
                    return;
                }
            }
        }

        void RemoveIf(const SR_HTYPES_NS::Function<bool(Index, T&)>& condition, const SR_HTYPES_NS::Function<void(T)>& deleter) {
            Index index = 0;
            for (auto&& [isAlive, object] : m_objects) {
                if (isAlive) SR_LIKELY_ATTRIBUTE {
                    if (condition(index, object)) {
                        isAlive = false;
                        deleter(std::move(object));
                        m_freeIndices.Push(index);
                    }
                }
                ++index;
            }
        }

        void ForEach(const SR_HTYPES_NS::Function<void(Index, T&)>& func) {
            Index index = 0;
            for (auto&& [isAlive, object] : m_objects) {
                if (isAlive) SR_LIKELY_ATTRIBUTE {
                    func(index, object);
                }
                ++index;
            }
        }

        void ForEach(const SR_HTYPES_NS::Function<void(Index, const T&)>& func) const {
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
        Stack<Index> m_freeIndices;

    };
}

#endif //SR_ENGINE_UTILS_OBJECT_POOL_H
