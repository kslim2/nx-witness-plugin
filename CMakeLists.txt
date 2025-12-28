## Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/
cmake_minimum_required(VERSION 3.10)
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")  # Keep this as backup, but the conf above overrides it
project(opencv_object_detection_analytics_plugin)

set(metadataSdkVersion "5.1.1.37512")
set(metadataSdkDir "" CACHE PATH
    "Path to unpacked VMS Metadata SDK zip. Tested with version ${metadataSdkVersion}.")
if(metadataSdkDir STREQUAL "")
    message(FATAL_ERROR "metadataSdkDir cache variable is undefined.")
endif()

if(NOT DEFINED ARTIFACTORY_URL)
    set(ARTIFACTORY_URL "https://artifactory.nxvms.dev/artifactory/artifacts/nx_open_integrations")
endif()

set(OpenCV_DIR "C:/Users/a/Downloads/opencv-4.11.0/build")  # Use forward slashes or double backslashes
find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR})
message(STATUS "Found OpenCV: ${OpenCV_VERSION} at ${OpenCV_DIR}")

# Download model files (unchanged)
set(model_file_names
    MobileNetSSD.caffemodel
    MobileNetSSD.prototxt
)
foreach(model_file_name IN LISTS model_file_names)
    set(model_file "${CMAKE_BINARY_DIR}/${model_file_name}")
    list(APPEND model_files "${model_file}")
    if(NOT EXISTS "${model_file}")
        message("Downloading ${model_file_name}")
        file(DOWNLOAD
            "${ARTIFACTORY_URL}/opencv_object_detection_analytics_plugin/models/${model_file_name}"
            "${model_file}"
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
        )
        list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
        if(NOT STATUS_CODE EQUAL 0)
            file(REMOVE "${model_file}")
            message(FATAL_ERROR "Error downloading ${ARTIFACTORY_URL}/opencv_object_detection_analytics_plugin/models/${model_file_name}")
        endif()
    else()
        message("${model_file_name} already exists")
    endif()
endforeach()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX)
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
endif()

set(CMAKE_SKIP_BUILD_RPATH ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

if(WIN32)
    set(API_EXPORT_MACRO "__declspec(dllexport)")
elseif(UNIX)
    set(API_EXPORT_MACRO "__attribute__((visibility(\"default\")))")
endif()

# nx_kit, nx_sdk, plugin (unchanged)
set(nxKitLibraryType "STATIC" CACHE STRING "" FORCE)
set(nxKitWithTests "YES" CACHE STRING "" FORCE)
add_subdirectory(${metadataSdkDir}/nx_kit ${CMAKE_CURRENT_BINARY_DIR}/nx_kit)

set(SDK_SRC_DIR ${metadataSdkDir}/src)
file(GLOB_RECURSE SDK_SRC ${SDK_SRC_DIR}/*)
add_library(nx_sdk STATIC ${SDK_SRC})
target_include_directories(nx_sdk PUBLIC ${SDK_SRC_DIR})
target_link_libraries(nx_sdk PRIVATE nx_kit)
target_compile_definitions(nx_sdk PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO})

set(OPENCV_OBJECT_DETECTION_ANALYTICS_PLUGIN_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE OPENCV_OBJECT_DETECTION_ANALYTICS_PLUGIN_SRC
    ${OPENCV_OBJECT_DETECTION_ANALYTICS_PLUGIN_SRC_DIR}/*)
add_library(opencv_object_detection_analytics_plugin SHARED
    ${OPENCV_OBJECT_DETECTION_ANALYTICS_PLUGIN_SRC})

target_include_directories(opencv_object_detection_analytics_plugin PRIVATE
    ${OPENCV_OBJECT_DETECTION_ANALYTICS_PLUGIN_SRC_DIR})  # Fixed typo in original (was _ANALYTICS_PLUGIN_SRC_DIR)

target_link_libraries(opencv_object_detection_analytics_plugin
    nx_kit
    nx_sdk
    opencv_core          # <- plain name
    opencv_imgproc
    opencv_imgcodecs
    opencv_dnn
    opencv_flann
    opencv_features2d
    opencv_calib3d
    opencv_ml
    opencv_video
    opencv_tracking
    # Remove any that cause "not found" errors (e.g., plot, datasets – they may not be built with your options)
)

target_compile_definitions(opencv_object_detection_analytics_plugin
    PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO}
)