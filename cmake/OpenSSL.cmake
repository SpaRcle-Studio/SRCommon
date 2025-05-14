#if (SR_COMMON_OPENSSL)
#    # Prefer static linking
#    set(OPENSSL_USE_STATIC_LIBS TRUE)
#    set(OPENSSL_MSVC_STATIC_RUNTIME TRUE)
#
#    # Additional paths to search for OpenSSL
#    if (UNIX AND NOT ANDROID_NDK)
#        # Common Linux OpenSSL paths
#        list(APPEND CMAKE_PREFIX_PATH
#                "/usr/local/openssl"
#                "/usr/local/ssl"
#                "/usr/local"
#                "/usr"
#        )
#
#        # Additional library and include paths
#        list(APPEND CMAKE_LIBRARY_PATH
#                "/usr/local/openssl/lib"
#                "/usr/local/ssl/lib"
#                "/usr/local/lib"
#                "/usr/lib/x86_64-linux-gnu"
#        )
#
#        list(APPEND CMAKE_INCLUDE_PATH
#                "/usr/local/openssl/include"
#                "/usr/local/ssl/include"
#                "/usr/local/include"
#                "/usr/include/openssl"
#        )
#    endif()
#
#    # On macOS
#    if (APPLE)
#        # Homebrew OpenSSL paths
#        list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/openssl")
#        list(APPEND CMAKE_LIBRARY_PATH "/usr/local/opt/openssl/lib")
#        list(APPEND CMAKE_INCLUDE_PATH "/usr/local/opt/openssl/include")
#    endif()
#
#    # On Windows
#    if (WIN32)
#        # Potential OpenSSL installation paths
#        list(APPEND CMAKE_PREFIX_PATH
#                "C:/OpenSSL-Win64"
#                "C:/Program Files/OpenSSL"
#                "C:/Program Files/OpenSSL-Win64"
#        )
#    endif()
#
#    add_compile_definitions(SR_COMMON_OPENSSL)
#    find_package(OpenSSL REQUIRED)
#
#    include_directories(${OPENSSL_INCLUDE_DIR})
#
#    if (OpenSSL_FOUND)
#        message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
#        message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
#        message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
#    else()
#        message(FATAL_ERROR "System OpenSSL is not found.")
#    endif()
#endif()