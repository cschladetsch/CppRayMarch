#!/bin/bash

# Check if clang is installed
if ! command -v clang++ &> /dev/null; then
    echo "Error: clang++ not found. Please install clang."
    exit 1
fi

# Check if ninja is installed
if ! command -v ninja &> /dev/null; then
    echo "Error: ninja not found. Please install ninja-build."
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p build-clang
cd build-clang

# Configure with CMake using Clang and Ninja
cmake -G Ninja \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_BUILD_TYPE=Release \
      ..

# Build with Ninja
ninja

echo "Build complete. Run with: ./build-clang/raymarch"