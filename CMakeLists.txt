cmake_minimum_required(VERSION 3.15)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
project(face_plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(metadataSdkVersion "5.1.1.37512")
set(metadataSdkDir ${CMAKE_CURRENT_LIST_DIR}/include/metadata_sdk)

if(metadataSdkDir STREQUAL "")
  message(FATAL_ERROR "metadataSdkDir cache variable in undefined.")
endif()

if(NOT DEFINED ARTIFACTORY_URL)
  set(ARTIFACTORY_URL "https://artifactory.nxvms.dev/artifactory/artifacts/nx_open_integrations")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(UNIX)
    # In Linux, for the plugin .so library, set `rpath` to "$ORIGIN" and do not set `runpath`, thus
    # enabling the lookup of the dependencies in the plugin dir first.
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

#-------------------------------------------------------------------
# Define OpenCV
message("message===== ${VCPKG_ROOT}")


set(FACE_PLUGIN_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE FACE_PLUGIN_SOURCES
    "${FACE_PLUGIN_SRC_DIR}/*.cpp"
    "${FACE_PLUGIN_SRC_DIR}/*.c"
)
if(NOT FACE_PLUGIN_SOURCES)
    message(FATAL_ERROR "No source files found in: ${FACE_PLUGIN_SRC_DIR}. Check the path and file extensions.")
endif()



# --- Define the target (This is now Line 72 in the corrected code) ---
add_library(face_plugin SHARED ${FACE_PLUGIN_SOURCES})


find_package(OpenCV REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core dnn tracking)

target_include_directories(face_plugin PRIVATE 
          ${FACE_PLUGIN_SRC_DIR}
          ${OpenCV_INCLUDE_DIRS})

#--------------------------------------------------------------------
# define nx_kit lib, static
set(nxKitLibraryType "STATIC" CACHE STRING "" FORCE)
set(nxKitWithTests "YES" CACHE STRING "" FORCE)

add_subdirectory(${metadataSdkDir}/nx_kit ${CMAKE_CURRENT_BINARY_DIR}/nx_kit)

#--------------------------------------------------------------------
# define nx_sdk lib, static, depends on nx_kit
set(SDK_SRC_DIR ${metadataSdkDir}/src)
file(GLOB_RECURSE SDK_SRC ${SDK_SRC_DIR}/*)

add_library(nx_sdk STATIC ${SDK_SRC})
target_include_directories(nx_sdk PUBLIC ${SDK_SRC_DIR})
target_link_libraries(nx_sdk PRIVATE nx_kit)

target_compile_definitions(nx_sdk PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO})

#-------------------------------------------------------------------------------
# define face_plugin lib, depends on nx_kit, nx_sdk, opencv, faiss, zmpcmq

set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(
  face_plugin
    PRIVATE
    opencv_core        # Ensure core is explicitly linked
    opencv_dnn         # Found via COMPONENTS dnn
    opencv_tracking    # Found via COMPONENTS tracking
    nx_kit
    nx_sdk
)
target_compile_definitions(face_plugin PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO})