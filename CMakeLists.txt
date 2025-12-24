cmake_minimum_required(VERSION 3.14)
project(sample_analytics_plugin)

# Path to the unpacked VMS Metadata SDK
set(metadataSdkDir "" CACHE PATH "Path to unpacked VMS Metadata SDK zip.")
if(metadataSdkDir STREQUAL "")
    set(metadataSdkDir ${CMAKE_CURRENT_LIST_DIR}/../..) # Assume building samples inside the SDK.
    if(NOT EXISTS ${metadataSdkDir}/src/nx/sdk OR NOT EXISTS ${metadataSdkDir}/nx_kit/src/nx/kit)
        message(FATAL_ERROR "Define metadataSdkDir cache variable to point to the unzipped SDK.")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# rpath settings for Linux
if(UNIX)
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
endif()

set(CMAKE_SKIP_BUILD_RPATH ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

# MSVC multi-processor compilation + debug info workaround
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /MP")
    add_compile_options("/Z7")
endif()

# Define export macro for plugin API
if(WIN32)
    set(API_EXPORT_MACRO "__declspec(dllexport)")
else()
    set(API_EXPORT_MACRO "__attribute__((visibility(\"default\")))")
endif()

#--------------------------------------------------------------------------------------------------
# Find OpenCV (with dnn, contrib; ffmpeg feature enables FFmpeg in videoio)
#--------------------------------------------------------------------------------------------------
find_package(OpenCV REQUIRED CONFIG COMPONENTS core imgproc imgcodecs videoio dnn)
# Note: No separate 'contrib' COMPONENT—vcpkg includes contrib modules in the main libs if feature enabled.
#       'ml' removed as it's not a standard vcpkg COMPONENT.

# Optional: Verify DNN is available at configure time
if(NOT TARGET opencv_dnn)
    message(FATAL_ERROR "OpenCV DNN module not available. Install with 'opencv4[dnn]' feature.")
endif()

#--------------------------------------------------------------------------------------------------
# nx_kit (static)
#--------------------------------------------------------------------------------------------------
set(nxKitLibraryType "STATIC" CACHE STRING "" FORCE)
set(nxKitWithTests "NO" CACHE STRING "" FORCE)
add_subdirectory(${metadataSdkDir}/nx_kit ${CMAKE_CURRENT_BINARY_DIR}/nx_kit)

#--------------------------------------------------------------------------------------------------
# nx_sdk (static)
#--------------------------------------------------------------------------------------------------
set(SDK_SRC_DIR ${metadataSdkDir}/src)
file(GLOB_RECURSE SDK_SRC CONFIGURE_DEPENDS ${SDK_SRC_DIR}/*)
add_library(nx_sdk STATIC ${SDK_SRC})
target_include_directories(nx_sdk PUBLIC ${SDK_SRC_DIR})
target_link_libraries(nx_sdk PRIVATE nx_kit)
target_compile_definitions(nx_sdk PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO})

#--------------------------------------------------------------------------------------------------
# sample_analytics_plugin (shared library)
#--------------------------------------------------------------------------------------------------
set(SAMPLE_ANALYTICS_PLUGIN_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE SAMPLE_ANALYTICS_PLUGIN_SRC CONFIGURE_DEPENDS
    ${SAMPLE_ANALYTICS_PLUGIN_SRC_DIR}/*)
add_library(sample_analytics_plugin SHARED ${SAMPLE_ANALYTICS_PLUGIN_SRC})

target_include_directories(sample_analytics_plugin
    PRIVATE
        ${SAMPLE_ANALYTICS_PLUGIN_SRC_DIR}
    PUBLIC
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(sample_analytics_plugin
    PRIVATE
        nx_kit
        nx_sdk
        opencv_core     # Always
        opencv_imgproc  # Common for analytics
        opencv_imgcodecs# Image loading/saving
        opencv_videoio  # Video I/O with FFmpeg
        opencv_dnn      # DNN module
        # Add more if needed, e.g., other contrib modules expose as opencv_<module>
)

target_compile_definitions(sample_analytics_plugin
    PRIVATE
        NX_PLUGIN_API=${API_EXPORT_MACRO}
        WITH_OPENCV
        WITH_FFMPEG  # Since OpenCV uses FFmpeg internally
)

# Optional: If using opencv_world (enable with opencv4[world] feature), simplify to:
# target_link_libraries(sample_analytics_plugin PRIVATE nx_kit nx_sdk opencv_world)