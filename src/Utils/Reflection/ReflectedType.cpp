//
// Created by Monika on 21.07.2026.
//

#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Types/Vector.h>
#include <Utils/Types/Optional.h>
#include <Utils/Types/UnicodeString.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/TypeTraits/Factory.h>
#include <Utils/Common/EnumReflector.h>

#include <Enum/ReflectedCategoryType.hpp>
#include <Enum/SizeMetric.hpp>

namespace SR_UTILS_NS::Reflection {
    ReflectedValue ReflectedValue::MakeFromPointer(void* pData, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.SetData(pData);
        value.storageType = storageType;
        return value;
    }

    ReflectedValue ReflectedValue::MakeFromNumeric(uint64_t numeric, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.GetNumeric() = static_cast<int64_t>(numeric);
        value.storageType = storageType;
        return value;
    }

    ReflectedValue ReflectedValue::MakeFromInlineData(const ReflectedValue::Storage& inlineData, ReflectedValueStorageType storageType) {
        ReflectedValue value;
        value.storage = inlineData;
        value.storageType = storageType;
        return value;
    }

    int64_t& ReflectedValue::GetNumeric() {
        return *std::launder(reinterpret_cast<int64_t*>(storage.data));
    }

    int64_t ReflectedValue::GetNumeric() const {
        return *std::launder(reinterpret_cast<const int64_t*>(storage.data));
    }

    const void* ReflectedValue::GetData() const {
        if (storageType == ReflectedValueStorageType::Dynamic || storageType == ReflectedValueStorageType::Reference || storageType == ReflectedValueStorageType::ConstReference) {
            return *std::launder(reinterpret_cast<void* const*>(storage.data));
        }
        return &storage.data[0];
    }

    void* ReflectedValue::GetData() {
        if (storageType == ReflectedValueStorageType::Dynamic || storageType == ReflectedValueStorageType::Reference || storageType == ReflectedValueStorageType::ConstReference) {
            return *std::launder(reinterpret_cast<void**>(storage.data));
        }
        return &storage.data[0];
    }

    void ReflectedValue::SetData(void* pData) {
        *std::launder(reinterpret_cast<void**>(storage.data)) = pData;
    }

    /// ================================================================================================================

    ReflectedCategoryType GetTypeKindCategory(const TypeKind kind) {
        if (IsTypeKindArithmetic(kind)) {
            return ReflectedCategoryType::Arithmetic;
        }
        if (IsTypeKindString(kind)) {
            return ReflectedCategoryType::String;
        }
        if (IsTypeKindContainer(kind)) {
            return ReflectedCategoryType::Container;
        }

        switch (kind) {
            case TypeKind::Value: return ReflectedCategoryType::Value;
            case TypeKind::Enum: return ReflectedCategoryType::Enum;
            case TypeKind::Object: return ReflectedCategoryType::Object;
            case TypeKind::ScriptObject:
            case TypeKind::ScriptHandle: return ReflectedCategoryType::Object;
            case TypeKind::NativeHandle: return ReflectedCategoryType::Object;
            case TypeKind::MathVector: return ReflectedCategoryType::MathVector;
            case TypeKind::MathRect: return ReflectedCategoryType::MathRect;
            case TypeKind::MathSize: return ReflectedCategoryType::MathSize;
            case TypeKind::Color:
            case TypeKind::Quaternion:
            case TypeKind::AABB:
            case TypeKind::Matrix3x3:
            case TypeKind::Matrix4x4:
                return ReflectedCategoryType::MathObject;
            default:
                return ReflectedCategoryType::Unknown;
        }
    }

    /// Имя нешаблонного встроенного вида, пустое для enum, SRClass, математических шаблонов и контейнеров
    StringView GetTypeKindBuiltinName(const TypeKind kind) {
        switch (kind) {
            case TypeKind::Unknown: return "Unknown";
            case TypeKind::Value: return "Value";
            case TypeKind::Bool: return "bool";
            case TypeKind::Int8: return "int8";
            case TypeKind::Int16: return "int16";
            case TypeKind::Int32: return "int32";
            case TypeKind::Int64: return "int64";
            case TypeKind::UInt8: return "uint8";
            case TypeKind::UInt16: return "uint16";
            case TypeKind::UInt32: return "uint32";
            case TypeKind::UInt64: return "uint64";
            case TypeKind::Float: return "float";
            case TypeKind::Double: return "double";
            case TypeKind::String: return "String";
            case TypeKind::StringView: return "StringView";
            case TypeKind::StringAtom: return "StringAtom";
            case TypeKind::Path: return "Path";
            case TypeKind::UnicodeString: return "UnicodeString";
            case TypeKind::Color: return "FColor";
            case TypeKind::Quaternion: return "Quaternion";
            case TypeKind::AABB: return "AABB";
            case TypeKind::Matrix3x3: return "Matrix3x3";
            case TypeKind::Matrix4x4: return "Matrix4x4";
            default: return StringView();
        }
    }

    StringAtom GetTypeKindTemplateName(const TypeKind kind) {
        static const StringAtom vectorName = "Vector";
        static const StringAtom setName = "Set";
        static const StringAtom flatHashSetName = "FlatHashSet";
        static const StringAtom mapName = "Map";
        static const StringAtom flatHashMapName = "FlatHashMap";
        static const StringAtom pairName = "Pair";
        static const StringAtom optionalName = "Optional";
        static const StringAtom sharedPtrName = "SharedPtr";
        static const StringAtom entityRefName = "EntityRef";
        static const StringAtom resourceRefName = "ResourceRef";

        switch (kind) {
            case TypeKind::Vector: return vectorName;
            case TypeKind::Set: return setName;
            case TypeKind::FlatHashSet: return flatHashSetName;
            case TypeKind::Map: return mapName;
            case TypeKind::FlatHashMap: return flatHashMapName;
            case TypeKind::Pair: return pairName;
            case TypeKind::Optional: return optionalName;
            case TypeKind::SharedPtr: return sharedPtrName;
            case TypeKind::EntityRef: return entityRefName;
            case TypeKind::ResourceRef: return resourceRefName;
            default: return StringAtom();
        }
    }

    TypeKind FindTypeKindByTemplateName(const StringView name) {
        for (auto kind = static_cast<uint8_t>(TypeKind::Vector); kind <= static_cast<uint8_t>(TypeKind::ResourceRef); ++kind) {
            if (GetTypeKindTemplateName(static_cast<TypeKind>(kind)) == name) {
                return static_cast<TypeKind>(kind);
            }
        }
        return TypeKind::Unknown;
    }

    struct MathTypePrefix {
        TypeKind scalar;
        StringView prefix;
    };

    /// Совпадает с псевдонимами из Utils/Math: FVector3, USVector2, IRect, USize2 и т.д.
    static constexpr MathTypePrefix MATH_TYPE_PREFIXES[] = {
        { TypeKind::Float, "F" },
        { TypeKind::Double, "D" },
        { TypeKind::Int16, "S" },
        { TypeKind::Int32, "I" },
        { TypeKind::Int64, "L" },
        { TypeKind::UInt16, "US" },
        { TypeKind::UInt32, "U" },
        { TypeKind::UInt64, "UL" },
        { TypeKind::Bool, "B" },
    };

    StringView GetMathTypePrefix(const TypeInfo* pScalar) {
        if (!pScalar) {
            return StringView();
        }
        for (auto&& prefix : MATH_TYPE_PREFIXES) {
            if (prefix.scalar == pScalar->kind) {
                return prefix.prefix;
            }
        }
        return StringView();
    }

    /// Разбирает имена математических типов вида FVector3, USRect, ISize2
    bool ParseMathTypeName(const StringView name, TypeInfoKey& key) {
        for (auto&& prefix : MATH_TYPE_PREFIXES) {
            if (name.size() <= prefix.prefix.size() || name.substr(0, prefix.prefix.size()) != prefix.prefix) {
                continue;
            }

            const StringView rest = name.substr(prefix.prefix.size());

            if (rest == "Rect") {
                key.kind = TypeKind::MathRect;
            }
            else if (rest == "Size") {
                key.kind = TypeKind::MathSize;
                key.detailedSize = 1;
            }
            else if (rest == "Size2") {
                key.kind = TypeKind::MathSize;
                key.detailedSize = 2;
            }
            else if (rest.size() == 7 && rest.substr(0, 6) == "Vector" && rest[6] >= '2' && rest[6] <= '6') {
                key.kind = TypeKind::MathVector;
                key.detailedSize = static_cast<uint8_t>(rest[6] - '0');
            }
            else {
                continue;
            }

            key.pNext[0] = GetScalarTypeInfo(prefix.scalar);
            return true;
        }

        return false;
    }

    String BuildTypeName(const TypeInfoKey& key) {
        if (key.kind == TypeKind::Enum || key.kind == TypeKind::Object || key.kind == TypeKind::ScriptObject || key.kind == TypeKind::ScriptHandle || key.kind == TypeKind::NativeHandle || key.kind == TypeKind::ScriptEnum) {
            return String(key.name.ToStringView());
        }

        if (IsTypeKindContainer(key.kind)) {
            String name(GetTypeKindTemplateName(key.kind).ToStringView());
            name += '<';

            const TypeInfo* pArguments[2] = { key.pNext[0], key.pNext[1] };
            if (IsTypeKindMapLike(key.kind) && key.pNext[0] && key.pNext[0]->kind == TypeKind::Pair) {
                pArguments[0] = key.pNext[0]->pNext[0];
                pArguments[1] = key.pNext[0]->pNext[1];
            }

            for (uint32_t i = 0; i < 2; ++i) {
                if (!pArguments[i]) {
                    continue;
                }
                if (i > 0) {
                    name += ", ";
                }
                name += pArguments[i]->name.ToStringView();
            }

            name += '>';
            return name;
        }

        if (key.kind == TypeKind::MathVector || key.kind == TypeKind::MathRect || key.kind == TypeKind::MathSize) {
            const StringView prefix = GetMathTypePrefix(key.pNext[0]);
            String name(prefix.empty() ? StringView("?") : prefix);

            if (key.kind == TypeKind::MathVector) {
                FormatTo(name, "Vector{}", key.detailedSize);
            }
            else if (key.kind == TypeKind::MathRect) {
                name += "Rect";
            }
            else {
                name += key.detailedSize == 2 ? "Size2" : "Size";
            }

            return name;
        }

        return String(GetTypeKindBuiltinName(key.kind));
    }

    /// ================================================================================================================

    SRHashType TypeInfoKey::GetHash() const noexcept {
        SRHashType hash = HashCombine(static_cast<uint64_t>(kind));
        hash = HashCombine(static_cast<uint64_t>(detailedSize), hash);
        hash = HashCombine(name.GetHash(), hash);
        for (auto&& pNextType : pNext) {
            hash = HashCombine(reinterpret_cast<uintptr_t>(pNextType), hash);
        }
        return hash;
    }

    bool TypeInfoKey::operator==(const TypeInfoKey& other) const noexcept {
        return kind == other.kind &&
               detailedSize == other.detailedSize &&
               name == other.name &&
               pNext[0] == other.pNext[0] &&
               pNext[1] == other.pNext[1];
    }

    bool TypeInfoKey::operator!=(const TypeInfoKey& other) const noexcept {
        return !(*this == other);
    }

    TypeInfoKey TypeInfo::GetKey() const noexcept {
        TypeInfoKey key;
        key.kind = kind;
        key.detailedSize = detailedSize;
        if (kind == TypeKind::Enum || kind == TypeKind::Object || kind == TypeKind::ScriptObject || kind == TypeKind::ScriptHandle || kind == TypeKind::NativeHandle || kind == TypeKind::ScriptEnum) {
            key.name = name;
        }
        key.pNext[0] = pNext[0];
        key.pNext[1] = pNext[1];
        return key;
    }

    const TypeInfo* TypeInfo::GetArgument(const uint32_t index) const noexcept {
        if (index >= 2) {
            return nullptr;
        }
        if (IsMapLike()) {
            return pNext[0] ? pNext[0]->pNext[index] : nullptr;
        }
        return pNext[index];
    }

    /// ================================================================================================================

    struct TypeInfoRegistry {
        TypeInfoRegistry() {
            unknownType.name = GetTypeKindBuiltinName(TypeKind::Unknown);
            byName[unknownType.name] = &unknownType;
        }

        std::mutex mutex;
        SR_HTYPES_NS::FlatHashMap<SRHashType, Vector<TypeInfo*>> buckets;
        SR_HTYPES_NS::FlatHashMap<StringAtom, TypeInfo*> byName;
        Vector<TypeInfo*> types;
        TypeInfo unknownType;
    };

    TypeInfoRegistry& GetTypeInfoRegistry() {
        /// намеренно не уничтожается: интернированные типы используются вплоть до статических деструкторов
        static TypeInfoRegistry* pRegistry = new TypeInfoRegistry();
        return *pRegistry;
    }

    TypeInfoKey NormalizeTypeInfoKey(const TypeInfoKey& key) {
        TypeInfoKey normalized = key;
        if (key.kind != TypeKind::Enum && key.kind != TypeKind::Object && key.kind != TypeKind::ScriptObject && key.kind != TypeKind::ScriptHandle && key.kind != TypeKind::NativeHandle && key.kind != TypeKind::ScriptEnum) {
            normalized.name = StringAtom();
        }
        return normalized;
    }

    TypeInfo* FindTypeInfoLocked(TypeInfoRegistry& registry, const TypeInfoKey& key, const SRHashType hash) {
        auto&& pIt = registry.buckets.find(hash);
        if (pIt == registry.buckets.end()) {
            return nullptr;
        }

        for (auto&& pTypeInfo : pIt->second) {
            if (pTypeInfo->GetKey() == key) {
                return pTypeInfo;
            }
        }

        return nullptr;
    }

    TypeInfo* GetOrCreateTypeInfoLocked(TypeInfoRegistry& registry, const TypeInfoKey& key) {
        const SRHashType hash = key.GetHash();
        if (auto&& pExisting = FindTypeInfoLocked(registry, key, hash)) {
            return pExisting;
        }

        auto&& pTypeInfo = new TypeInfo();
        pTypeInfo->kind = key.kind;
        pTypeInfo->detailedSize = key.detailedSize;
        pTypeInfo->name = BuildTypeName(key);
        pTypeInfo->pNext[0] = key.pNext[0];
        pTypeInfo->pNext[1] = key.pNext[1];
        pTypeInfo->hash = hash;

        registry.buckets[hash].emplace_back(pTypeInfo);
        registry.types.emplace_back(pTypeInfo);

        /// enum и SRClass с одинаковым именем: по имени находится первый зарегистрированный
        if (registry.byName.find(pTypeInfo->name) == registry.byName.end()) {
            registry.byName[pTypeInfo->name] = pTypeInfo;
        }

        return pTypeInfo;
    }

    void ResolveTypeInfoLocked(TypeInfo& typeInfo, const TypeInfoVTable& vtable, const void* pOwnerModule) {
        if (!typeInfo.pFields && vtable.fieldsCount > 0) {
            auto&& pFields = new FieldInfo[vtable.fieldsCount];
            for (uint32_t i = 0; i < vtable.fieldsCount; ++i) {
                pFields[i] = vtable.pFields[i];
            }
            typeInfo.pFields = pFields;
            typeInfo.fieldsCount = vtable.fieldsCount;
        }
        typeInfo.vtable = vtable;
        typeInfo.vtable.pFields = typeInfo.pFields;
        typeInfo.pOwnerModule = pOwnerModule;
        typeInfo.isResolved.store(true, std::memory_order_release);
    }

    void UnresolveTypeInfoLocked(TypeInfo& typeInfo) {
        typeInfo.isResolved.store(false, std::memory_order_release);
        const auto size = typeInfo.vtable.lifecycle.size;
        const auto alignment = typeInfo.vtable.lifecycle.alignment;
        typeInfo.vtable = TypeInfoVTable();
        typeInfo.vtable.lifecycle.size = size;
        typeInfo.vtable.lifecycle.alignment = alignment;
        typeInfo.pOwnerModule = nullptr;
    }

    bool IsTypeInfoContainsObject(const TypeInfo* pTypeInfo, const StringAtom className) { /// NOLINT(misc-no-recursion)
        if (!pTypeInfo) {
            return false;
        }
        if (pTypeInfo->kind == TypeKind::Object && pTypeInfo->name == className) {
            return true;
        }
        return IsTypeInfoContainsObject(pTypeInfo->pNext[0], className) || IsTypeInfoContainsObject(pTypeInfo->pNext[1], className);
    }

    const TypeInfo* GetUnknownTypeInfo() {
        return &GetTypeInfoRegistry().unknownType;
    }

    const TypeInfo* InternTypeInfo(const TypeInfoKey& key, const TypeInfoVTable& vtable, const void* pOwnerModule) {
        const TypeInfoKey normalized = NormalizeTypeInfoKey(key);

        auto&& registry = GetTypeInfoRegistry();
        std::lock_guard lock(registry.mutex);

        auto&& pTypeInfo = GetOrCreateTypeInfoLocked(registry, normalized);
        if (!pTypeInfo->IsResolved()) {
            if (normalized.kind == TypeKind::Vector) {
                TypeInfoVTable builtin;
                if (ResolveBuiltinVTable(normalized, builtin)) {
                    ResolveTypeInfoLocked(*pTypeInfo, builtin, SR_REFLECTION_MODULE_ID);
                }
            }
            else {
                ResolveTypeInfoLocked(*pTypeInfo, vtable, pOwnerModule);
            }
        }

        return pTypeInfo;
    }

    const TypeInfo* ResolveTypeInfo(const TypeInfoKey& key) {
        if (key.kind == TypeKind::Unknown) {
            return GetUnknownTypeInfo();
        }

        const TypeInfoKey normalized = NormalizeTypeInfoKey(key);

        auto&& registry = GetTypeInfoRegistry();
        std::lock_guard lock(registry.mutex);

        auto&& pTypeInfo = GetOrCreateTypeInfoLocked(registry, normalized);
        if (!pTypeInfo->IsResolved()) {
            TypeInfoVTable vtable;
            if (ResolveBuiltinVTable(normalized, vtable)) {
                ResolveTypeInfoLocked(*pTypeInfo, vtable, SR_REFLECTION_MODULE_ID);
            }
        }

        return pTypeInfo;
    }

    const TypeInfo* ResolveTemplateTypeInfo(const TypeKind kind, const TypeInfo* pFirst, const TypeInfo* pSecond) {
        const uint32_t argumentsCount = GetTypeKindArgumentsCount(kind);
        if (argumentsCount == 0) {
            SRHalt("ResolveTemplateTypeInfo() : type kind {} is not a template!", static_cast<uint32_t>(kind));
            return GetUnknownTypeInfo();
        }

        TypeInfoKey key;
        key.kind = kind;

        if (IsTypeKindMapLike(kind)) {
            key.pNext[0] = ResolveTemplateTypeInfo(TypeKind::Pair, pFirst, pSecond);
        }
        else {
            key.pNext[0] = pFirst ? pFirst : GetUnknownTypeInfo();
            if (argumentsCount > 1) {
                key.pNext[1] = pSecond ? pSecond : GetUnknownTypeInfo();
            }
        }

        return ResolveTypeInfo(key);
    }

    const TypeInfo* FindTypeInfoByName(const StringAtom name) {
        if (name.empty()) {
            return nullptr;
        }

        auto&& registry = GetTypeInfoRegistry();

        const TypeInfo* pFound = nullptr;
        {
            std::lock_guard lock(registry.mutex);
            if (auto&& pIt = registry.byName.find(name); pIt != registry.byName.end()) {
                pFound = pIt->second;
            }
        }

        if (pFound) {
            /// enum или SRClass мог быть снят при выгрузке модуля, повторное разрешение вернёт ему vtable
            if (!pFound->IsResolved() && (pFound->kind == TypeKind::Enum || pFound->kind == TypeKind::Object)) {
                return ResolveTypeInfo(pFound->GetKey());
            }
            return pFound;
        }

        TypeInfoKey key;
        if (EnumReflectorManager::Instance().TryGetReflector(name)) {
            key.kind = TypeKind::Enum;
            key.name = name;
        }
        else if (Factory::Instance().GetType(name)) {
            key.kind = TypeKind::Object;
            key.name = name;
        }
        else if (!ParseMathTypeName(name.ToStringView(), key)) {
            return nullptr;
        }

        return ResolveTypeInfo(key);
    }

    const TypeInfo* GetScalarTypeInfo(const TypeKind kind) {
        switch (kind) {
            case TypeKind::Bool: return TypeOf<bool>();
            case TypeKind::Int8: return TypeOf<int8_t>();
            case TypeKind::Int16: return TypeOf<int16_t>();
            case TypeKind::Int32: return TypeOf<int32_t>();
            case TypeKind::Int64: return TypeOf<int64_t>();
            case TypeKind::UInt8: return TypeOf<uint8_t>();
            case TypeKind::UInt16: return TypeOf<uint16_t>();
            case TypeKind::UInt32: return TypeOf<uint32_t>();
            case TypeKind::UInt64: return TypeOf<uint64_t>();
            case TypeKind::Float: return TypeOf<float_t>();
            case TypeKind::Double: return TypeOf<double_t>();
            default: return nullptr;
        }
    }

    void UnregisterModuleTypes(const void* pOwnerModule) {
        SR_TRACY_ZONE;

        auto&& registry = GetTypeInfoRegistry();
        std::lock_guard lock(registry.mutex);

        for (auto&& pTypeInfo : registry.types) {
            if (pTypeInfo->IsResolved() && pTypeInfo->kind != TypeKind::Vector && pTypeInfo->pOwnerModule == pOwnerModule) {
                UnresolveTypeInfoLocked(*pTypeInfo);
            }
        }
    }

    void UnregisterObjectTypes(const StringAtom className) {
        SR_TRACY_ZONE;

        auto&& registry = GetTypeInfoRegistry();
        std::lock_guard lock(registry.mutex);

        for (auto&& pTypeInfo : registry.types) {
            if (pTypeInfo->IsResolved() && pTypeInfo->kind != TypeKind::Vector && IsTypeInfoContainsObject(pTypeInfo, className)) {
                UnresolveTypeInfoLocked(*pTypeInfo);
            }
        }
    }

    uint64_t GetTypeInfoCount() {
        auto&& registry = GetTypeInfoRegistry();
        std::lock_guard lock(registry.mutex);
        return registry.types.size();
    }

    /// ================================================================================================================

    using PreregisterTypeFn = const TypeInfo*(*)();
    PreregisterTypeFn g_preregisteredTypes[] = {
        &TypeOf<Value>,
        &TypeOf<int8_t>,
        &TypeOf<int16_t>,
        &TypeOf<int32_t>,
        &TypeOf<int64_t>,
        &TypeOf<uint8_t>,
        &TypeOf<uint16_t>,
        &TypeOf<uint32_t>,
        &TypeOf<uint64_t>,
        &TypeOf<float>,
        &TypeOf<double>,
        &TypeOf<bool>,
        &TypeOf<Path>,
        &TypeOf<String>,
        &TypeOf<StringView>,
        &TypeOf<StringAtom>,
        &TypeOf<UnicodeString>,
        &TypeOf<SR_MATH_NS::FColor>,
        &TypeOf<SR_MATH_NS::Quaternion>,
        &TypeOf<SR_MATH_NS::AABB>,
        &TypeOf<SR_MATH_NS::Matrix3x3>,
        &TypeOf<SR_MATH_NS::Matrix4x4>,
        &TypeOf<SR_MATH_NS::FRect>,
        &TypeOf<SR_MATH_NS::IRect>,
        &TypeOf<SR_MATH_NS::URect>,
        &TypeOf<SR_MATH_NS::USRect>,
        &TypeOf<SR_MATH_NS::FSize>,
        &TypeOf<SR_MATH_NS::USize>,
        &TypeOf<SR_MATH_NS::ISize>,
        &TypeOf<SR_MATH_NS::FSize2>,
        &TypeOf<SR_MATH_NS::USize2>,
        &TypeOf<SR_MATH_NS::ISize2>,
        &TypeOf<SR_MATH_NS::FVector2>,
        &TypeOf<SR_MATH_NS::FVector3>,
        &TypeOf<SR_MATH_NS::FVector4>,
        &TypeOf<SR_MATH_NS::FVector6>,
        &TypeOf<SR_MATH_NS::SVector2>,
        &TypeOf<SR_MATH_NS::IVector2>,
        &TypeOf<SR_MATH_NS::IVector3>,
        &TypeOf<SR_MATH_NS::IVector4>,
        &TypeOf<SR_MATH_NS::IVector6>,
        &TypeOf<SR_MATH_NS::UVector2>,
        &TypeOf<SR_MATH_NS::USVector2>,
        &TypeOf<SR_MATH_NS::UVector3>,
        &TypeOf<SR_MATH_NS::UVector4>,
        &TypeOf<SR_MATH_NS::UVector6>,
        &TypeOf<SR_MATH_NS::BVector2>,
        &TypeOf<SR_MATH_NS::BVector3>,
        &TypeOf<SR_MATH_NS::BVector4>,
        &TypeOf<SR_MATH_NS::BVector6>,

        &TypeOf<Vector<Path>>,
        &TypeOf<Vector<String>>,
        &TypeOf<Vector<StringAtom>>
    };

    void InitReflection() {
        SR_TRACY_ZONE;

        /// типы, которые могут прийти из сериализованных данных раньше, чем их запросит C++ код
        for (auto&& pPreregister : g_preregisteredTypes) {
            pPreregister();
        }
    }

    /// ================================================================================================================

    void GetTypeNamesByCategory(ReflectedCategoryType category, Vector<StringAtom>& outNames) {
        SR_TRACY_ZONE;
        outNames.clear();
        switch (category) {
            case ReflectedCategoryType::Arithmetic:
                outNames = {
                    "int8", "int16", "int32", "int64",
                    "uint8", "uint16", "uint32", "uint64",
                    "float", "double", "bool"
                };
                break;
            case ReflectedCategoryType::String:
                outNames = {
                    "Path", "String", "StringView", "StringAtom", "UnicodeString"
                };
                break;
            case ReflectedCategoryType::MathVector:
                outNames = {
                    "int32", "int64", "uint32", "uint64", "float", "bool",
                };
                break;
            case ReflectedCategoryType::MathRect:
                outNames = {
                    "float", "int32", "uint32", "uint16"
                };
                break;
            case ReflectedCategoryType::MathSize:
                outNames = {
                    "float", "int32", "uint32"
                };
                break;
            case ReflectedCategoryType::MathObject:
                outNames = {
                    "FColor", "Quaternion", "AABB", "Matrix3x3", "Matrix4x4"
                };
                break;
            case ReflectedCategoryType::Value:
                break;
            case ReflectedCategoryType::Container:
                for (auto kind = static_cast<uint8_t>(TypeKind::Vector); kind <= static_cast<uint8_t>(TypeKind::ResourceRef); ++kind) {
                    outNames.emplace_back(GetTypeKindTemplateName(static_cast<TypeKind>(kind)));
                }
                break;
            case ReflectedCategoryType::Object: {
                auto&& factory = Factory::Instance();
                factory.ForEachClass([&](const SRClassMeta* pMeta) {
                    if (pMeta->IsAbstract()) {
                        return;
                    }
                    outNames.emplace_back(pMeta->GetFactoryName());
                });
                break;
            }
            case ReflectedCategoryType::Enum: {
                auto&& enumReflectorManager = EnumReflectorManager::Instance();
                for (auto&& [name, pReflector] : enumReflectorManager.GetReflectors()) {
                    outNames.emplace_back(name);
                }
                break;
            }
            default:
                break;
        }
    }

    void GetTypeDetailedSizes(ReflectedCategoryType category, Vector<uint8_t>& outSizes) {
        outSizes.clear();
        switch (category) {
            case ReflectedCategoryType::MathVector:
                outSizes.emplace_back(2);
                outSizes.emplace_back(3);
                outSizes.emplace_back(4);
                outSizes.emplace_back(6);
                break;
            case ReflectedCategoryType::MathSize:
                outSizes.emplace_back(1);
                outSizes.emplace_back(2);
                break;
            default:
                break;
        }
    }

    StringAtom GetTypeCategoryItemName(const TypeInfo& typeInfo) {
        switch (typeInfo.kind) {
            case TypeKind::MathVector:
            case TypeKind::MathRect:
            case TypeKind::MathSize:
                return typeInfo.pNext[0] ? typeInfo.pNext[0]->name : StringAtom();
            case TypeKind::Unknown:
            case TypeKind::Value:
                return StringAtom();
            default:
                break;
        }

        if (typeInfo.IsContainer()) {
            return GetTypeKindTemplateName(typeInfo.kind);
        }

        return typeInfo.name;
    }

    const TypeInfo* ResolveTypeInfoByCategory(const ReflectedCategoryType category, const StringAtom itemName, const uint8_t detailedSize) {
        TypeInfoKey key;

        switch (category) {
            case ReflectedCategoryType::Value:
                return TypeOf<Value>();
            case ReflectedCategoryType::Arithmetic:
            case ReflectedCategoryType::String:
            case ReflectedCategoryType::MathObject: {
                auto&& pTypeInfo = FindTypeInfoByName(itemName);
                return pTypeInfo ? pTypeInfo : GetUnknownTypeInfo();
            }
            case ReflectedCategoryType::Enum:
                key.kind = TypeKind::Enum;
                key.name = itemName;
                return ResolveTypeInfo(key);
            case ReflectedCategoryType::Object:
                key.kind = TypeKind::Object;
                key.name = itemName;
                return ResolveTypeInfo(key);
            case ReflectedCategoryType::MathVector:
                key.kind = TypeKind::MathVector;
                break;
            case ReflectedCategoryType::MathRect:
                key.kind = TypeKind::MathRect;
                break;
            case ReflectedCategoryType::MathSize:
                key.kind = TypeKind::MathSize;
                break;
            case ReflectedCategoryType::Container: {
                const TypeKind kind = FindTypeKindByTemplateName(itemName.ToStringView());
                if (kind == TypeKind::Unknown) {
                    return GetUnknownTypeInfo();
                }
                return ResolveTemplateTypeInfo(kind, GetUnknownTypeInfo(), GetUnknownTypeInfo());
            }
            default:
                return GetUnknownTypeInfo();
        }

        key.pNext[0] = FindTypeInfoByName(itemName);
        if (!key.pNext[0]) {
            return GetUnknownTypeInfo();
        }
        if (key.kind != TypeKind::MathRect) {
            key.detailedSize = detailedSize;
        }
        return ResolveTypeInfo(key);
    }

    /// ================================================================================================================

    template<typename T> constexpr TypeKind GetArithmeticTypeKind() {
        if constexpr (std::is_same_v<T, bool>) { return TypeKind::Bool; }
        else if constexpr (std::is_floating_point_v<T> && sizeof(T) == sizeof(float_t)) { return TypeKind::Float; }
        else if constexpr (std::is_floating_point_v<T>) { return TypeKind::Double; }
        else if constexpr (std::is_signed_v<T> && sizeof(T) == 1) { return TypeKind::Int8; }
        else if constexpr (std::is_signed_v<T> && sizeof(T) == 2) { return TypeKind::Int16; }
        else if constexpr (std::is_signed_v<T> && sizeof(T) == 4) { return TypeKind::Int32; }
        else if constexpr (std::is_signed_v<T>) { return TypeKind::Int64; }
        else if constexpr (sizeof(T) == 1) { return TypeKind::UInt8; }
        else if constexpr (sizeof(T) == 2) { return TypeKind::UInt16; }
        else if constexpr (sizeof(T) == 4) { return TypeKind::UInt32; }
        else { return TypeKind::UInt64; }
    }

    template<typename T> void DetermineTypeInfoRegistered(TypeInfoKey& key, TypeInfoVTable& vtable) {
        if constexpr (!std::is_arithmetic_v<T>) {
            FillLifecycle<T>(vtable);
        }
        vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<T>;

        if constexpr (std::is_same_v<T, Value>) {
            key.kind = TypeKind::Value;
        }
        else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
            key.kind = GetArithmeticTypeKind<T>();
            FillArithmeticVTable(vtable, sizeof(T));
            vtable.pLess = [](const void* pLeft, const void* pRight) {
                return *static_cast<const T*>(pLeft) < *static_cast<const T*>(pRight);
            };
        }
        else if constexpr (std::is_same_v<T, SR_MATH_NS::FColor>) {
            key.kind = TypeKind::Color;
            static const FieldInfo fields[] = {
                { "r", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, r) },
                { "g", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, g) },
                { "b", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, b) },
                { "a", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, a) }
            };
            vtable.pFields = fields;
            vtable.fieldsCount = 4;
        }
        else if constexpr (std::is_same_v<T, SR_MATH_NS::Quaternion>) {
            key.kind = TypeKind::Quaternion;
            static const FieldInfo fields[] = {
                { "x", TypeOf<float_t>(), offsetof(T, x) },
                { "y", TypeOf<float_t>(), offsetof(T, y) },
                { "z", TypeOf<float_t>(), offsetof(T, z) },
                { "w", TypeOf<float_t>(), offsetof(T, w) }
            };
            vtable.pFields = fields;
            vtable.fieldsCount = 4;
        }
        else if constexpr (std::is_same_v<T, SR_MATH_NS::AABB>) {
            key.kind = TypeKind::AABB;
            static const FieldInfo fields[] = {
                { "min", TypeOf<SR_MATH_NS::FVector3>(), offsetof(T, min) },
                { "max", TypeOf<SR_MATH_NS::FVector3>(), offsetof(T, max) }
            };
            vtable.pFields = fields;
            vtable.fieldsCount = 2;
        }
        else if constexpr (std::is_same_v<T, SR_MATH_NS::Matrix3x3>) {
            key.kind = TypeKind::Matrix3x3;
            static const FieldInfo fields[] = {
                { "m00", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) },
                { "m01", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + sizeof(SR_MATH_NS::Unit) },
                { "m02", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + 2 * sizeof(SR_MATH_NS::Unit) },
                { "m10", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + 3 * sizeof(SR_MATH_NS::Unit) },
                { "m11", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + 4 * sizeof(SR_MATH_NS::Unit) },
                { "m12", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + 5 * sizeof(SR_MATH_NS::Unit) },
                { "m20", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + 6 * sizeof(SR_MATH_NS::Unit) },
                { "m21", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + 7 * sizeof(SR_MATH_NS::Unit) },
                { "m22", TypeOf<SR_MATH_NS::Unit>(), offsetof(T, m) + 8 * sizeof(SR_MATH_NS::Unit) }
            };
            vtable.pFields = fields;
            vtable.fieldsCount = 9;
        }
        else if constexpr (std::is_same_v<T, SR_MATH_NS::Matrix4x4>) {
            key.kind = TypeKind::Matrix4x4;
            static const FieldInfo fields[] = {
                { "m00", TypeOf<float_t>(), offsetof(T, m00) }, { "m01", TypeOf<float_t>(), offsetof(T, m01) },
                { "m02", TypeOf<float_t>(), offsetof(T, m02) }, { "m03", TypeOf<float_t>(), offsetof(T, m03) },
                { "m10", TypeOf<float_t>(), offsetof(T, m10) }, { "m11", TypeOf<float_t>(), offsetof(T, m11) },
                { "m12", TypeOf<float_t>(), offsetof(T, m12) }, { "m13", TypeOf<float_t>(), offsetof(T, m13) },
                { "m20", TypeOf<float_t>(), offsetof(T, m20) }, { "m21", TypeOf<float_t>(), offsetof(T, m21) },
                { "m22", TypeOf<float_t>(), offsetof(T, m22) }, { "m23", TypeOf<float_t>(), offsetof(T, m23) },
                { "m30", TypeOf<float_t>(), offsetof(T, m30) }, { "m31", TypeOf<float_t>(), offsetof(T, m31) },
                { "m32", TypeOf<float_t>(), offsetof(T, m32) }, { "m33", TypeOf<float_t>(), offsetof(T, m33) }
            };
            vtable.pFields = fields;
            vtable.fieldsCount = 16;
        }
        else if constexpr (IsTypeMathRectTemplateV<T>) {
            key.kind = TypeKind::MathRect;
            key.pNext[0] = TypeOf<typename T::ValueType>();
            static const FieldInfo fields[] = {
                { "x", key.pNext[0], offsetof(T, x) }, { "y", key.pNext[0], offsetof(T, y) },
                { "w", key.pNext[0], offsetof(T, w) }, { "h", key.pNext[0], offsetof(T, h) }
            };
            vtable.pFields = fields;
            vtable.fieldsCount = 4;
        }
        else if constexpr (IsTypeMathSizeTemplateV<T>) {
            key.kind = TypeKind::MathSize;
            key.pNext[0] = TypeOf<typename T::ValueType>();
            key.detailedSize = T::Dimensions();
            if constexpr (T::Dimensions() == 1) {
                static const FieldInfo fields[] = {
                    { "value", key.pNext[0], offsetof(T, value) },
                    { "metric", TypeOf<SR_MATH_NS::SizeMetric>(), offsetof(T, metric) }
                };
                vtable.pFields = fields;
            }
            else {
                static const FieldInfo fields[] = {
                    { "width", TypeOf<SR_MATH_NS::Size<typename T::ValueType>>(), offsetof(T, width) },
                    { "height", TypeOf<SR_MATH_NS::Size<typename T::ValueType>>(), offsetof(T, height) }
                };
                vtable.pFields = fields;
            }
            vtable.fieldsCount = 2;
        }
        else if constexpr (IsTypeMathVectorTemplateV<T>) {
            key.kind = TypeKind::MathVector;
            key.pNext[0] = TypeOf<typename T::ValueType>();
            key.detailedSize = T::Dimensions();
            static const Vector<FieldInfo> fields = [&key]() {
                Vector<FieldInfo> result = {
                    { "x", key.pNext[0], offsetof(T, x) },
                    { "y", key.pNext[0], offsetof(T, y) }
                };
                if constexpr (T::Dimensions() >= 3) {
                    result.emplace_back(FieldInfo{ "z", key.pNext[0], offsetof(T, z) });
                }
                if constexpr (T::Dimensions() >= 4) {
                    result.emplace_back(FieldInfo{ "w", key.pNext[0], offsetof(T, w) });
                }
                if constexpr (T::Dimensions() == 6) {
                    result.emplace_back(FieldInfo{ "v", key.pNext[0], offsetof(T, v) });
                    result.emplace_back(FieldInfo{ "u", key.pNext[0], offsetof(T, u) });
                }
                return result;
            }();
            vtable.pFields = fields.data();
            vtable.fieldsCount = static_cast<uint32_t>(fields.size());
        }
        else if constexpr (std::is_same_v<T, String> || std::is_same_v<T, StringView> || std::is_same_v<T, StringAtom> || std::is_same_v<T, Path> || std::is_same_v<T, UnicodeString>) {
            if constexpr (std::is_same_v<T, String>) {
                key.kind = TypeKind::String;
            }
            else if constexpr (std::is_same_v<T, StringView>) {
                key.kind = TypeKind::StringView;
            }
            else if constexpr (std::is_same_v<T, StringAtom>) {
                key.kind = TypeKind::StringAtom;
            }
            else if constexpr (std::is_same_v<T, Path>) {
                key.kind = TypeKind::Path;
            }
            else {
                key.kind = TypeKind::UnicodeString;
            }
        }
        else {
            static_assert(AlwaysFalseV<T>, "Type is not registered for reflection");
        }
    }

    template void DetermineTypeInfoRegistered<Value>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<int8_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<int16_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<int32_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<int64_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<uint8_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<uint16_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<uint32_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<uint64_t>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<float>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<double>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<bool>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<Path>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<String>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<StringView>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<StringAtom>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<UnicodeString>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FRect>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IRect>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::URect>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USRect>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USize>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::SVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(TypeInfoKey& key, TypeInfoVTable& vtable);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(TypeInfoKey& key, TypeInfoVTable& vtable);
}
