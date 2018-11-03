cmake_minimum_required(VERSION 3.0.0)

# glfw
set(GLFW_BUILD_EXAMPLES  OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS     OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS      OFF CACHE BOOL " " FORCE)
set(GLFW_INSTALL         OFF CACHE BOOL " " FORCE)

if (MSVC)
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL " " FORCE)
endif ()

add_subdirectory(glfw)
set_property(TARGET glfw PROPERTY FOLDER "dependencies")

# glew
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL " " FORCE)
set(glew-cmake_BUILD_STATIC ON  CACHE BOOL " " FORCE)

add_subdirectory(glew)
set_property(TARGET libglew_static PROPERTY FOLDER "dependencies")

# imgui
set(
    INCLUDE_DIRS
    glfw/include
    glew/include
)
include_directories(${INCLUDE_DIRS})
add_library(
    imgui_static STATIC
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_glfw.h
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
)
target_link_libraries(imgui_static libglew_static glfw)
set_property(TARGET imgui_static PROPERTY FOLDER "dependencies")

# zlib(only on Windows)
if (WIN32)
  set(ZLIB_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
  set(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
  add_subdirectory(zlib)

  set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE PATH " " FORCE)
  set(ZLIB_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/zlib/$<CONFIGURATION>/zlibstatic.lib" CACHE FILEPATH " " FORCE)

  set_property(TARGET zlibstatic PROPERTY FOLDER "dependencies")
  include_directories(${ZLIB_INCLUDE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/zlib")
endif()

# Build OpenEXR
set(ILMBASE_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(OPENEXR_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(ILMBASE_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
set(OPENEXR_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
add_subdirectory(openexr)
set_property(TARGET IexMath eLut toFloat b44ExpLogTable dwaLookups CopyIlmBaseLibs IlmThread Half Iex Imath IlmImf IlmImf-obj PROPERTY FOLDER "dependencies")

# Build Thread Building Blocks (main shared libraries only)
set(TBB_BUILD_SHARED          OFF  CACHE BOOL " " FORCE)
set(TBB_BUILD_STATIC          ON   CACHE BOOL " " FORCE)
set(TBB_BUILD_TBBMALLOC       OFF  CACHE BOOL " " FORCE)
set(TBB_BUILD_TBBMALLOC_PROXY OFF  CACHE BOOL " " FORCE)
add_subdirectory(tbb)
set_property(TARGET tbb_static tbb_def_files PROPERTY FOLDER "dependencies")

# Build pugixml
if(MSVC)
    set(STATIC_CRT ON   CACHE BOOL " " FORCE)
endif()
add_subdirectory(pugixml)
set_property(TARGET pugixml PROPERTY FOLDER "dependencies")

# Build glog
set(WITH_GFLAGS       OFF  CACHE BOOL " " FORCE)
set(BUILD_TESTING     OFF  CACHE BOOL " " FORCE)
set(BUILD_SHARED_LIBS OFF  CACHE BOOL " " FORCE)
add_subdirectory(glog)
set_property(TARGET glog PROPERTY FOLDER "dependencies")

# include dir
set(OPENEXR_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/openexr/IlmBase/Imath
    ${CMAKE_CURRENT_SOURCE_DIR}/openexr/IlmBase/Iex
    ${CMAKE_CURRENT_SOURCE_DIR}/openexr/IlmBase/Half
    ${CMAKE_CURRENT_SOURCE_DIR}/openexr/OpenEXR/IlmImf
    ${CMAKE_CURRENT_BINARY_DIR}/openexr/OpenEXR/config
    ${CMAKE_CURRENT_BINARY_DIR}/openexr/IlmBase/config
)

set(GLFW_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
)

set(GLEW_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/glew/include
)

set(IMGUI_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

set(TBB_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/tbb/include
)

set(EIGEN_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/eigen
)

set(PUGIXML_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src
)

set(
    TFM_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyformat
)

set(
    PCG32_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/pcg32
)

set(
    GLOG_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/glog/src
)

set(
    FILESYSTEM_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem
)

# Compile remainder of the codebase with compiler warnings turned on
if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-anonymous-struct -Wno-c99-extensions -Wno-nested-anon-types -Wno-deprecated-register")
    endif()
endif()

set(CompilerFlags
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO COMPILE_DEFINITIONS U_CMAKE_BUILD_TYPE
    CMAKE_MACOSX_RPATH
    OPENEXR_INCLUDE_DIRS  
    GLFW_INCLUDE_DIR 
    GLEW_INCLUDE_DIR  
    IMGUI_INCLUDE_DIR
    TBB_INCLUDE_DIR  
    EIGEN_INCLUDE_DIR
    PUGIXML_INCLUDE_DIR
    TFM_INCLUDE_DIR
    PCG32_INCLUDE_DIR
    GLOG_INCLUDE_DIR
    FILESYSTEM_INCLUDE_DIR
)

foreach(CompilerFlag ${CompilerFlags})
    set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()