cmake_minimum_required(VERSION 3.16)
project(Utils)

set(CMAKE_CXX_STANDARD 20)

option(SR_UTILS_CMAKE_ZLIB "" ON)
option(SR_UTILS_CMAKE_ASSIMP "" ON)
option(SR_UTILS_DLL_EXPORTS "" ON)
option(SR_UTILS_CI_BUILD "" OFF)
option(SR_UTILS_USE_SDL "" OFF)

if (SR_UTILS_USE_SDL)
    add_compile_definitions(SR_UTILS_USE_SDL)
endif()

macro(SR_UTILS_INCLUDE_DIRECTORIES_APPEND var)
    set_property(GLOBAL APPEND PROPERTY SR_UTILS_INCLUDE_DIRECTORIES "${var}")
endmacro(SR_UTILS_INCLUDE_DIRECTORIES_APPEND)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX")
endif()

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


if (SR_UTILS_DLL_EXPORTS)
    add_definitions(
        -DSR_DLL_EXPORTS=1
    )
    add_compile_definitions(
        -DSR_DLL_EXPORTS=1
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSR_DLL_EXPORTS")
endif()

if (SR_UTILS_CMAKE_ZLIB)
    set(ZLIB_BUILD_EXAMPLES OFF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASSIMP_BUILD_ZLIB=OFF")
    set(ASSIMP_BUILD_ZLIB OFF CACHE INTERNAL "" FORCE)
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/zlib ${CMAKE_CURRENT_BINARY_DIR}/libs/zlib)
    add_subdirectory(libs/zlib)
endif()

if (SR_UTILS_CMAKE_ASSIMP)
    message("SpaRcle utils will be using Assimp")

    add_definitions(
        -DSR_UTILS_ASSIMP=1
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSR_UTILS_ASSIMP")
    add_compile_definitions(
        SR_UTILS_ASSIMP
    )

    set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE INTERNAL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE INTERNAL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE)

    #if (ANDROID_NDK)
    #    set(ASSIMP_BUILD_ZLIB OFF CACHE INTERNAL "" FORCE)
    #else()
    #    set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "" FORCE)
    #endif()

    add_subdirectory(libs/assimp)
endif()

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

if (SR_UTILS_USE_SDL)
    add_subdirectory(libs/SDL)
endif()

set(ICU_PREBUILT_VERSION "69.1")

if (NOT ANDROID_NDK)
    add_subdirectory(libs/nativefiledialog)
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

add_subdirectory(libs/fmt)

if (SR_TRACY_ENABLE)
    add_subdirectory(libs/tracy)
endif()

add_subdirectory(libs/rapidyaml)
add_subdirectory(libs/cppcoro)
add_subdirectory(libs/cpptrace)

list(APPEND SR_UTILS_LINK_LIBRARIES cxx/ICU.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Utils.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/ECS.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/World.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Platform.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Types.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Common.cxx)
list(APPEND SR_UTILS_LINK_LIBRARIES cxx/Network.cxx)

define_property(GLOBAL PROPERTY SR_UTILS_INCLUDE_DIRECTORIES
    BRIEF_DOCS "Contains linkable libraries"
    FULL_DOCS "Contains paths to libraries for linking"
)

set_property(GLOBAL PROPERTY SR_UTILS_INCLUDE_DIRECTORIES "")

if (SR_UTILS_CMAKE_ZLIB)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/zlib)
endif()

if (SR_UTILS_CMAKE_ASSIMP)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/libs/assimp/include)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/assimp/include)
endif()

if (SR_UTILS_USE_SDL)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL/include)
endif()

SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/libs/nativefiledialog/src/include)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/nativefiledialog/src/include)

SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/inc)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/glm)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/fmt/include)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/cppcoro/include)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/cpptrace/include)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/rapidyaml/src)

SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/openssl/include)

SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/include/Utils/EmbedResources)

add_library(asio INTERFACE)

target_include_directories(asio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/asio/include)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/libs/asio/include)

if (SR_ICU)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/include/icu)
endif()

if (SR_TRACY_ENABLE)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/include/tracy/tracy)
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_SOURCE_DIR}/libs/tracy/tracy)
endif()

if (UNIX)
    list(APPEND SR_UTILS_LINK_LIBRARIES src/Utils/Platform/LinuxStacktraceImpl.cpp)
endif()

if (SR_UTILS_STATIC_LIBRARY)
    add_library(Utils STATIC ${SR_UTILS_LINK_LIBRARIES})

    if (CYGWIN)

    else()

    endif()
else()
    add_library(Utils SHARED ${SR_UTILS_LINK_LIBRARIES})
    add_library(Utils::lib ALIAS Utils)
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(Utils ${X11_LIBRARIES})
    target_link_libraries(Utils ${X11_Xrandr_LIB})
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${X11_Xrandr_INCLUDE_PATH})

    target_link_libraries(Utils ${XFIXES_LIBRARIES})
    SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${XFIXES_INCLUDE_DIRS})

    if (X11_xcb_randr_FOUND)
        target_link_libraries(Utils ${X11_xcb_randr_LIB})
        SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${X11_xcb_randr_INCLUDE_PATH})
    endif()

    target_link_libraries(Utils bfd)
endif()

if (SR_UTILS_USE_SDL)
    target_link_libraries(Utils SDL3::SDL3)
endif()

if (SR_UTILS_CMAKE_ZLIB)
    target_link_libraries(Utils zlibstatic)
endif()

if (SR_UTILS_CMAKE_ASSIMP)
    target_link_libraries(Utils assimp)
endif()

if (ANDROID_NDK)
else()
    target_link_libraries(Utils nfd)
endif()

if (SR_TRACY_ENABLE)
    target_link_libraries(Utils tracy)
endif()

if (MSVC)
    target_compile_options(Utils PUBLIC /openmp)
endif()

if (SR_UTILS_FMT_HEADER_ONLY)
else()
    target_link_libraries(Utils fmt)
endif()

target_link_libraries(Utils asio)
target_link_libraries(Utils ryml::ryml cppcoro cpptrace::cpptrace)

get_property(SR_UTILS_INCLUDE_DIRECTORIES_CONTENT GLOBAL PROPERTY SR_UTILS_INCLUDE_DIRECTORIES)
message("SpaRcle utils include directories: ${SR_UTILS_INCLUDE_DIRECTORIES_CONTENT}")
target_include_directories(Utils PUBLIC ${SR_UTILS_INCLUDE_DIRECTORIES_CONTENT})

set(SR_UTILS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SR_UTILS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyFiles.cmake)

if (SR_UTILS_DLL_EXPORTS)
    message("SpaRcle utils will be using DLL exports")
    target_compile_definitions(Utils PUBLIC
        SR_DLL_EXPORTS
    )
endif()

set(SR_UTILS_DEPENDENCIES "")

if (NOT SR_UTILS_CI_BUILD)
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
        COMMENT "SpaRcle Engine: Copy utils source files to build directory"
    )

    set(SR_UTILS_DEPENDENCIES  ${SR_UTILS_DEPENDENCIES} "CopyUtilsFiles ")
endif()

SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/include)
SR_UTILS_INCLUDE_DIRECTORIES_APPEND(${CMAKE_CURRENT_BINARY_DIR}/libs)

if (SR_UTILS_CI_BUILD)
    message("SRCommon is built as a part of CI build.")

    add_compile_definitions(SR_UTILS_CI_BUILD)
    include(cmake/ResourceEmbedder.cmake)
    EmbedResources(" " " " " ")
    set(SR_UTILS_DEPENDENCIES  ${SR_UTILS_DEPENDENCIES} "EmbedResourcesTarget ")

    file(WRITE cmake/main.cpp "int main() { return 0; }")
    add_executable(SR_UTILS_CI cmake/main.cpp)
    target_link_libraries(SR_UTILS_CI Utils)
endif()

#set(SR_UTILS_DEPENDENCIES  ${SR_UTILS_DEPENDENCIES} "EmbedResourcesTarget")

if (NOT SR_UTILS_CI_BUILD)
    add_dependencies(${PROJECT_NAME} CopyUtilsFiles EmbedResourcesTarget) #SRUtilsCodegenTarget
endif()
