#!/bin/bash

# Simple build script for CppRayMarch

# Create build directory if it doesn't exist
mkdir -p build-clang

# Build using CMake and Ninja
cd build-clang

# Configure with CMake using Clang and Ninja
cmake -G Ninja \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_BUILD_TYPE=Release \
      ..

# Build with Ninja
ninja

# Exit with the status of the ninja command
exit $?