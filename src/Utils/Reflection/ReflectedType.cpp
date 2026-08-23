//
// Created by Monika on 21.07.2026.
//

#include <Utils/Reflection/ReflectedType.h>
#include <Utils/Types/Vector.h>
#include <Utils/Types/Optional.h>
#include <Utils/Types/UnicodeString.h>
#include <Utils/FileSystem/Path.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/Platform/Platform.h>

namespace SR_UTILS_NS::Reflection {
    Value ReflectedContainerIterator::operator*() const {
        auto pNonConstContainer = const_cast<Value*>(pContainer);
        ReflectedValue reflectedValue = pContainer->GetTypeInfo().vtable.iteratorVTable.pGetValue(pNonConstContainer->GetStorage(), *this);
        Value value;
        value.m_storage = reflectedValue;
        value.m_typeInfo = CopyTypeInfo(pContainer->GetTypeInfo().pNext[0]);
        return value;
    }

    Value ReflectedContainerIterator::operator->() const {
        return **this;
    }

    ReflectedContainerIterator ReflectedContainerIterator::operator+(int64_t offset) const {
        ReflectedContainerIterator pIt = pContainer->GetTypeInfo().vtable.iteratorVTable.pOffset(*this, offset);
        pIt.pContainer = pContainer;
        return pIt;
    }

    ReflectedContainerIterator& ReflectedContainerIterator::operator++() {
        *this = *this + 1;
        return *this;
    }

    ReflectedContainerIterator ReflectedContainerIterator::operator-(int64_t offset) const {
        ReflectedContainerIterator pIt = pContainer->GetTypeInfo().vtable.iteratorVTable.pOffset(*this, -offset);
        pIt.pContainer = pContainer;
        return pIt;
    }

    ReflectedContainerIterator& ReflectedContainerIterator::operator--() {
        *this = *this - 1;
        return *this;
    }

    bool ReflectedContainerIterator::operator==(const ReflectedContainerIterator& other) const noexcept {
        return memcmp(data, other.data, sizeof(data)) == 0;
    }

    bool ReflectedContainerIterator::operator!=(const ReflectedContainerIterator& other) const noexcept {
        return !(*this == other);
    }

    Value ReflectedContainerIterator::First() const {
        if (!IsMapIterator()) {
            return **this;
        }
        auto&& iteratorValue = (**this);
        auto&& pairVTable = pContainer->GetTypeInfo().pNext[0]->vtable.pairVTable;
        ReflectedValue reflectedValue = pairVTable.pGetPairValue(iteratorValue.GetStorage(), true);
        Value value;
        value.m_storage = reflectedValue;
        value.m_typeInfo = CopyTypeInfo(pContainer->GetTypeInfo().pNext[0]->pNext[0]);
        return value;
    }

    Value ReflectedContainerIterator::Second() const {
        if (!IsMapIterator()) {
            SRHalt("ReflectedContainerIterator::Second() : not a map iterator!");
            return Value();
        }
        auto&& iteratorValue = (**this);
        auto&& pairVTable = pContainer->GetTypeInfo().pNext[0]->vtable.pairVTable;
        ReflectedValue reflectedValue = pairVTable.pGetPairValue(iteratorValue.GetStorage(), false);
        Value value;
        value.m_storage = reflectedValue;
        value.m_typeInfo = CopyTypeInfo(pContainer->GetTypeInfo().pNext[0]->pNext[1]);
        return value;
    }

    bool ReflectedContainerIterator::IsMapIterator() const {
        return pContainer && pContainer->GetTypeInfo().category == ReflectedCategoryType::Container &&
               (pContainer->GetTypeInfo().detailedType == "Map" || pContainer->GetTypeInfo().detailedType == "FlatHashMap");
    }

    bool IsReflectedTypeSigned(StringView type) {
        return type == "int8" || type == "int16" || type == "int32" || type == "int64";
    }

    bool IsReflectedTypeIntegral(StringView type) {
        return type == "int8" || type == "int16" || type == "int32" || type == "int64" ||
               type == "uint8" || type == "uint16" || type == "uint32" || type == "uint64";
    }

    SizeType GetReflectedTypeSize(StringView type) {
        if (type == "int8" || type == "uint8") {
            return sizeof(uint8_t);
        }
        else if (type == "int16" || type == "uint16") {
            return sizeof(uint16_t);
        }
        else if (type == "int32" || type == "uint32") {
            return sizeof(uint32_t);
        }
        else if (type == "int64" || type == "uint64") {
            return sizeof(uint64_t);
        }
        else if (type == "float") {
            return sizeof(float);
        }
        else if (type == "double") {
            return sizeof(double);
        }
        else if (type == "bool") {
            return sizeof(bool);
        }
        else if (type == "Path") {
            return sizeof(Path);
        }
        else if (type == "String") {
            return sizeof(String);
        }
        else if (type == "StringView") {
            return sizeof(StringView);
        }
        else if (type == "StringAtom") {
            return sizeof(StringAtom);
        }
        else if (type == "UnicodeString") {
            return sizeof(UnicodeString);
        }

        SRHalt("GetReflectedTypeSize() : unknown reflected type: {}", type);
        return 0;
    }

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

    bool TypeInfo::operator==(const TypeInfo& other) const noexcept {
        if (category != other.category || detailedType != other.detailedType || detailedSize != other.detailedSize) {
            return false;
        }
        for (size_t i = 0; i < 2; ++i) {
            if (pNext[i] != other.pNext[i]) {
                return false;
            }
        }
        return true;
    }

    bool TypeInfo::operator!=(const TypeInfo& other) const noexcept {
        return !(*this == other);
    }

    SRHashType TypeInfo::GetHash() const noexcept {
        SRHashType hash = static_cast<SRHashType>(category);
        hash = HashCombine(hash, detailedSize);
        hash = HashCombine(hash, detailedType);
        for (size_t i = 0; i < 2; ++i) {
            if (pNext[i]) {
                hash = HashCombine(hash, pNext[i]->GetHash());
            }
        }
        return hash;
    }

    template<typename T, typename Enable = void> struct DetermineTypeName {
        static StringAtom Get() { return StringAtom(); }
    };

    template<> struct DetermineTypeName<int8_t> { static StringAtom Get() { static StringAtom name = "int8"; return name; } };
    template<> struct DetermineTypeName<int16_t> { static StringAtom Get() { static StringAtom name = "int16"; return name; } };
    template<> struct DetermineTypeName<int32_t> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<int64_t> { static StringAtom Get() { static StringAtom name = "int64"; return name; } };
    template<> struct DetermineTypeName<uint8_t> { static StringAtom Get() { static StringAtom name = "uint8"; return name; } };
    template<> struct DetermineTypeName<uint16_t> { static StringAtom Get() { static StringAtom name = "uint16"; return name; } };
    template<> struct DetermineTypeName<uint32_t> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<uint64_t> { static StringAtom Get() { static StringAtom name = "uint64"; return name; } };
    template<> struct DetermineTypeName<float> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<double> { static StringAtom Get() { static StringAtom name = "double"; return name; } };
    template<> struct DetermineTypeName<bool> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<Path> { static StringAtom Get() { static StringAtom name = "Path"; return name; } };
    template<> struct DetermineTypeName<String> { static StringAtom Get() { static StringAtom name = "String"; return name; } };
    template<> struct DetermineTypeName<StringView> { static StringAtom Get() { static StringAtom name = "StringView"; return name; } };
    template<> struct DetermineTypeName<StringAtom> { static StringAtom Get() { static StringAtom name = "StringAtom"; return name; } };
    template<> struct DetermineTypeName<UnicodeString> { static StringAtom Get() { static StringAtom name = "UnicodeString"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FColor> { static StringAtom Get() { static StringAtom name = "Color"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::Quaternion> { static StringAtom Get() { static StringAtom name = "Quaternion"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::AABB> { static StringAtom Get() { static StringAtom name = "AABB"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::Matrix3x3> { static StringAtom Get() { static StringAtom name = "Matrix3x3"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::Matrix4x4> { static StringAtom Get() { static StringAtom name = "Matrix4x4"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FRect> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IRect> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::URect> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::USRect> { static StringAtom Get() { static StringAtom name = "uint16"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FSize> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::USize> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::ISize> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FSize2> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::USize2> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::ISize2> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector2> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector3> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector4> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::FVector6> { static StringAtom Get() { static StringAtom name = "float"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector2> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector3> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector4> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::IVector6> { static StringAtom Get() { static StringAtom name = "int32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector2> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector3> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector4> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::UVector6> { static StringAtom Get() { static StringAtom name = "uint32"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector2> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector3> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector4> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };
    template<> struct DetermineTypeName<SR_MATH_NS::BVector6> { static StringAtom Get() { static StringAtom name = "bool"; return name; } };

    struct TypeInfoPool {
        SR_UTILS_NS::MonotonicAllocator allocator;
        Vector<TypeInfo*> available;
        uint64_t allocatedCount = 0;
        std::mutex mutex;
        TypeInfoPool()
            : allocator(sizeof(TypeInfo) * 1024)
            , available()
        { }
        ~TypeInfoPool() = default;
    };

    TypeInfoPool* g_typeInfoPool = nullptr;

    TypeInfo* AllocateTypeInfo() {
        std::lock_guard<std::mutex> lock(g_typeInfoPool->mutex);

        if (g_typeInfoPool->available.empty()) {
            g_typeInfoPool->allocatedCount++;
            auto pTypeInfo = g_typeInfoPool->allocator.Allocate(sizeof(TypeInfo), alignof(TypeInfo));
            new (pTypeInfo) TypeInfo();
            return reinterpret_cast<TypeInfo*>(pTypeInfo);
        }
        else {
            auto pTypeInfo = g_typeInfoPool->available.back();
            memset(pTypeInfo, 0, sizeof(TypeInfo));
            g_typeInfoPool->available.pop_back();
            return pTypeInfo;
        }
    }

    void FreeTypeInfo(TypeInfo* pTypeInfo) {
        if (!pTypeInfo) {
            return;
        }
        if (!g_typeInfoPool) SR_UNLIKELY_ATTRIBUTE {
            SR_PLATFORM_NS::WriteConsoleError("FreeTypeInfo() : TypeInfoPool is not initialized!");
            SR_PLATFORM_NS::Terminate(true);
        }
        FreeTypeInfo(pTypeInfo->pNext[0]);
        FreeTypeInfo(pTypeInfo->pNext[1]);
        std::lock_guard<std::mutex> lock(g_typeInfoPool->mutex);
        g_typeInfoPool->available.push_back(pTypeInfo);
    }

    TypeInfo* CopyTypeInfo(TypeInfo* pTypeInfo) {
        if (!pTypeInfo) {
            return nullptr;
        }
        auto pNewTypeInfo = AllocateTypeInfo();
        *pNewTypeInfo = *pTypeInfo;
        pNewTypeInfo->pNext[0] = CopyTypeInfo(pTypeInfo->pNext[0]);
        pNewTypeInfo->pNext[1] = CopyTypeInfo(pTypeInfo->pNext[1]);
        return pNewTypeInfo;
    }

    using SwithTypeRegisterFunc = void(*)(bool);
    SwithTypeRegisterFunc g_switchTypeRegisterFuncs[] = {
        &SwitchTypeRegister<Path>,
        &SwitchTypeRegister<String>,
        &SwitchTypeRegister<StringView>,
        &SwitchTypeRegister<StringAtom>,
        &SwitchTypeRegister<UnicodeString>,
        &SwitchTypeRegister<SR_MATH_NS::FColor>,
        &SwitchTypeRegister<SR_MATH_NS::Quaternion>,
        &SwitchTypeRegister<SR_MATH_NS::AABB>,
        &SwitchTypeRegister<SR_MATH_NS::Matrix3x3>,
        &SwitchTypeRegister<SR_MATH_NS::Matrix4x4>,
        &SwitchTypeRegister<SR_MATH_NS::FRect>,
        &SwitchTypeRegister<SR_MATH_NS::IRect>,
        &SwitchTypeRegister<SR_MATH_NS::URect>,
        &SwitchTypeRegister<SR_MATH_NS::USRect>,
        &SwitchTypeRegister<SR_MATH_NS::FSize>,
        &SwitchTypeRegister<SR_MATH_NS::USize>,
        &SwitchTypeRegister<SR_MATH_NS::ISize>,
        &SwitchTypeRegister<SR_MATH_NS::FSize2>,
        &SwitchTypeRegister<SR_MATH_NS::USize2>,
        &SwitchTypeRegister<SR_MATH_NS::ISize2>,
        &SwitchTypeRegister<SR_MATH_NS::FVector2>,
        &SwitchTypeRegister<SR_MATH_NS::FVector3>,
        &SwitchTypeRegister<SR_MATH_NS::FVector4>,
        &SwitchTypeRegister<SR_MATH_NS::FVector6>,
        &SwitchTypeRegister<SR_MATH_NS::IVector2>,
        &SwitchTypeRegister<SR_MATH_NS::IVector3>,
        &SwitchTypeRegister<SR_MATH_NS::IVector4>,
        &SwitchTypeRegister<SR_MATH_NS::IVector6>,
        &SwitchTypeRegister<SR_MATH_NS::UVector2>,
        &SwitchTypeRegister<SR_MATH_NS::UVector3>,
        &SwitchTypeRegister<SR_MATH_NS::UVector4>,
        &SwitchTypeRegister<SR_MATH_NS::UVector6>,
        &SwitchTypeRegister<SR_MATH_NS::BVector2>,
        &SwitchTypeRegister<SR_MATH_NS::BVector3>,
        &SwitchTypeRegister<SR_MATH_NS::BVector4>,
        &SwitchTypeRegister<SR_MATH_NS::BVector6>
    };

    void InitReflection() {
        SR_TRACY_ZONE;

        if (!g_typeInfoPool) SR_UNLIKELY_ATTRIBUTE {
            g_typeInfoPool = new TypeInfoPool();
        }

        for (auto&& func : g_switchTypeRegisterFuncs) {
            func(true);
        }
    }

    void DeInitReflection() {
        SR_TRACY_ZONE;

        if (!g_typeInfoPool) {
            return;
        }

        for (auto&& func : g_switchTypeRegisterFuncs) {
            func(false);
        }

        if (auto&& size = GetVTableSize(); size > 0) {
            SR_PLATFORM_NS::WriteConsoleError("DeInitReflection() : TypeInfoVTable is not empty! Count: {}"_format(size));
        }

        delete g_typeInfoPool;
        g_typeInfoPool = nullptr;
    }

    template<typename T> void ReflectedTypeInlineCopy(const ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = reinterpret_cast<const T*>(from.GetData());
        auto pTo = reinterpret_cast<T*>(to.GetData());
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeInlineMove(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = reinterpret_cast<T*>(from.GetData());
        auto pTo = reinterpret_cast<T*>(to.GetData());
        *pTo = std::move(*pFrom);
    }

    template<typename T> ReflectedValue ReflectedTypeConstructor(IAllocator& allocator) {
        if constexpr (sizeof(T) <= ReflectedValueStorageSize) {
            ReflectedValue reflectedValue;
            new(&reflectedValue.storage.data) T();
            return ReflectedValue::MakeFromInlineData(reflectedValue.storage, ReflectedValueStorageType::Embedded);
        }
        auto pValue = static_cast<T*>(allocator.Allocate(sizeof(T), alignof(T)));
        new (pValue) T();
        return ReflectedValue::MakeFromPointer(pValue, ReflectedValueStorageType::Dynamic);
    }

    template<typename T> void ReflectedTypeDestructor(IAllocator& allocator, ReflectedValue& value) {
        auto pValue = static_cast<T*>(value.GetData());
        pValue->~T();
        if constexpr (sizeof(T) > ReflectedValueStorageSize) {
            allocator.Free(pValue, sizeof(T), alignof(T));
        }
    }

    template<typename T> void ReflectedTypeCopy(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<T*>(from.GetData());
        auto pTo = static_cast<T*>(to.GetData());
        *pTo = *pFrom;
    }

    template<typename T> void ReflectedTypeMove(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<T*>(from.GetData());
        auto pTo = static_cast<T*>(to.GetData());
        *pTo = std::move(*pFrom);
    }

    template<typename T> SizeType ReflectedTypeSize(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        return pContainer->size();
    }

    template<typename T> void ReflectedTypeResize(ReflectedValue& value, SizeType size, bool reserve) {
        auto pContainer = static_cast<T*>(value.GetData());
        if (reserve)  {
            pContainer->reserve(size);
        }
        else {
            pContainer->resize(size);
        }
    }

    template<typename T> void ReflectedTypeClear(ReflectedValue& value) {
        auto pContainer = static_cast<T*>(value.GetData());
        pContainer->clear();
    }

    ReflectedValue ArithmeticTypeConstructor(IAllocator&) {
        return ReflectedValue::MakeFromNumeric(uint64_t(), ReflectedValueStorageType::Embedded);
    }

    void ArithmeticTypeCopy(const ReflectedValue& from, ReflectedValue& to) {
        void* pFrom = const_cast<void*>(from.GetData());
        void* pTo = to.GetData();
        memcpy(pTo, pFrom, sizeof(uint64_t));
    }

    void ArithmeticTypeMove(ReflectedValue& from, ReflectedValue& to) {
        void* pFrom = from.GetData();
        void* pTo = to.GetData();
        memcpy(pTo, pFrom, sizeof(uint64_t));
        memset(pFrom, 0, sizeof(uint64_t));
    }

    void ValueCopy(const ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<const Value*>(from.GetData());
        auto pTo = static_cast<Value*>(to.GetData());
        *pTo = *pFrom;
    }

    void ValueMove(ReflectedValue& from, ReflectedValue& to) {
        auto pFrom = static_cast<Value*>(from.GetData());
        auto pTo = static_cast<Value*>(to.GetData());
        *pTo = std::move(*pFrom);
    }

    template<typename T> void DetermineTypeInfoRegistered(TypeInfo* pTypeInfo) {
        pTypeInfo->vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<T>;

        if constexpr (std::is_same_v<T, Value>) {
            pTypeInfo->category = ReflectedCategoryType::Value;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeDestructor<T>;
            pTypeInfo->vtable.pCopy = &ValueCopy;
            pTypeInfo->vtable.pMove = &ValueMove;
        }
        else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, bool>) {
            pTypeInfo->category = ReflectedCategoryType::Arithmetic;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ArithmeticTypeConstructor;
            pTypeInfo->vtable.pCopy = &ArithmeticTypeCopy;
            pTypeInfo->vtable.pMove = &ArithmeticTypeMove;
        }
        else if constexpr (std::is_same_v<T, SR_MATH_NS::Quaternion> || std::is_same_v<T, SR_MATH_NS::Matrix3x3> ||
            std::is_same_v<T, SR_MATH_NS::Matrix4x4> || std::is_same_v<T, SR_MATH_NS::FColor> || std::is_same_v<T, SR_MATH_NS::AABB>
        ) {
            pTypeInfo->category = ReflectedCategoryType::MathObject;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (IsTypeMathRectTemplateV<T>) {
            pTypeInfo->category = ReflectedCategoryType::MathRect;
            pTypeInfo->detailedType = DetermineTypeName<typename T::ValueType>::Get();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (IsTypeMathSizeTemplateV<T>) {
            pTypeInfo->category = ReflectedCategoryType::MathSize;
            pTypeInfo->detailedType = DetermineTypeName<typename T::ValueType>::Get();
            pTypeInfo->detailedSize = T::Dimensions();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (IsTypeMathVectorTemplateV<T>) {
            pTypeInfo->category = ReflectedCategoryType::MathVector;
            pTypeInfo->detailedType = DetermineTypeName<typename T::ValueType>::Get();
            pTypeInfo->detailedSize = T::Dimensions();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
        }
        else if constexpr (std::is_same_v<T, String> || std::is_same_v<T, StringView> || std::is_same_v<T, StringAtom> || std::is_same_v<T, Path> || std::is_same_v<T, UnicodeString>) {
            pTypeInfo->category = ReflectedCategoryType::String;
            pTypeInfo->detailedType = DetermineTypeName<T>::Get();
            pTypeInfo->vtable.pConstructor = &ReflectedTypeConstructor<T>;
            pTypeInfo->vtable.pDestructor = &ReflectedTypeDestructor<T>;
            pTypeInfo->vtable.pCopy = &ReflectedTypeInlineCopy<T>;
            pTypeInfo->vtable.pMove = &ReflectedTypeInlineMove<T>;
            pTypeInfo->vtable.containerVTable.pSize = &ReflectedTypeSize<T>;
            if constexpr (std::is_same_v<T, String> || std::is_same_v<T, UnicodeString>) {
                pTypeInfo->vtable.containerVTable.pResize = &ReflectedTypeResize<T>;
            }
            pTypeInfo->vtable.containerVTable.pClear = &ReflectedTypeClear<T>;
        }
        else {
            static_assert(AlwaysFalseV<T>, "Type is not registered for reflection");
        }
    }

    template void DetermineTypeInfoRegistered<Value>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<int8_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<int16_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<int32_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<int64_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint8_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint16_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint32_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<uint64_t>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<float>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<double>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<bool>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<Path>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<String>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<StringView>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<StringAtom>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<UnicodeString>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FColor>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Quaternion>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::AABB>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix3x3>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::Matrix4x4>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FRect>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IRect>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::URect>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USRect>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FSize>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USize>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::ISize>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FSize2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USize2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::ISize2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector3>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector4>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::FVector6>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::SVector2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector3>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector4>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::IVector6>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::USVector2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector3>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector4>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::UVector6>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector2>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector3>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector4>(TypeInfo* pTypeInfo);
    template void DetermineTypeInfoRegistered<SR_MATH_NS::BVector6>(TypeInfo* pTypeInfo);

    bool Test() {
        Vector<uint32_t> values = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
        Optional<int> optionalValue = 42;
        auto valuesRef = Reflection::ReflectedValue::MakeFromPointer(
            &values,
            Reflection::ReflectedValueStorageType::Reference
        );

        auto valueRef = Reflection::Value::CreateRef(values);
        Vector<uint32_t>* pValues2 = valueRef.Cast<Vector<uint32_t>>();

        Set<String> stringSet = { "Hello", "World", "!" };
        Map<String, int> stringMap = { {"One", 1}, {"Two", 2}, {"Three", 3} };
        auto mapRef = Reflection::ReflectedValue::MakeFromPointer(
            &stringMap,
            Reflection::ReflectedValueStorageType::Reference
        );

        String string = "Two";

        IAllocator* pAllocator = IAllocator::GetDefaultAllocator();
        TypeInfo* types[19] {
            DetermineTypeInfoAlloc(values),
            DetermineTypeInfoAlloc(optionalValue),
            DetermineTypeInfoAlloc(2),
            DetermineTypeInfoAlloc(true),
            DetermineTypeInfoAlloc(String()),
            DetermineTypeInfoAlloc(StringView()),
            DetermineTypeInfoAlloc(StringAtom()),
            DetermineTypeInfoAlloc(UnicodeString()),
            DetermineTypeInfoAlloc(Path()),
            DetermineTypeInfoAlloc(SR_MATH_NS::FSize2()),
            DetermineTypeInfoAlloc(SR_MATH_NS::FRect()),
            DetermineTypeInfoAlloc(SR_MATH_NS::Quaternion()),
            DetermineTypeInfoAlloc(SR_MATH_NS::Matrix4x4()),
            DetermineTypeInfoAlloc(SR_MATH_NS::UVector6()),
            DetermineTypeInfoAlloc(ReflectedCategoryType()),
            DetermineTypeInfoAlloc(Serializable()),
            DetermineTypeInfoAlloc(SRClass()),
            DetermineTypeInfoAlloc(stringSet),
            DetermineTypeInfoAlloc(stringMap)
        };

        Vector<uint32_t>* pValues = reinterpret_cast<Vector<uint32_t>*>(valuesRef.GetData());
        types[0]->vtable.containerVTable.pResize(valuesRef, 4, false);
        ReflectedContainerIterator mapIt = types[18]->vtable.containerVTable.pFind(mapRef, ReflectedValue::MakeFromPointer(&string, ReflectedValueStorageType::Reference));
        ReflectedValue mapValue = types[18]->vtable.iteratorVTable.pGetValue(mapRef, mapIt);
        auto value = *static_cast<Pair<String, int>*>(mapValue.GetData());

        Value vectorVal = Value::Create(Vector<int32_t>{ 1, 2, 3, 4, 5 });
        Value vectorValCopy = vectorVal;
        auto* vec = vectorVal.Cast<Vector<int32_t>>();
        vectorVal = {};
        auto* vecCopy = vectorValCopy.Cast<Vector<int32_t>>();

        Value bigTypeVal = Value::Create(SR_MATH_NS::Matrix4x4());
        auto* bigType = bigTypeVal.Cast<SR_MATH_NS::Matrix4x4>();
        bigTypeVal = {};

        return true;
    }
    //bool b = Test();
}
