if (SR_COMMON_OPENSSL)
    target_compile_definitions(Utils PUBLIC SR_COMMON_OPENSSL)
endif()

if (SR_COMMON_SDL)
    target_compile_definitions(Utils PUBLIC SR_COMMON_SDL)
endif()

if (SR_COMMON_LITEHTML)
    target_compile_definitions(Utils PUBLIC SR_COMMON_LITEHTML)
endif()

if (SR_COMMON_EMBED_RESOURCES)
    target_compile_definitions(Utils PUBLIC SR_COMMON_EMBED_RESOURCES)
endif()

if (SR_COMMON_GIT_METADATA)
    target_compile_definitions(Utils PUBLIC SR_COMMON_GIT_METADATA)
endif()

if (SR_COMMON_SDL)
    target_link_libraries(Utils SDL3::SDL3)
endif()

if (SR_COMMON_LIBPNG)
    target_link_libraries(Utils png_static)
endif()

if (SR_COMMON_MESHOPTIMIZER)
    target_link_libraries(Utils meshoptimizer)
endif()

if (SR_COMMON_ZLIB)
    target_link_libraries(Utils zlibstatic)
endif()

if (SR_COMMON_ASSIMP)
    target_link_libraries(Utils assimp)
endif()

if (SR_COMMON_CURL)
    target_link_libraries(Utils libcurl)
endif()

if (SR_COMMON_GIT_METADATA)
    target_link_libraries(Utils cmake_git_version_tracking)
endif()

if (SR_COMMON_JSON)
    target_link_libraries(Utils nlohmann_json)
endif()

if (NOT ANDROID_NDK AND NOT SR_EMSCRIPTEN)
    target_link_libraries(Utils nfd) # nativefiledialog
    target_link_libraries(Utils TinyFileDialogs)
    target_compile_definitions(Utils PUBLIC SR_COMMON_USE_TINY_FILE_DIALOGS)
endif()

if (SR_TRACY_ENABLE)
    target_compile_definitions(Utils PUBLIC TRACY_DELAYED_INIT)
    target_compile_definitions(Utils PUBLIC TRACY_MANUAL_LIFETIME)
    target_compile_definitions(Utils PUBLIC TRACY_BROADCAST_PORT=8086)
endif()

if (SR_EFSW_USE)
    target_link_libraries(Utils efsw)
endif()

if (SR_COMMON_ASIO)
    target_link_libraries(Utils asio)
    target_compile_definitions(Utils PUBLIC SR_COMMON_ASIO)
endif()

if (SR_COMMON_GNS)
    target_link_libraries(Utils GameNetworkingSockets_s)
    target_compile_definitions(Utils PUBLIC SR_COMMON_GNS)
endif()