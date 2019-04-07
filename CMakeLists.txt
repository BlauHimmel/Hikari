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
        ${FILESYSTEM_INCLUDE_DIR}
        ${HYPOTHESIS_INCLUDE_DIR}
        ${STB_LIB_INCLUDE_DIR}
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
)

#########################################################################

set(
        INCLUDE_DIRS

        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(
        SOURCE_FILES

        ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/acceleration/BVHAcceleration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/acceleration/HLBVHAcceleration.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/ConductorBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/DielectricBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/DiffuseBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/MicrofacetBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/MirrorBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/PlasticBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/RoughConductorBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/RoughDielectricBSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bsdf/RoughDiffuseBSDF.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/camera/PerspectiveCamera.cpp
        
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Acceleration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Bitmap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Block.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/BoundingBox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/BSDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Chi2Test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Color.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Common.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/DiscretePDF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Emitter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Frame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Integrator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Intersection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/MemoryArena.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Mesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/MicrofacetDistribution.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/MipMap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Object.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/PropertyList.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Ray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ReconstructionFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Sampling.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Scene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Screen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Shape.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/StudentTTest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Timer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Transform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Vector.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/emitter/AreaLight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/emitter/ConstantLight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/emitter/DirectionalLight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/emitter/EnvironmentLight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/emitter/PointLight.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/BoxFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/GaussianFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/MitchellNetravalFilter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/filter/TentFilter.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/integrator/AoIntegrator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/integrator/NormalIntegrator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/integrator/PathEMSIntegrator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/integrator/PathMATSIntegrator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/integrator/PathMISIntegrator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/integrator/SimpleIntegrator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/integrator/WhittedIntegrator.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/mesh/WavefrontObjMesh.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/sampler/IndependentSampler.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/BitmapTexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/CheckerboardTexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/CurvatureTexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/GridTexture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/WireframeTexture.cpp
)

set(
        HEADER_FILS

        ${CMAKE_CURRENT_SOURCE_DIR}/include/acceleration/BVHAcceleration.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/acceleration/HLBVHAcceleration.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/ConductorBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/DielectricBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/DiffuseBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/MicrofacetBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/MirrorBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/PlasticBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/RoughConductorBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/RoughDielectricBSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bsdf/RoughDiffuseBSDF.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/camera/PerspectiveCamera.hpp
        
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Acceleration.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Bitmap.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Block.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/BoundingBox.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/BSDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Camera.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Chi2Test.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Color.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Common.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/DiscretePDF.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Emitter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Frame.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Integrator.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Intersection.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/MemoryArena.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Mesh.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/MicrofacetDistribution.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/MipMap.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Object.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Parser.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/PropertyList.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Ray.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/ReconstructionFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Sampler.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Sampling.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Scene.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Screen.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Shape.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/StudentTTest.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Texture.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Timer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Transform.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Vector.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/emitter/AreaLight.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/emitter/ConstantLight.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/emitter/DirectionalLight.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/emitter/EnvironmentLight.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/emitter/PointLight.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/BoxFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/GaussianFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/MitchellNetravalFilter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/filter/TentFilter.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/integrator/AoIntegrator.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/integrator/NormalIntegrator.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/integrator/PathEMSIntegrator.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/integrator/PathMATSIntegrator.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/integrator/PathMISIntegrator.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/integrator/SimpleIntegrator.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/integrator/WhittedIntegrator.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/mesh/WavefrontObjMesh.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/sampler/IndependentSampler.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/texture/BitmapTexture.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/texture/CheckerboardTexture.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/texture/CurvatureTexture.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/texture/GridTexture.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/texture/WireframeTexture.hpp
)

#########################################################################

include_directories(${EXTENSION_INCLUDE_DIRS} ${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILS})

target_link_libraries(${PROJECT_NAME} ${EXTENSION_LIBS})