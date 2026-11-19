//
// Created by Monika on 21.07.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> SizeType ReflectedArraySize(const void* pArray) {
        return static_cast<const Vector<T>*>(pArray)->size();
    }

    template<typename T> void* ReflectedArrayAt(void* pArray, SizeType index) {
        return &(*static_cast<Vector<T>*>(pArray))[index];
    }

    template<typename T> void ReflectedArrayResize(void* pArray, SizeType size) {
        static_cast<Vector<T>*>(pArray)->resize(size);
    }

    template<typename T> void ReflectedArrayReserve(void* pArray, SizeType capacity) {
        static_cast<Vector<T>*>(pArray)->reserve(capacity);
    }

    template<typename T> void ReflectedArrayInsert(void* pArray, SizeType index, const void* pValue) {
        auto&& array = *static_cast<Vector<T>*>(pArray);
        array.insert(array.begin() + index, *static_cast<const T*>(pValue));
    }

    template<typename T> void ReflectedArrayErase(void* pArray, SizeType index) {
        auto&& array = *static_cast<Vector<T>*>(pArray);
        array.erase(array.begin() + index);
    }

    template<typename T> void ReflectedArrayClear(void* pArray) {
        static_cast<Vector<T>*>(pArray)->clear();
    }

    template<typename T> struct DetermineTypeInfoAccessor<Vector<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable& vtable) {
            key.kind = TypeKind::Vector;
            key.pNext[0] = TypeOf<T>();
            vtable.pSizeOfAlign = &ReflectedTypeTemplateSizeOfAlign<Vector<T>>;
            vtable.pConstructor = &ReflectedTypeTemplateConstructor<Vector<T>>;
            vtable.pDestructor = &ReflectedTypeTemplateDestructor<Vector<T>>;
            vtable.pCopy = &ReflectedTypeTemplateCopy<Vector<T>>;
            vtable.pMove = &ReflectedTypeTemplateMove<Vector<T>>;
            vtable.arrayVTable.pSize = &ReflectedArraySize<T>;
            vtable.arrayVTable.pAt = &ReflectedArrayAt<T>;
            vtable.arrayVTable.pResize = &ReflectedArrayResize<T>;
            vtable.arrayVTable.pReserve = &ReflectedArrayReserve<T>;
            vtable.arrayVTable.pInsert = &ReflectedArrayInsert<T>;
            vtable.arrayVTable.pErase = &ReflectedArrayErase<T>;
            vtable.arrayVTable.pClear = &ReflectedArrayClear<T>;
        }
    };
}
