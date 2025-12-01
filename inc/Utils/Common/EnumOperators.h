//
// Created by Monika on 01.12.2025.
//

#ifndef SR_ENGINE_UTILS_ENUM_OPERATORS_H
#define SR_ENGINE_UTILS_ENUM_OPERATORS_H

#define SR_CODEGEN_ENUM_OPERATORS(modifier, assignModifier, enumName)                                                   \
	modifier constexpr enumName operator|(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) |                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator&(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) &                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator^(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) ^                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator~(enumName lhs) {                                                               \
	    return static_cast<enumName>(                                                                                   \
	        ~static_cast<std::underlying_type_t<enumName>>(lhs)                                                         \
	    );                                                                                                              \
	}                                                                                                                   \
	assignModifier constexpr enumName& operator|=(enumName& lhs, enumName rhs) {                                        \
	    lhs = lhs | rhs;                                                                                                \
	    return lhs;                                                                                                     \
	}                                                                                                                   \
	assignModifier constexpr enumName& operator&=(enumName& lhs, enumName rhs) {                                        \
	    lhs = lhs & rhs;                                                                                                \
	    return lhs;                                                                                                     \
	}                                                                                                                   \
	assignModifier constexpr enumName& operator^=(enumName& lhs, enumName rhs) {                                        \
	    lhs = lhs ^ rhs;                                                                                                \
	    return lhs;                                                                                                     \
	}                                                                                                                   \
	modifier constexpr bool operator!(enumName lhs) {                                                                   \
	    return !static_cast<std::underlying_type_t<enumName>>(lhs);                                                     \
	}                                                                                                                   \
	modifier constexpr bool operator&&(enumName lhs, enumName rhs) {                                                    \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) &&                                                    \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr bool operator||(enumName lhs, enumName rhs) {                                                    \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) ||                                                    \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr bool operator==(enumName lhs, enumName rhs) {                                                    \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) ==                                                    \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr bool operator!=(enumName lhs, enumName rhs) {                                                    \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) !=                                                    \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr bool operator<(enumName lhs, enumName rhs) {                                                     \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) <                                                     \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr bool operator>(enumName lhs, enumName rhs) {                                                     \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) >                                                     \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr bool operator<=(enumName lhs, enumName rhs) {                                                    \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) <=                                                    \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr bool operator>=(enumName lhs, enumName rhs) {                                                    \
	    return static_cast<std::underlying_type_t<enumName>>(lhs) >=                                                    \
	             static_cast<std::underlying_type_t<enumName>>(rhs);                                                    \
	}                                                                                                                   \
	modifier constexpr enumName operator+(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) +                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator-(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) -                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator*(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) *                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator/(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) /                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator%(enumName lhs, enumName rhs) {                                                 \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) %                                                        \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName& operator++(enumName& lhs) {                                                            \
	    lhs = static_cast<enumName>(static_cast<std::underlying_type_t<enumName>>(lhs) + 1);                            \
	    return lhs;                                                                                                     \
	}                                                                                                                   \
	modifier constexpr enumName operator++(enumName& lhs, int) {                                                        \
	    enumName result = lhs;                                                                                          \
	    ++lhs;                                                                                                          \
	    return result;                                                                                                  \
	}                                                                                                                   \
	modifier constexpr enumName& operator--(enumName& lhs) {                                                            \
	    lhs = static_cast<enumName>(static_cast<std::underlying_type_t<enumName>>(lhs) - 1);                            \
	    return lhs;                                                                                                     \
	}                                                                                                                   \
	modifier constexpr enumName operator--(enumName& lhs, int) {                                                        \
	    enumName result = lhs;                                                                                          \
	    --lhs;                                                                                                          \
	    return result;                                                                                                  \
	}                                                                                                                   \
	modifier constexpr enumName operator<<(enumName lhs, enumName rhs) {                                                \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) <<                                                       \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	modifier constexpr enumName operator>>(enumName lhs, enumName rhs) {                                                \
	    return static_cast<enumName>(                                                                                   \
	        static_cast<std::underlying_type_t<enumName>>(lhs) >>                                                       \
	        static_cast<std::underlying_type_t<enumName>>(rhs)                                                          \
	    );                                                                                                              \
	}                                                                                                                   \
	assignModifier constexpr enumName& operator<<=(enumName& lhs, enumName rhs) {                                       \
	    lhs = lhs << rhs;                                                                                               \
	    return lhs;                                                                                                     \
	}                                                                                                                   \
	assignModifier constexpr enumName& operator>>=(enumName& lhs, enumName rhs) {                                       \
	    lhs = lhs >> rhs;                                                                                               \
	    return lhs;                                                                                                     \
	}                                                                                                                   \

#endif //SR_ENGINE_UTILS_ENUM_OPERATORS_H
