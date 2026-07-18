//
// Created by Monika on 17.07.2026.
//

#ifndef SR_ENGINE_COMMON_MEMORY_LITERALS_H
#define SR_ENGINE_COMMON_MEMORY_LITERALS_H

#include <Utils/stdInclude.h>

constexpr std::uint64_t operator""_B(unsigned long long value) {
    return value;
}

// Binary (1024)
constexpr std::uint64_t operator""_KiB(unsigned long long value) {
    return value * 1024ull;
}

constexpr std::uint64_t operator""_MiB(unsigned long long value) {
    return value * 1024ull * 1024ull;
}

constexpr std::uint64_t operator""_GiB(unsigned long long value) {
    return value * 1024ull * 1024ull * 1024ull;
}

constexpr std::uint64_t operator""_TiB(unsigned long long value) {
    return value * 1024ull * 1024ull * 1024ull * 1024ull;
}

// Decimal (1000)
constexpr std::uint64_t operator""_KB(unsigned long long value) {
    return value * 1000ull;
}

constexpr std::uint64_t operator""_MB(unsigned long long value) {
    return value * 1000ull * 1000ull;
}

constexpr std::uint64_t operator""_GB(unsigned long long value) {
    return value * 1000ull * 1000ull * 1000ull;
}

constexpr std::uint64_t operator""_TB(unsigned long long value) {
    return value * 1000ull * 1000ull * 1000ull * 1000ull;
}

#endif //SR_ENGINE_COMMON_MEMORY_LITERALS_H
