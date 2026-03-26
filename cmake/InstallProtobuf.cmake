include(FetchContent)

# куда ставим vcpkg
set(VCPKG_DIR "${CMAKE_BINARY_DIR}/vcpkg")

# если нет vcpkg — качаем
if(NOT EXISTS "${VCPKG_DIR}")
    message(STATUS "vcpkg not found, downloading...")

    execute_process(
        COMMAND git clone https://github.com/microsoft/vcpkg ${VCPKG_DIR}
        RESULT_VARIABLE GIT_RESULT
    )

    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone vcpkg")
    endif()

    # bootstrap
    if(WIN32)
        set(BOOTSTRAP_CMD "${VCPKG_DIR}/bootstrap-vcpkg.bat")
    else()
        set(BOOTSTRAP_CMD "${VCPKG_DIR}/bootstrap-vcpkg.sh")
    endif()

    execute_process(
        COMMAND ${BOOTSTRAP_CMD}
        WORKING_DIRECTORY ${VCPKG_DIR}
        RESULT_VARIABLE BOOTSTRAP_RESULT
    )

    if(NOT BOOTSTRAP_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to bootstrap vcpkg")
    endif()
endif()

# подключаем toolchain ДО find_package
set(CMAKE_TOOLCHAIN_FILE
    "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING ""
)

# ставим protobuf если нет
execute_process(
    COMMAND ${VCPKG_DIR}/vcpkg install protobuf
    RESULT_VARIABLE VCPKG_INSTALL_RESULT
)

if(NOT VCPKG_INSTALL_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to install protobuf via vcpkg")
endif()

list(APPEND CMAKE_PREFIX_PATH "${VCPKG_DIR}/installed/x64-windows")
set(Protobuf_PROTOC_EXECUTABLE "${VCPKG_DIR}/installed/x64-windows/tools/protobuf/protoc.exe")

find_package(Protobuf REQUIRED)

if(NOT TARGET protobuf::protoc)
    add_executable(protobuf::protoc IMPORTED)
    set_target_properties(protobuf::protoc PROPERTIES
        IMPORTED_LOCATION "${Protobuf_PROTOC_EXECUTABLE}"
    )
endif()