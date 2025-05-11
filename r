#!/bin/bash

# Build and run script for CppRayMarch

# First, run the build script
./b

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful, running application..."
    # Run the application
    ./build-clang/raymarch
else
    echo "Build failed, not running application."
    exit 1
fi