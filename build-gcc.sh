#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Create module cache directory
mkdir -p gcm.cache

# Function to compile a module
compile_module() {
    local module_name="$1"
    local dependencies="$2"
    
    echo "Compiling module: $module_name"
    
    # Dependency flags
    local dep_flags=""
    if [ ! -z "$dependencies" ]; then
        for dep in $dependencies; do
            if [ -f "gcm.cache/${dep}.gcm" ]; then
                dep_flags="$dep_flags -fmodule-file=gcm.cache/${dep}.gcm"
            fi
        done
    fi
    
    g++ -std=c++23 -fmodules-ts $dep_flags -c -x c++ \
        -o ${module_name}.o \
        ../src/modules/${module_name}.cpp
    
    # Check if compilation succeeded
    if [ $? -ne 0 ]; then
        echo "Error compiling module: $module_name"
        exit 1
    fi
}

# Compile modules in dependency order
compile_module "common"
compile_module "camera" "common"
compile_module "scene" "common"
compile_module "renderer" "common camera scene"

# Compile main program
echo "Compiling main program"
g++ -std=c++23 -fmodules-ts \
    -fmodule-file=gcm.cache/common.gcm \
    -fmodule-file=gcm.cache/camera.gcm \
    -fmodule-file=gcm.cache/scene.gcm \
    -fmodule-file=gcm.cache/renderer.gcm \
    -c -o main.o ../src/main.cpp

# Link everything
echo "Linking..."
g++ -o raymarch main.o common.o camera.o scene.o renderer.o -lsfml-graphics -lsfml-window -lsfml-system

echo "Build complete. Run with: ./raymarch"