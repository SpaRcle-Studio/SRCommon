//
// Created by Monika on 19.11.2026.
//

#ifndef SR_ENGINE_UTILS_REFLECTION_VALUE_REF_H
#define SR_ENGINE_UTILS_REFLECTION_VALUE_REF_H

#include <Utils/Reflection/ReflectedType.h>

namespace SR_HTYPES_NS {
    class SharedPtrBase;
}

namespace SR_UTILS_NS {
    class OptionalBase;
}

namespace SR_UTILS_NS::Reflection {
    class Value;
    class ArrayView;
    class MapView;

    /// Невладеющая ссылка на значение известного типа: не аллоцирует, не вызывает деструктор и копируется побайтно.
    /// Действительна, пока жив объект, на который ссылается.
    class SR_COMMON_DLL_API ValueRef {
    public:
        ValueRef() = default;
        ValueRef(const TypeInfo* pTypeInfo, void* pData, bool isConst) noexcept
            : m_pTypeInfo(pTypeInfo)
            , m_pData(pData)
            , m_isConst(isConst)
        { }

        template<typename T> static ValueRef Of(T& value);

        template<typename T> T* Cast() const noexcept { return static_cast<T*>(m_pData); }

        SR_NODISCARD const TypeInfo& GetTypeInfo() const noexcept;
        SR_NODISCARD void* Data() const noexcept { return m_pData; }
        SR_NODISCARD bool IsConst() const noexcept { return m_isConst; }
        SR_NODISCARD bool IsValid() const noexcept { return m_pTypeInfo && m_pData; }
        SR_NODISCARD explicit operator bool() const noexcept { return IsValid(); }
        SR_NODISCARD SizeType SizeOf() const;

        SR_NODISCARD SRClass* GetSRClass() const;
        SR_NODISCARD SR_HTYPES_NS::SharedPtrBase* GetSharedPtrBase() const;
        SR_NODISCARD OptionalBase* GetOptionalBase() const;

        SR_NODISCARD ArrayView AsArray() const;
        SR_NODISCARD MapView AsMap() const;

        /// Value, ссылающийся на те же данные
        SR_NODISCARD Value ToValue() const;
        SR_NODISCARD Value Copy() const;

        /// false, если типы различаются или ссылка константная
        bool Assign(const ValueRef& source) const;

    private:
        void* GetTypeController() const;

    private:
        const TypeInfo* m_pTypeInfo = nullptr;
        void* m_pData = nullptr;
        bool m_isConst = false;

    };

    /// ----------------------------------------------------------------------------------------------------------------

    /// Vector с доступом по индексу. Ссылки на элементы становятся недействительны после изменения размера.
    class SR_COMMON_DLL_API SR_NODISCARD ArrayView {
    public:
        ArrayView() = default;
        explicit ArrayView(const ValueRef& array) noexcept;

        SR_NODISCARD bool IsValid() const noexcept { return m_pVTable; }
        SR_NODISCARD explicit operator bool() const noexcept { return IsValid(); }
        SR_NODISCARD bool IsConst() const noexcept { return m_array.IsConst(); }
        SR_NODISCARD const TypeInfo* GetElementType() const noexcept;

        SR_NODISCARD SizeType Size() const;
        SR_NODISCARD bool Empty() const { return Size() == 0; }
        SR_NODISCARD ValueRef At(SizeType index) const;
        SR_NODISCARD ValueRef Back() const;

        void Resize(SizeType size) const;
        void Reserve(SizeType capacity) const;
        void Clear() const;

        /// Значение копируется, его тип должен совпадать с типом элемента
        bool Insert(SizeType index, const ValueRef& value) const;
        bool PushBack(const ValueRef& value) const;
        bool PushFront(const ValueRef& value) const;
        void Erase(SizeType index) const;

        /// Переносит элемент на позицию to, сдвигая остальные
        void Move(SizeType from, SizeType to) const;

    private:
        SR_NODISCARD bool CanModify() const;

    private:
        ValueRef m_array;
        const ArrayVTable* m_pVTable = nullptr;

    };

    /// ----------------------------------------------------------------------------------------------------------------

    /// Map, FlatHashMap, Set или FlatHashSet. У множеств нет значения: Find возвращает сам ключ, а ForEach передаёт пустое значение.
    class SR_COMMON_DLL_API SR_NODISCARD MapView {
        template<typename Fn> struct ForEachContext {
            const TypeInfo* pKeyType;
            const TypeInfo* pValueType;
            bool isConst;
            std::remove_reference_t<Fn>* pFunction;

            static bool Visit(void* pUserData, const void* pKey, void* pValue);
        };
    public:
        MapView() = default;
        explicit MapView(const ValueRef& map) noexcept;

        SR_NODISCARD bool IsValid() const noexcept { return m_pVTable; }
        SR_NODISCARD explicit operator bool() const noexcept { return IsValid(); }
        SR_NODISCARD bool IsConst() const noexcept { return m_map.IsConst(); }
        SR_NODISCARD bool IsSet() const noexcept { return !m_pValueType; }
        SR_NODISCARD const TypeInfo* GetKeyType() const noexcept { return m_pKeyType; }
        SR_NODISCARD const TypeInfo* GetValueType() const noexcept { return m_pValueType; }

        SR_NODISCARD SizeType Size() const;
        SR_NODISCARD bool Empty() const { return Size() == 0; }
        void Clear() const;

        SR_NODISCARD bool Contains(const ValueRef& key) const;
        /// Пустая ссылка, если ключа нет
        SR_NODISCARD ValueRef Find(const ValueRef& key) const;
        /// Добавляет значение по умолчанию, если ключа нет
        ValueRef FindOrAdd(const ValueRef& key) const;
        /// true, если ключ добавлен. У множеств value не используется
        bool InsertOrAssign(const ValueRef& key, const ValueRef& value) const;
        bool Erase(const ValueRef& key) const;

        /// fn(const ValueRef& key, const ValueRef& value) может вернуть false, чтобы остановить обход.
        /// Менять контейнер во время обхода нельзя.
        template<typename Fn> void ForEach(Fn&& fn) const;

    private:
        SR_NODISCARD bool CanModify() const;
        SR_NODISCARD bool IsKey(const ValueRef& key) const;

        void ForEachRaw(bool(*pVisit)(void* pUserData, const void* pKey, void* pValue), void* pUserData) const;

    private:
        ValueRef m_map;
        const MapVTable* m_pVTable = nullptr;
        const TypeInfo* m_pKeyType = nullptr;
        const TypeInfo* m_pValueType = nullptr;

    };

    /// ----------------------------------------------------------------------------------------------------------------

    template<typename T> ValueRef ValueRef::Of(T& value) {
        return ValueRef(TypeOf<T>(), const_cast<std::remove_const_t<T>*>(&value), std::is_const_v<T>);
    }

    template<typename Fn> bool MapView::ForEachContext<Fn>::Visit(void* pUserData, const void* pKey, void* pValue) {
        auto&& context = *static_cast<ForEachContext*>(pUserData);
        const ValueRef key(context.pKeyType, const_cast<void*>(pKey), true);
        const ValueRef value = pValue ? ValueRef(context.pValueType, pValue, context.isConst) : ValueRef();

        if constexpr (std::is_same_v<std::invoke_result_t<std::remove_reference_t<Fn>&, const ValueRef&, const ValueRef&>, bool>) {
            return (*context.pFunction)(key, value);
        }
        else {
            (*context.pFunction)(key, value);
            return true;
        }
    }

    template<typename Fn> void MapView::ForEach(Fn&& fn) const {
        ForEachContext<Fn> context = { m_pKeyType, m_pValueType, IsConst(), &fn };
        ForEachRaw(&ForEachContext<Fn>::Visit, &context);
    }
}

#endif //SR_ENGINE_UTILS_REFLECTION_VALUE_REF_H
