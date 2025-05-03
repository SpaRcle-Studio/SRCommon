project(TinyFileDialogs)

message(STATUS "SpaRcle Engine: SRCommon library will be using TinyFileDialogs")

add_library(TinyFileDialogs STATIC tinyfiledialogs.c)

if (UNIX)
    target_compile_options(TinyFileDialogs PRIVATE -fPIC)
endif()

target_include_directories(TinyFileDialogs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})