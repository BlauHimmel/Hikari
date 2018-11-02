cmake_minimum_required(VERSION 3.10)
project(Hikari)

# basic
include(CheckCXXCompilerFlag)

# Set a default build configuration (Release)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
        "MinSizeRel" "RelWithDebInfo")
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

# Enable folders for projects in Visual Studio
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if (APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")

    CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11_FLAG)

    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    elseif (HAS_CPP11_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        message(FATAL_ERROR "Unsupported compiler -- Requires C++11 support at least!")
    endif()

    # Enable link time optimization and set the default symbol
    # visibility to hidden (very important to obtain small binaries)
    if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
        # Default symbol visibility
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    endif()
endif()

# Sanitize build environment for static build with C++14
if (MSVC)

    CHECK_CXX_COMPILER_FLAG("/std:c++14" HAS_CPP14_FLAG)

    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
    endif()

    # Disable annoying secure CRT warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS")

    # We'll select the TBB library ourselves
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__TBB_NO_IMPLICIT_LINKAGE")

    # Parallel build on MSVC (all targets)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")

        # Disable Eigen vectorization for Windows 32 bit builds (issues with unaligned access segfaults)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEIGEN_DONT_ALIGN")
    endif()

  # Static build
  set(CompilerFlags
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
elseif(APPLE)
    # Try to auto-detect a suitable SDK
    execute_process(COMMAND bash -c "xcodebuild -version -sdk | grep MacOSX | grep Path | head -n 1 | cut -f 2 -d ' '" OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
    string(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
    string(REGEX REPLACE "^.*X([0-9.]*).sdk$" "\\1" CMAKE_OSX_DEPLOYMENT_TARGET "${CMAKE_OSX_SYSROOT}")
endif()

#########################################################################

# static link glew library
add_definitions(-DGLEW_STATIC)

# static link glog library
add_definitions(-DGOOGLE_GLOG_DLL_DECL=)
if(MSVC)
    add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
endif()

# add extension libraries
add_subdirectory(ext)

# vulkan
find_package(Vulkan)
if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()

set(
        EXTENSION_INCLUDE_DIRS

        ${GLFW_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}
        ${OPENEXR_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${PUGIXML_INCLUDE_DIR}
        ${TFM_INCLUDE_DIR}
        ${PCG32_INCLUDE_DIR}
        ${GLOG_INCLUDE_DIR}
)

set(
        EXTENSION_LIBS

        libglew_static
        glfw
        imgui_static
        tbb_static
        IlmImf
        pugixml
        glog
        ${Vulkan_LIBRARIES}
)

#########################################################################

set(
        INCLUDE_DIRS

        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(
        SOURCE_FILES

        ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
        
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Bitmap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Block.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/BoundingBox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Color.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Common.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Frame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Object.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/PropertyList.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Ray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ReconstructionFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Timer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Transform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Vector.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/BoxFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/GaussianFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/MitchellNetravalFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/TentFilter.cpp

)

set(
        HEADER_FILS

        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Bitmap.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Block.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/BoundingBox.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Color.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Common.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Frame.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Object.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/PropertyList.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Ray.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/ReconstructionFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Timer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Transform.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Vector.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/BoxFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/GaussianFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/MitchellNetravalFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/TentFilter.hpp
)

#########################################################################

include_directories(${EXTENSION_INCLUDE_DIRS} ${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILS})

target_link_libraries(${PROJECT_NAME} ${EXTENSION_LIBS})