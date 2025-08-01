cmake_minimum_required(VERSION 3.11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Bubble)

#-------------------------------------------------------------------------------
# Build options
#-------------------------------------------------------------------------------
option(BUILD_DEBUG     "Build with debug settings"    OFF)
option(BUILD_DOCS      "Build documentation"          OFF)


set(BUILD_DEBUG ${BUILD_DEBUG} CACHE BOOL "Build debug" FORCE)

if (BUILD_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
endif()

#-------------------------------------------------------------------------------
# Set target
#-------------------------------------------------------------------------------
set(APPLICATION_SOURCE
    src/main.cpp
    src/application.cpp
)

set(APPLICATION_HEADERS
    src/application.h
    src/util/Shader.h
    
    src/pathtracer/random_util.h
    src/pathtracer/bvh.h
    src/pathtracer/intersection.h
    src/pathtracer/primitive.h
    src/pathtracer/bbox.h
    src/pathtracer/triangle.h
    src/pathtracer/ray.h
    src/pathtracer/bsdf.h
    src/pathtracer/sampler.h
)

set(GL_UTILS_SOURCE
    src/util/Shader.cpp
)

set(PATH_TRACER_SOURCE
    src/pathtracer/bvh.cpp
    src/pathtracer/bbox.cpp
    src/pathtracer/triangle.cpp
    src/pathtracer/bsdf.cpp
    src/pathtracer/advanced_bsdf.cpp
    src/pathtracer/sampler.cpp
)

if (WIN32)
    list(APPEND APPLICATION_SOURCE src/util/win32/getopt.c)
endif()


if (WIN32 OR APPLE)
    add_library(gl_utils STATIC ${GL_UTILS_SOURCE} ${APPLICATION_HEADERS})
    add_library(pathtracer STATIC ${PATH_TRACER_SOURCE} ${APPLICATION_HEADERS})
else()
    add_library(gl_utils SHARED ${GL_UTILS_SOURCE} ${APPLICATION_HEADERS})
    add_library(pathtracer SHARED ${PATH_TRACER_SOURCE} ${APPLICATION_HEADERS})
endif()

target_include_directories(gl_utils PUBLIC src)
target_include_directories(pathtracer PUBLIC src)

add_executable(bubble ${APPLICATION_SOURCE} ${APPLICATION_HEADERS})
target_include_directories(bubble PUBLIC src)

target_link_libraries(gl_utils PUBLIC CGL)
target_link_libraries(pathtracer PUBLIC CGL)
target_link_libraries(bubble PUBLIC pathtracer)
target_link_libraries(bubble PUBLIC gl_utils)

set(CGL_INCLUDE_DIRS CGL/include CGL/deps/glew/include CGL/deps/glfw/include src/imgui src/imgui/backends)

#-------------------------------------------------------------------------------
# Find dependencies
#-------------------------------------------------------------------------------
add_subdirectory(CGL)
target_include_directories(gl_utils PUBLIC ${CGL_INCLUDE_DIRS})
target_include_directories(bubble PUBLIC ${CGL_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CGL_CXX_FLAGS}")

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
target_link_libraries(bubble PUBLIC OpenGL::GL)
target_link_libraries(bubble PUBLIC OpenGL::GLU)

#-------------------------------------------------------------------------------
# Add subdirectories
#-------------------------------------------------------------------------------

# build documentation
if(BUILD_DOCS)
  find_package(DOXYGEN)
  if(DOXYGEN_FOUND AND BUILD_DOCS)
    add_subdirectory(docs)
  endif()
endif()

# Install settings
set(CMAKE_INSTALL_PREFIX "${Bubble_SOURCE_DIR}/")
