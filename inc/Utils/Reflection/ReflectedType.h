//
// Created by Monika on 21.07.2026.
//

#ifndef SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
#define SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Reflection/VTable.h>
#include <Utils/TypeTraits/SRClass.h>
#include <Utils/TypeTraits/SRClassMeta.h>
#include <Utils/TypeTraits/TypeTraits.h>
#include <Utils/Math/Size.h>
#include <Utils/Math/Vector6.h>
#include <Utils/Math/Vector4.h>

namespace SR_HTYPES_NS {
    class SharedPtrBase;
}

#define SR_REFLECTION_MODULE_ID (&SR_UTILS_NS::Reflection::g_reflectionModuleTag)

namespace SR_UTILS_NS {
    class EntityRefBase;
    class OptionalBase;
    class ResourceRefBase;
    template<typename T> class Vector;
    template<typename T> class Optional;
    template<typename T> class EntityRef;
    template<typename T> class ResourceRef;
    template<typename T1, typename T2> struct Pair;
    class UnicodeString;
    class Path;

    template<typename>
    inline constexpr bool IsEntityRefV = false;

    template<typename U>
    inline constexpr bool IsEntityRefV<EntityRef<U>> = true;

    template<typename>
    inline constexpr bool IsResourceRefV = false;

    template<typename U>
    inline constexpr bool IsResourceRefV<ResourceRef<U>> = true;
}

namespace SR_MATH_NS {
    template<typename T> struct Vector2;
    template<typename T> struct Vector3;
    template<typename T> struct Vector4;
    template<typename T> struct Vector6;
    template<typename T> struct Rect;
    template<typename T> struct Size;
    template<typename T> struct Size2;
    struct AABB;
    class Quaternion;
    class Matrix3x3;
    class Matrix4x4;
    class FColor;
}

namespace SR_HTYPES_NS {
    template<typename T> class SharedPtr;
    template<typename Key> class FlatHashSet;
    template<typename Key, typename Value> class FlatHashMap;
}

namespace SR_UTILS_NS::Reflection {
    SR_ENUM_NS_CLASS_T(ReflectedCategoryType, uint8_t,
        Unknown,

        Value,

        Object,
        String,
        Arithmetic,
        Enum,

        MathObject,
        MathSize,
        MathVector,
        MathRect,

        Container
    );

    enum class ReflectedValueStorageType : uint8_t {
        Embedded, Dynamic, Reference, ConstReference,
    };

    /// Своя копия в каждом модуле (exe/dll/so): адрес служит идентификатором модуля,
    /// чей код попал в vtable интернированного типа.
#if defined(_WIN32)
    inline const char g_reflectionModuleTag = 0;
#else
    __attribute__((visibility("hidden"))) inline const char g_reflectionModuleTag = 0;
#endif

    /// Конкретный вид типа. Категория (ReflectedCategoryType) выводится из вида и нужна только для группировки в редакторе.
    enum class TypeKind : uint8_t {
        Unknown,
        Value,

        Bool, Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64, Float, Double,
        Enum,

        String, StringView, StringAtom, Path, UnicodeString,

        Object,

        /// аргумент шаблона - скалярный тип компонента, detailedSize - число компонентов
        MathVector, MathRect, MathSize,
        Color, Quaternion, AABB, Matrix3x3, Matrix4x4,

        /// у Map и FlatHashMap единственный аргумент - Pair<Key, Value>, так же как тип элемента при итерации
        Vector, Set, FlatHashSet, Map, FlatHashMap, Pair, Optional, SharedPtr, EntityRef, ResourceRef,
    };

    SR_NODISCARD constexpr bool IsTypeKindSigned(const TypeKind kind) noexcept {
        return kind == TypeKind::Int8 || kind == TypeKind::Int16 || kind == TypeKind::Int32 || kind == TypeKind::Int64;
    }

    SR_NODISCARD constexpr bool IsTypeKindIntegral(const TypeKind kind) noexcept {
        return kind >= TypeKind::Int8 && kind <= TypeKind::UInt64;
    }

    SR_NODISCARD constexpr bool IsTypeKindFloatingPoint(const TypeKind kind) noexcept {
        return kind == TypeKind::Float || kind == TypeKind::Double;
    }

    SR_NODISCARD constexpr bool IsTypeKindArithmetic(const TypeKind kind) noexcept {
        return kind >= TypeKind::Bool && kind <= TypeKind::Double;
    }

    SR_NODISCARD constexpr bool IsTypeKindString(const TypeKind kind) noexcept {
        return kind >= TypeKind::String && kind <= TypeKind::UnicodeString;
    }

    SR_NODISCARD constexpr bool IsTypeKindMath(const TypeKind kind) noexcept {
        return kind >= TypeKind::MathVector && kind <= TypeKind::Matrix4x4;
    }

    SR_NODISCARD constexpr bool IsTypeKindContainer(const TypeKind kind) noexcept {
        return kind >= TypeKind::Vector && kind <= TypeKind::ResourceRef;
    }

    SR_NODISCARD constexpr bool IsTypeKindMapLike(const TypeKind kind) noexcept {
        return kind == TypeKind::Map || kind == TypeKind::FlatHashMap;
    }

    SR_NODISCARD constexpr bool IsTypeKindSetLike(const TypeKind kind) noexcept {
        return kind == TypeKind::Set || kind == TypeKind::FlatHashSet;
    }

    /// Размер скалярного типа в байтах, 0 для остальных
    SR_NODISCARD constexpr SizeType GetTypeKindScalarSize(const TypeKind kind) noexcept {
        switch (kind) {
            case TypeKind::Bool: return sizeof(bool);
            case TypeKind::Int8: case TypeKind::UInt8: return sizeof(uint8_t);
            case TypeKind::Int16: case TypeKind::UInt16: return sizeof(uint16_t);
            case TypeKind::Int32: case TypeKind::UInt32: return sizeof(uint32_t);
            case TypeKind::Int64: case TypeKind::UInt64: return sizeof(uint64_t);
            case TypeKind::Float: return sizeof(float_t);
            case TypeKind::Double: return sizeof(double_t);
            default: return 0;
        }
    }

    /// Число аргументов шаблона в записи типа: Map<K, V> записывается с двумя, хотя хранит один Pair<K, V>
    SR_NODISCARD constexpr uint32_t GetTypeKindArgumentsCount(const TypeKind kind) noexcept {
        switch (kind) {
            case TypeKind::Vector:
            case TypeKind::Set:
            case TypeKind::FlatHashSet:
            case TypeKind::Optional:
            case TypeKind::SharedPtr:
            case TypeKind::EntityRef:
            case TypeKind::ResourceRef:
                return 1;
            case TypeKind::Map:
            case TypeKind::FlatHashMap:
            case TypeKind::Pair:
                return 2;
            default:
                return 0;
        }
    }

    extern SR_COMMON_DLL_API ReflectedCategoryType GetTypeKindCategory(TypeKind kind);

    constexpr inline SizeType ReflectedValueStorageSize = 64;

    /// Storage копируется побайтно, а std::u32string внутри UnicodeString в libstdc++ хранит указатель на собственный SSO-буфер
    template<typename T> constexpr inline bool IsReflectedValueEmbeddable =
        sizeof(T) <= ReflectedValueStorageSize && !std::is_same_v<T, SR_UTILS_NS::UnicodeString>;

    struct SR_COMMON_DLL_API ReflectedValue {
        struct Storage { alignas(std::max_align_t) std::byte data[ReflectedValueStorageSize]; };
        static ReflectedValue MakeFromPointer(void* pData, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromNumeric(uint64_t numeric, ReflectedValueStorageType storageType);
        static ReflectedValue MakeFromInlineData(const Storage& inlineData, ReflectedValueStorageType storageType);

        SR_NODISCARD int64_t& GetNumeric();
        SR_NODISCARD int64_t GetNumeric() const;
        SR_NODISCARD const void* GetData() const;
        SR_NODISCARD void* GetData();

        void SetData(void* pData);

        Storage storage = {};
        ReflectedValueStorageType storageType = ReflectedValueStorageType::Embedded;
    };

    class Value;
    struct TypeInfo;

    /// Структурное описание типа, по которому реестр находит или создаёт единственный TypeInfo.
    /// Вложенные типы уже интернированы, поэтому сравниваются по указателю.
    struct SR_COMMON_DLL_API TypeInfoKey {
        TypeKind kind = TypeKind::Unknown;
        uint8_t detailedSize = 0;
        /// имя enum или SRClass, у остальных видов не используется
        StringAtom name;
        const TypeInfo* pNext[2] = { nullptr, nullptr };

        SR_NODISCARD SRHashType GetHash() const noexcept;

        bool operator==(const TypeInfoKey& other) const noexcept;
        bool operator!=(const TypeInfoKey& other) const noexcept;
    };

    /// Создаётся только реестром типов: один экземпляр на тип, не освобождается до завершения процесса,
    /// поэтому указатель можно хранить без копирования, а типы сравнивать по адресу.
    /// Тип без vtable (не разрешён) остаётся валидным: так описываются типы, собираемые в редакторе по частям,
    /// и типы выгруженного модуля - их vtable заполняется повторно при следующей регистрации.
    struct SR_COMMON_DLL_API TypeInfo {
        TypeInfo() = default;
        TypeInfo(const TypeInfo&) = delete;
        TypeInfo& operator=(const TypeInfo&) = delete;

        TypeKind kind = TypeKind::Unknown;
        ReflectedCategoryType category = ReflectedCategoryType::Unknown;
        uint8_t detailedSize = 0;
        /// каноническое имя: "float", "FVector3", "KeyCode", "Map<String, int32>"; оно же пишется при сериализации
        StringAtom name;
        const TypeInfo* pNext[2] = { nullptr, nullptr };
        TypeInfoVTable vtable;
        SRHashType hash = 0;
        const void* pOwnerModule = nullptr;
        std::atomic<bool> isResolved = false;

        SR_NODISCARD SRHashType GetHash() const noexcept { return hash; }
        SR_NODISCARD bool IsResolved() const noexcept { return isResolved.load(std::memory_order_acquire); }
        SR_NODISCARD TypeInfoKey GetKey() const noexcept;

        SR_NODISCARD bool IsSigned() const noexcept { return IsTypeKindSigned(kind); }
        SR_NODISCARD bool IsIntegral() const noexcept { return IsTypeKindIntegral(kind); }
        SR_NODISCARD bool IsFloatingPoint() const noexcept { return IsTypeKindFloatingPoint(kind); }
        SR_NODISCARD bool IsArithmetic() const noexcept { return IsTypeKindArithmetic(kind); }
        SR_NODISCARD bool IsString() const noexcept { return IsTypeKindString(kind); }
        SR_NODISCARD bool IsMath() const noexcept { return IsTypeKindMath(kind); }
        SR_NODISCARD bool IsContainer() const noexcept { return IsTypeKindContainer(kind); }
        SR_NODISCARD bool IsMapLike() const noexcept { return IsTypeKindMapLike(kind); }
        SR_NODISCARD bool IsSetLike() const noexcept { return IsTypeKindSetLike(kind); }
        SR_NODISCARD SizeType GetScalarSize() const noexcept { return GetTypeKindScalarSize(kind); }

        /// Аргумент шаблона в порядке записи типа: для Map<K, V> это K и V, а не хранимый Pair<K, V>
        SR_NODISCARD const TypeInfo* GetArgument(uint32_t index) const noexcept;

        bool operator==(const TypeInfo& other) const noexcept { return this == &other; }
        bool operator!=(const TypeInfo& other) const noexcept { return this != &other; }
    };

    /// Тип без категории, его возвращает Value без значения
    extern SR_COMMON_DLL_API const TypeInfo* GetUnknownTypeInfo();

    /// pOwnerModule - модуль, чей код записан в vtable (SR_REFLECTION_MODULE_ID в месте вызова).
    /// Уже разрешённый тип не перезаписывается, нерешённый получает переданную vtable.
    extern SR_COMMON_DLL_API const TypeInfo* InternTypeInfo(const TypeInfoKey& key, const TypeInfoVTable& vtable, const void* pOwnerModule);

    /// Возвращает интернированный тип; enum и SRClass разрешаются по имени, остальные типы должны быть
    /// зарегистрированы через TypeOf (базовые - в InitReflection).
    /// Не возвращает nullptr: неизвестный тип интернируется без vtable, проверяйте IsResolved().
    extern SR_COMMON_DLL_API const TypeInfo* ResolveTypeInfo(const TypeInfoKey& key);

    /// Шаблонный тип по аргументам в порядке записи; для Map/FlatHashMap сам оборачивает аргументы в Pair
    extern SR_COMMON_DLL_API const TypeInfo* ResolveTemplateTypeInfo(TypeKind kind, const TypeInfo* pFirst, const TypeInfo* pSecond = nullptr);

    /// Тип по каноническому имени из уже интернированных, enum или SRClass. nullptr, если имя неизвестно
    extern SR_COMMON_DLL_API const TypeInfo* FindTypeInfoByName(StringAtom name);

    /// Встроенный скалярный тип по виду, nullptr для остальных видов
    extern SR_COMMON_DLL_API const TypeInfo* GetScalarTypeInfo(TypeKind kind);

    /// Имя шаблона без аргументов ("Vector", "Map"), пустое для нешаблонных видов
    extern SR_COMMON_DLL_API StringAtom GetTypeKindTemplateName(TypeKind kind);
    extern SR_COMMON_DLL_API TypeKind FindTypeKindByTemplateName(StringView name);

    /// Выбор типа в редакторе: категория -> имя пункта -> размер -> аргументы шаблона
    extern SR_COMMON_DLL_API void GetTypeNamesByCategory(ReflectedCategoryType category, Vector<StringAtom>& outNames);
    extern SR_COMMON_DLL_API void GetTypeDetailedSizes(ReflectedCategoryType category, Vector<uint8_t>& outSizes);
    extern SR_COMMON_DLL_API StringAtom GetTypeCategoryItemName(const TypeInfo& typeInfo);
    extern SR_COMMON_DLL_API const TypeInfo* ResolveTypeInfoByCategory(ReflectedCategoryType category, StringAtom itemName, uint8_t detailedSize);

    /// Снимает vtable со всех типов, код которых принадлежит выгружаемому модулю
    extern SR_COMMON_DLL_API void UnregisterModuleTypes(const void* pOwnerModule);

    /// Снимает vtable со всех типов, содержащих SRClass с указанным именем
    extern SR_COMMON_DLL_API void UnregisterObjectTypes(StringAtom className);

    extern SR_COMMON_DLL_API uint64_t GetTypeInfoCount();
    extern SR_COMMON_DLL_API void InitReflection();

    template<typename T, typename Enable = void> struct DetermineTypeInfoAccessor {
        static constexpr bool Supported = false;
        static void Determine(TypeInfoKey&, TypeInfoVTable&) { static_assert(AlwaysFalseV<T>, "Unable to determine type info for type!"); }
    };

    template<typename T> constexpr bool IsDetermineTypeInfoSupportedV = DetermineTypeInfoAccessor<T>::Supported;

    template<typename T> const TypeInfo* InternTypeOf() {
        TypeInfoKey key;
        TypeInfoVTable vtable;
        DetermineTypeInfoAccessor<T>::Determine(key, vtable);
        return InternTypeInfo(key, vtable, SR_REFLECTION_MODULE_ID);
    }

    template<typename T> const TypeInfo* TypeOf() {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        static const TypeInfo* const pTypeInfo = InternTypeOf<Type>();
        if (!pTypeInfo->IsResolved()) SR_UNLIKELY_ATTRIBUTE {
            /// тип мог быть снят при выгрузке модуля, который зарегистрировал его первым
            InternTypeOf<Type>();
        }
        return pTypeInfo;
    }

    template<typename T> const TypeInfo* TypeOf(const T&) {
        return TypeOf<T>();
    }

    template<typename T> Pair<SizeType, SizeType> ReflectedTypeTemplateSizeOfAlign(const TypeInfo&) {
        return { sizeof(T), alignof(T) };
    }

}

#include <Utils/Reflection/CommonReflection.inl.h>
#include <Utils/Reflection/VectorReflection.inl.h>
#include <Utils/Reflection/AssociaticeReflection.inl.h>

#endif //SR_ENGINE_COMMON_REFLECTION_REFLECTED_TYPE_H
