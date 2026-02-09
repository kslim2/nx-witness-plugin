cmake_minimum_required(VERSION 3.14)
project(face_recognition_analytics_plugin LANGUAGES CXX)

# Allow user to point to Metadata SDK
set(metadataSdkDir "" CACHE PATH "Path to unpacked VMS Metadata SDK")
if(metadataSdkDir STREQUAL "")
    set(metadataSdkDir ${CMAKE_CURRENT_LIST_DIR}/../..)
    if(NOT EXISTS ${metadataSdkDir}/src/nx/sdk OR NOT EXISTS ${metadataSdkDir}/nx_kit/src/nx/kit)
        message(FATAL_ERROR "Define metadataSdkDir to point to the unzipped Metadata SDK")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Linux: use $ORIGIN rpath
if(UNIX)
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
endif()

set(CMAKE_SKIP_BUILD_RPATH ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

# Faster MSVC builds
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /MP")
    add_compile_options("/Z7")
endif()

# DLL import/export macros
if(WIN32)
    set(API_IMPORT_MACRO "__declspec(dllimport)")
    set(API_EXPORT_MACRO "__declspec(dllexport)")
else()
    set(API_EXPORT_MACRO "__attribute__((visibility(\"default\")))")
endif()

# ────────────────────────────────────────────────
# nx_kit (static)
# ────────────────────────────────────────────────
set(nxKitLibraryType "STATIC" CACHE STRING "" FORCE)
set(nxKitWithTests "NO" CACHE STRING "" FORCE)
add_subdirectory(${metadataSdkDir}/nx_kit ${CMAKE_CURRENT_BINARY_DIR}/nx_kit)

# ────────────────────────────────────────────────
# nx_sdk (static)
# ────────────────────────────────────────────────
set(SDK_SRC_DIR ${metadataSdkDir}/src)
file(GLOB_RECURSE SDK_SRC CONFIGURE_DEPENDS ${SDK_SRC_DIR}/*.cpp ${SDK_SRC_DIR}/*.h)
add_library(nx_sdk STATIC ${SDK_SRC})
target_include_directories(nx_sdk PUBLIC ${SDK_SRC_DIR})
target_link_libraries(nx_sdk PRIVATE nx_kit)
target_compile_definitions(nx_sdk PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO})

# ────────────────────────────────────────────────
# zlib (static library) — must come BEFORE cnpy
# ────────────────────────────────────────────────
add_subdirectory(external/zlib)

# Force CMake to see our built zlib
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib" CACHE PATH "" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE FILEPATH "" FORCE)
set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)

mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY ZLIB_FOUND)

# ────────────────────────────────────────────────
# cnpy (for .npy support)
# ────────────────────────────────────────────────
add_subdirectory(external/cnpy)

# Make sure cnpy really uses our zlib (belt and suspenders approach)
target_include_directories(cnpy PRIVATE ${ZLIB_INCLUDE_DIR})

# ────────────────────────────────────────────────
# face_recognition_plugin (shared library)
# ────────────────────────────────────────────────
set(PLUGIN_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE PLUGIN_SRC
    ${PLUGIN_SRC_DIR}/*.cpp
    ${PLUGIN_SRC_DIR}/*.h
)

add_library(face_recognition_plugin SHARED ${PLUGIN_SRC})

target_include_directories(face_recognition_plugin
    PRIVATE ${PLUGIN_SRC_DIR}
)

target_link_libraries(face_recognition_plugin
    PRIVATE
        nx_kit
        nx_sdk
        cnpy
        zlibstatic          # also link zlibstatic here for safety
)

target_compile_definitions(face_recognition_plugin
    PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO}
)

# Filesystem library for older gcc versions
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(face_recognition_plugin PRIVATE stdc++fs)
endif()

if(NOT WIN32)
    target_link_libraries(face_recognition_plugin PRIVATE pthread)
endif()

# Optional: install rules
install(TARGETS face_recognition_plugin
    LIBRARY DESTINATION "analytics_plugins"
    RUNTIME DESTINATION "analytics_plugins"
)

# Optional: copy models / default data
# install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/models/
#     DESTINATION "analytics_plugins/models"
#     FILES_MATCHING PATTERN "*.onnx")