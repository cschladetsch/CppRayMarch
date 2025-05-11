# C++23 Ray Marching with Modules

A real-time ray marching renderer implemented in C++23 using modules and SFML.

## Features

- Full C++23 module support
- Signed Distance Field (SDF) rendering
- Constructive Solid Geometry (CSG) operations
- Basic materials and lighting
- Multi-threaded rendering

## Requirements

- C++23 compatible compiler with modules support (Clang 16+, GCC 13+, or MSVC 19.34+)
- SFML 2.5+
- CMake 3.28+
- Ninja build system (optional but recommended)

## Building with Clang and Ninja

For optimal C++23 modules support, we recommend using Clang and Ninja build system:

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install cmake libsfml-dev clang ninja-build

# Run the build script
./build-clang.sh
```

## Building with GCC

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install cmake libsfml-dev g++-13

# Run the GCC build script
./build.sh
```

## Usage

Run the executable generated in the build directory:

```bash
# If built with Clang/Ninja
./build-clang/raymarch

# If built with GCC
./build/raymarch
```

Controls:
- ESC: Exit the application
- SPACE: Force re-render (when animation is paused)

## Project Structure

The project uses C++23 modules:

- `common` - Basic types (Vec3, Ray, Material) and utility functions
- `camera` - Camera implementation for perspective projection
- `scene` - SDF primitives, CSG operations, and scene management
- `renderer` - Multi-threaded renderer with lighting and reflections

## Customizing the Scene

To customize the scene, modify the `main.cpp` file to add different SDFs and CSG operations.

Example:

```cpp
// Create a sphere
auto sphere = std::make_shared<Sphere>(Vec3(0.0f, 0.0f, 0.0f), 1.0f);
sphere->setMaterial(Material(Vec3(0.8f, 0.2f, 0.2f), 0.5f, 0.1f));
scene.add(sphere);

// Create a box
auto box = std::make_shared<Box>(Vec3(2.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f));
box->setMaterial(Material(Vec3(0.2f, 0.8f, 0.2f), 0.0f, 0.5f));
scene.add(box);

// Create a union of two shapes
auto union_shape = std::make_shared<Union>(sphere, box);
scene.add(union_shape);
```

## Troubleshooting

If you encounter build errors related to modules:

1. Ensure you're using a recent compiler with good C++23 module support
2. Try using the Clang build with Ninja for better modules support
3. Check that CMake version is at least 3.28
4. Set environment variable `CMAKE_CXX_STANDARD=23` before building

## License

[MIT License](LICENSE)