cmake_minimum_required(VERSION 3.16)
project(Utils)

set(CMAKE_CXX_STANDARD 20)

set(SR_COMMON_USE_CLANG_EMULATION OFF CACHE INTERNAL "" FORCE)

if (MSVC)
    if (CMAKE_CXX_COMPILER MATCHES "clang-cl")
        set(SR_COMMON_USE_CLANG_EMULATION ON CACHE INTERNAL "" FORCE)

        add_definitions(-DSR_COMMON_USE_CLANG_EMULATION=1)
        add_compile_definitions(SR_COMMON_USE_CLANG_EMULATION)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSR_COMMON_USE_CLANG_EMULATION")

        message(STATUS "Using Clang compiler emulation for MSVC")
    endif()
endif()

if (NOT SR_COMMON_USE_CLANG_EMULATION)
    option(SR_COMMON_ASSIMP "" ON)
else()
    option(SR_COMMON_ASSIMP "" OFF)
    message(STATUS "Assimp is disabled because Clang emulation is used")
endif()

option(SR_COMMON_ZLIB "" ON)
option(SR_COMMON_OPENSSL "" OFF)
option(SR_COMMON_SDL "" OFF)
option(SR_COMMON_CURL "" OFF)
option(SR_COMMON_JSON "" OFF)
option(SR_COMMON_MYHTML "" OFF)
option(SR_COMMON_LITEHTML "" OFF)
option(SR_COMMON_ASIO "" OFF)

option(SR_COMMON_DLL_EXPORTS "" ON)
option(SR_COMMON_GIT_METADATA "" ON)
option(SR_COMMON_STATIC_LIBRARY "" ON)
option(SR_COMMON_EMBED_RESOURCES "" OFF)
option(SR_COMMON_CI_BUILD "" OFF)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(SKIP_INSTALL_ALL ON)

if (SR_EMSCRIPTEN)
    set(ZLIB_ONLY_STATIC ON)
endif()

include(cmake/Linker.cmake)
include(cmake/CopyDLLToExeFolder.cmake)

macro(SR_UTILS_INCLUDE_DIRECTORIES_APPEND var)
    set_property(GLOBAL APPEND PROPERTY SR_UTILS_INCLUDE_DIRECTORIES "${var}")
endmacro(SR_UTILS_INCLUDE_DIRECTORIES_APPEND)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX")
endif()

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT MSVC)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++20")
endif()

if (SR_COMMON_ZLIB)
    set(ZLIB_BUILD_EXAMPLES OFF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASSIMP_BUILD_ZLIB=OFF")
    set(ASSIMP_BUILD_ZLIB OFF CACHE INTERNAL "" FORCE)
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/zlib ${CMAKE_CURRENT_BINARY_DIR}/libs/zlib)
    set(SR_ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
    add_subdirectory(libs/zlib)
endif()

if (SR_COMMON_ASSIMP AND NOT SR_COMMON_USE_CLANG_EMULATION)
    message("SRCommon will be using Assimp")

    add_definitions(-DSR_UTILS_ASSIMP=1)
    add_compile_definitions(SR_UTILS_ASSIMP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSR_UTILS_ASSIMP")

    include(cmake/AssimpConfigure.cmake)
    add_subdirectory(libs/assimp)

    target_compile_definitions(assimp PUBLIC ${SR_ASSIMP_DISABLED_IMPORTERS})
endif()

if (NOT ANDROID_NDK AND NOT SR_EMSCRIPTEN)
    if (UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
    endif()
    set(SR_EFSW_USE ON CACHE INTERNAL "" FORCE)
else()
    set(SR_EFSW_USE OFF CACHE INTERNAL "" FORCE)
endif()

if (SR_COMMON_SDL)
    add_subdirectory(libs/SDL)
endif()

if (SR_COMMON_JSON)
    set(JSON_BuildTests OFF CACHE INTERNAL "" FORCE)

    add_subdirectory(libs/json)
endif()

if (SR_COMMON_GIT_METADATA)
    add_subdirectory(libs/cmake-git-version-tracking)
endif()

set(SR_COMMON_CPPTRACE_ENABLED OFF)
if (NOT ANDROID_NDK AND UNIX AND NOT SR_EMSCRIPTEN)
    set(SR_COMMON_CPPTRACE_ENABLED ON)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(WARNING "Clang compiler is used. Stack tracing will not work.")
        set(CPPTRACE_UNWIND_WITH_NOTHING ON)
    endif()

    add_subdirectory(libs/cpptrace)
endif()

if (SR_COMMON_CURL)
    set(BUILD_LIBCURL_DOCS OFF CACHE INTERNAL "" FORCE)
    set(ENABLE_CURL_MANUAL OFF CACHE INTERNAL "" FORCE)
    set(BUILD_MISC_DOCS OFF CACHE INTERNAL "" FORCE)
    set(USE_MANUAL OFF CACHE INTERNAL "" FORCE)
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "" FORCE)
    set(BUILD_TESTING OFF CACHE INTERNAL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
    set(USE_OPENSSL ${SR_COMMON_OPENSSL} CACHE INTERNAL "" FORCE)
    set(CURL_USE_OPENSSL ${SR_COMMON_OPENSSL} CACHE INTERNAL "" FORCE)
    set(BUILD_STATIC_CURL ON CACHE INTERNAL "" FORCE)

    if(WIN32)
        add_definitions("-DCURL_STATICLIB")
    endif()

    add_subdirectory(libs/curl)
endif()

set(ICU_PREBUILT_VERSION "69.1")

if (NOT ANDROID_NDK AND NOT SR_EMSCRIPTEN)
    add_subdirectory(libs/nativefiledialog)
    add_subdirectory(libs/tinyfiledialogs)
else()
    #set(ASSIMP_ANDROID_JNIIOSYSTEM ON CACHE INTERNAL "" FORCE)
    #set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE INTERNAL "" FORCE)
    #set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE INTERNAL "" FORCE)
endif()

add_subdirectory(libs/glm)

if (SR_UTILS_FMT_HEADER_ONLY)
    add_definitions(
        -DFMT_HEADER_ONLY
    )
endif()

if (SR_EMSCRIPTEN)
    add_definitions(-DFMT_USE_CONSTEXPR=0)
endif()

add_subdirectory(libs/fmt)

if (SR_TRACY_ENABLE)
    list(APPEND SR_UTILS_LINK_LIBRARIES libs/tracy/TracyClient.cpp)
endif()

add_subdirectory(libs/rapidyaml)
add_subdirectory(libs/entt)

list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Allocator.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/ICU.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Utils.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/ECS.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/World.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Platform.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Types.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Common.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Network.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Web.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/CSSParser.cxx)

define_property(GLOBAL PROPERTY SR_UTILS_INCLUDE_DIRECTORIES
    BRIEF_DOCS "Contains linkable libraries"
    FULL_DOCS "Contains paths to libraries for linking"
)

set_property(GLOBAL PROPERTY SR_UTILS_INCLUDE_DIRECTORIES "")

include(cmake/SRCommonIncludes.cmake)

if (SR_COMMON_ASIO)
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/asio/asio/include)
    target_include_directories(asio INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/libs/asio/asio/include)
endif()

if (SR_EFSW_USE)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(libs/efsw)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/efsw/include)
    add_compile_definitions(SR_EFSW_USE)
endif()

if (SR_COMMON_STATIC_LIBRARY)
    add_library(Utils STATIC ${SR_UTILS_LINK_LIBRARIES})
else()
    add_library(Utils SHARED ${SR_UTILS_LINK_LIBRARIES})
    add_library(Utils::lib ALIAS Utils)

    if (UNIX)
        target_compile_options(Utils PRIVATE -fPIC)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_link_libraries(Utils atomic)
        endif()
    endif()

    SR_COPY_SHARED_MODULE_TO_EXECUTABLE_FOLDER(Utils)
    SR_COPY_STATIC_LIBRARY_TO_LIBRARY_FOLDER(Utils)
endif()

if (NOT ANDROID_NDK AND UNIX AND NOT APPLE AND NOT SR_EMSCRIPTEN)
    target_link_libraries(Utils ${X11_LIBRARIES})
    target_link_libraries(Utils ${X11_Xrandr_LIB})
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${X11_Xrandr_INCLUDE_PATH})

    target_link_libraries(Utils ${X11_Xfixes_LIB})
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${X11_Xfixes_INCLUDE_PATH})

    if (X11_xcb_randr_FOUND)
        target_link_libraries(Utils ${X11_xcb_randr_LIB})
        SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${X11_xcb_randr_INCLUDE_PATH})
    endif()

    target_link_libraries(Utils bfd)
endif()

if (WIN32)
    target_link_libraries(Utils wininet)
endif()

if (SR_COMMON_MYHTML)
    set(MyHTML_BUILD_WITHOUT_THREADS ON CACHE INTERNAL "" FORCE)
    set(MyCORE_BUILD_WITHOUT_THREADS ON CACHE INTERNAL "" FORCE)
    set(MyHTML_BUILD_SHARED OFF CACHE INTERNAL "" FORCE)
    set(PROJECT_OPTIMIZATION_LEVEL "" CACHE INTERNAL "" FORCE)

    add_compile_definitions(SR_COMMON_MYHTML)

    add_subdirectory(libs/myhtml)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/myhtml/include)
    target_link_libraries(Utils myhtml_static)
endif()

if (SR_COMMON_LITEHTML)
    add_subdirectory(libs/litehtml)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/litehtml/include)
    target_link_libraries(Utils litehtml)
endif()

include(cmake/SRCommonLinking.cmake)

if (MSVC)
    target_compile_options(Utils PUBLIC /openmp)
endif()

if (SR_UTILS_FMT_HEADER_ONLY)
else()
    target_link_libraries(Utils fmt::fmt)
endif()

target_link_libraries(Utils ryml::ryml EnTT)

if (SR_COMMON_CPPTRACE_ENABLED)
    target_link_libraries(Utils cpptrace::cpptrace)
endif()

get_property(SR_UTILS_INCLUDE_DIRECTORIES_CONTENT GLOBAL PROPERTY SR_UTILS_INCLUDE_DIRECTORIES)
message("SRCommon include directories: ${SR_UTILS_INCLUDE_DIRECTORIES_CONTENT}")
target_include_directories(Utils PUBLIC ${SR_UTILS_INCLUDE_DIRECTORIES_CONTENT})

set(SR_UTILS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SR_UTILS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyFiles.cmake)

if (SR_COMMON_DLL_EXPORTS)
    message("SRCommon will be using DLL exports")
    target_compile_definitions(Utils PUBLIC SR_COMMON_DLL_EXPORTS)
endif()

set(SR_UTILS_DEPENDENCIES "")

if (NOT SR_COMMON_CI_BUILD)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyFiles.cmake)
    add_custom_target(CopyUtilsFiles)
    add_custom_command(TARGET CopyUtilsFiles
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DSR_UTILS_SOURCE_DIR=${SR_UTILS_SOURCE_DIR}
            -DSR_UTILS_BINARY_DIR=${SR_UTILS_BINARY_DIR}
            -DSR_ICU=${SR_ICU}
            -DSR_TRACY_ENABLE=${SR_TRACY_ENABLE}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyFiles.cmake"
        COMMENT "SRCommon: Copy utils source files to build directory"
    )

    set(SR_UTILS_DEPENDENCIES ${SR_UTILS_DEPENDENCIES} "CopyUtilsFiles")
endif()

SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/include)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/libs)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/libs/cssparser)

if (SR_COMMON_CI_BUILD)
    message("SRCommon is built as a part of CI build.")

    file(WRITE cmake/main.cpp "int main() { return 0; }")
    add_executable(SR_UTILS_CI cmake/main.cpp)
    target_link_libraries(SR_UTILS_CI Utils)
endif()

if (SR_COMMON_EMBED_RESOURCES)
    add_dependencies(${PROJECT_NAME} CopyUtilsFiles EmbedResourcesTarget)
endif()
