//
// Created by Monika on 01.09.2021.
//

#ifndef SR_ENGINE_VECTORUTILS_H
#define SR_ENGINE_VECTORUTILS_H

#include <Utils/Common/StringFormat.h>

namespace SR_UTILS_NS {
    class VectorUtils {
    public:
        VectorUtils() = delete;
        VectorUtils(VectorUtils &) = delete;
        ~VectorUtils() = delete;
        VectorUtils operator=(VectorUtils&) = delete;
    public:
        template<typename T> static Vector<T> Combine(const Vector<T>& A, const Vector<T>& B) {
            Vector<T> AB;
            AB.reserve(A.size() + B.size());
            AB.insert(AB.end(), A.begin(), A.end());
            AB.insert(AB.end(), B.begin(), B.end());
            return AB;
        }

        template<typename A, typename B> static Vector<B> Cast(const Vector<A>& source) {
            Vector<B> result;

            if constexpr (std::is_same_v<A, String>) {
                for (const auto& src : source)
                    result.emplace_back(LexicalCast<B>(src));
            } else
                static_assert("Unsupported type!");

            return result;
        }

        inline static String Merge(const Vector<String>& lines) {
            String merged;

            for (auto&& line : lines)
                merged.append(line).append("\n");

            return merged;
        }

        template<typename T> void InsertSorted(Vector<T>& v, const T& value) {
            auto it = std::lower_bound(v.begin(), v.end(), value);
            v.insert(it, value);
        }

        template<typename T> bool Contains(const Vector<T>& v, const T& value) {
            auto it = std::lower_bound(v.begin(), v.end(), value);
            return it != v.end() && *it == value;
        }
    };
}

#endif //SR_ENGINE_VECTORUTILS_H
