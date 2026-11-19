//
// Created by Monika on 21.07.2026.
//

namespace SR_UTILS_NS::Reflection {
    template<typename T> struct DetermineTypeInfoAccessor<Vector<T>> {
        static constexpr bool Supported = true;
        static void Determine(TypeInfoKey& key, TypeInfoVTable&) {
            Vector<T>::ValidateErasedLayout();
            key.kind = TypeKind::Vector;
            key.pNext[0] = TypeOf<T>();
        }
    };
}
