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
# Find Eigen (add this near other find_package calls)
#-------------------------------------------------------------------------------
find_package(Eigen3 REQUIRED)

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
# Source files
#-------------------------------------------------------------------------------
file(GLOB_RECURSE APPLICATION_SOURCE "src/*.cpp")

file(GLOB_RECURSE APPLICATION_HEADERS "*.h")

if (WIN32)
    list(APPEND APPLICATION_SOURCE src/util/win32/getopt.c)
endif()



#-------------------------------------------------------------------------------
# Create final program
#-------------------------------------------------------------------------------
add_executable(bubble ${APPLICATION_SOURCE} ${APPLICATION_HEADERS})


#-------------------------------------------------------------------------------
# Create final program
#-------------------------------------------------------------------------------
target_include_directories(bubble PUBLIC src)


#-------------------------------------------------------------------------------
# Find include directories
#-------------------------------------------------------------------------------
set(CGL_INCLUDE_DIRS CGL/include CGL/deps/glew/include CGL/deps/glfw/include src/imgui src/imgui/backends)

add_subdirectory(CGL)
target_include_directories(bubble PUBLIC ${CGL_INCLUDE_DIRS})
target_include_directories(bubble PUBLIC 
    src
    ${EIGEN3_INCLUDE_DIR}
    ${CGL_INCLUDE_DIRS}
)




#-------------------------------------------------------------------------------
# Link libraries
#-------------------------------------------------------------------------------
target_link_libraries(bubble PUBLIC CGL)
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
target_link_libraries(bubble PUBLIC OpenGL::GL)
target_link_libraries(bubble PUBLIC OpenGL::GLU)
target_link_libraries(bubble PUBLIC CGL OpenGL::GL OpenGL::GLU)




#-------------------------------------------------------------------------------
# Copy shader files
#-------------------------------------------------------------------------------
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND}
        -D SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -D TARGET_DIR=$<TARGET_FILE_DIR:bubble>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/copy_shaders.cmake
    COMMENT "Copying shaders if changed or new"
)






set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CGL_CXX_FLAGS}")

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
