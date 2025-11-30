//
// Created by Monika on 30.11.2025.
//

#ifndef SR_ENGINE_UTILS_PCH_H
#define SR_ENGINE_UTILS_PCH_H

#include <Utils/macros.h>

#include <string>
#include <array>
#include <map>
#include <vector>
#include <cstdint>
#include <set>
#include <optional>

#ifndef SR_ENGINE_CODEGEN_CLANG_PARSE_MODE
    #include <cfloat>
    #include <span>
    #include <limits>
    #include <bitset>
    #include <type_traits>
    #include <cstdio>
    #include <iosfwd>
    #include <regex>
    #include <stdexcept>
    #include <string_view>
    #include <shared_mutex>
    #include <cstdarg>
    #include <initializer_list>
    #include <codecvt>
    #include <cstddef>
    #include <unordered_set>
    #include <stack>
    #include <cctype>
    #include <charconv>
    #include <locale>
    #include <cstring>
    #include <variant>
    #include <memory>
    #include <fstream>
    #include <ostream>
    #include <queue>
    #include <mutex>
    #include <cassert>
    #include <cmath>
    #include <ranges>
    #include <atomic>
    #include <utility>
    #include <functional>
    #include <exception>
    #include <unordered_map>
    #include <algorithm>
    #include <any>
    #include <thread>
    #include <sstream>
    #include <list>
    #include <ctime>
    #include <iostream>
    #include <ratio>
    #include <chrono>
    #include <random>
    #include <iomanip>
    #include <concepts>
    #include <condition_variable>
    #include <numeric>
    #include <numbers>
    #include <execution>

    #define XXH_STATIC_LINKING_ONLY
    #include <xxHash/xxhash.h>

    #ifndef SR_ENGINE_SCRIPT_API_MODE
        #include <zlib.h>
    #endif

    #ifdef SR_SUPPORT_PARALLEL
        #include <omp.h>
    #endif

    #if !defined(SR_ANDROID) && defined(SR_CXX_20)
        #include <forward_list>
    #endif

    #ifdef SR_MINGW
        #include <iomanip>
    #endif

    #if defined(SR_LINUX) || defined(SR_ANDROID)
        #include <sys/stat.h>
    #endif

    #ifdef SR_LINUX
        #include <cstdarg>
        #include <signal.h>
    #endif

    #if defined(SR_WIN32)
        #include <direct.h>
    #endif

    #include <fmt/format.h>
    #include <fmt/printf.h>
    #include <fmt/color.h>
#endif

#undef min
#undef max

#endif //SR_ENGINE_UTILS_PCH_H
