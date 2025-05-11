cmake_minimum_required(VERSION 3.28)
project(CppRayMarch VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable C++23 modules support
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

# Find SFML
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

# Define the module files
add_library(raymond_modules)
target_sources(raymond_modules
  PUBLIC
    FILE_SET CXX_MODULES FILES
      src/modules/common.cpp
      src/modules/camera.cpp
      src/modules/scene.cpp
      src/modules/renderer.cpp
)
target_link_libraries(raymond_modules PRIVATE sfml-system sfml-window sfml-graphics)

# Define the executable
add_executable(raymarch src/main.cpp)
target_link_libraries(raymarch PRIVATE raymond_modules sfml-system sfml-window sfml-graphics)

# Set include directories
target_include_directories(raymarch PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Copy any needed runtime dependencies
if(WIN32)
  add_custom_command(TARGET raymarch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:raymarch> $<TARGET_FILE_DIR:raymarch>
    COMMAND_EXPAND_LISTS
  )
endif()
