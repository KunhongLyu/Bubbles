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
    src/application/application.cpp)

set(APPLICATION_HEADERS
    src/application/application.h)

if (WIN32)
    list(APPEND APPLICATION_SOURCE src/util/win32/getopt.c)
endif()

#if (WIN32 OR APPLE)
#  add_library(pt31 STATIC ${APPLICATION_3_1_SOURCE} ${APPLICATION_HEADERS})
#else()
#  add_library(pt31 SHARED ${APPLICATION_3_1_SOURCE} ${APPLICATION_HEADERS})
#endif()

#target_include_directories(pt31 PUBLIC src)

add_executable(bubble ${APPLICATION_SOURCE} ${APPLICATION_HEADERS})
target_include_directories(bubble PUBLIC src)

#target_link_libraries(pt31 PUBLIC CGL)
target_link_libraries(bubble PUBLIC CGL)

set(CGL_INCLUDE_DIRS CGL/include CGL/deps/glew/include CGL/deps/glfw/include src/imgui src/imgui/backends)

#-------------------------------------------------------------------------------
# Find dependencies
#-------------------------------------------------------------------------------
add_subdirectory(CGL)
#target_include_directories(pt31 PUBLIC ${CGL_INCLUDE_DIRS})
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
