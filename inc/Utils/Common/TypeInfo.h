//
// Created by Monika on 29.06.2022.
//

#ifndef SR_ENGINE_TYPEINFO_H
#define SR_ENGINE_TYPEINFO_H

#include <Utils/Common/Enumerations.h>
#include <Utils/Math/Size.h>
#include <Utils/Math/Vector4.h>
#include <Utils/Math/Vector6.h>
#include <Utils/Types/UnicodeString.h>

namespace SR_UTILS_NS {
    SR_ENUM_NS_CLASS_T(StandardType, uint16_t,
        Unknown,
        /// Boolean
        Bool,
        /// Numeric
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Double,
        /// Strings
        String,
        StringAtom,
        UnicodeString,
        Path,
        /// Containers
        Array, Map, Set,
        /// Reflected enums
        Enum,
        /// Mathematical size
        FSize, FSize2,
        ISize, ISize2,
        USize, USize2,
        /// Mathematical vectors
        FVector2, FVector3, FVector4, FVector5, FVector6,
        IVector2, IVector3, IVector4, IVector5, IVector6,
        UVector2, UVector3, UVector4, UVector5, UVector6,
        BVector2, BVector3, BVector4, BVector5, BVector6,
        /// Reflected objects
        Object, Class,
        /// Pointer to reflected object
        Ptr
    );

    SR_MAYBE_UNUSED uint64_t GetTypeSize(const StandardType& type);
    SR_MAYBE_UNUSED bool IsNumber(const StandardType& type);

    //specialize a type for all of the STL containers.
//    namespace is_stl_container_impl{
//        template <typename T>       struct is_stl_vector:std::false_type{};
//        template <typename... Args> struct is_stl_vector<std::vector<Args...>>:std::true_type{};
//
//        template <typename T>       struct is_stl_container:std::false_type{};
//        template <typename T, std::size_t N> struct is_stl_container<std::array    <T,N>>    :std::true_type{};
//        template <typename... Args> struct is_stl_container<std::vector            <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::deque             <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::list              <Args...>>:std::true_type{};
//#if !defined(SR_ANDROID) && defined(SR_CXX_20)
//        template <typename... Args> struct is_stl_container<std::forward_list      <Args...>>:std::true_type{};
//#endif
//        template <typename... Args> struct is_stl_container<std::set               <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::multiset          <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::map               <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::multimap          <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::unordered_set     <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::unordered_multiset<Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::unordered_map     <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::unordered_multimap<Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::stack             <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::queue             <Args...>>:std::true_type{};
//        template <typename... Args> struct is_stl_container<std::priority_queue    <Args...>>:std::true_type{};
//    }

    /// type trait to utilize the implementation type traits as well as decay the type
//    template <typename T> struct IsSTLContainer {
//        static constexpr bool const value = is_stl_container_impl::is_stl_container<std::decay_t<T>>::value;
//    };
//
//    template<typename T> constexpr bool IsSTLVector() {
//        return is_stl_container_impl::is_stl_vector<std::decay_t<T>>::value;
//    }

//    template<class T>
//    inline constexpr bool IsSTLVectorV = IsSTLVector<T>();
}

#endif //SR_ENGINE_TYPEINFO_H
