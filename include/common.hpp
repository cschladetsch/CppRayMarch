#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <memory>
#include <array>
#include <numbers>
#include <algorithm>
#include <format>

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const { return *this / length(); }
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction) {}
    
    Vec3 at(float t) const { return origin + direction * t; }
};

struct Material {
    Vec3 albedo;  // Base color
    float metallic;  // 0 = dielectric, 1 = metallic
    float roughness;  // 0 = smooth, 1 = rough
    float emissive;  // Emission strength
    
    Material() : albedo(1.0f, 1.0f, 1.0f), metallic(0.0f), roughness(0.5f), emissive(0.0f) {}
    Material(const Vec3& albedo, float metallic = 0.0f, float roughness = 0.5f, float emissive = 0.0f)
        : albedo(albedo), metallic(metallic), roughness(roughness), emissive(emissive) {}
};

struct Hit {
    float distance;
    Vec3 position;
    Vec3 normal;
    Material material;
    
    Hit() : distance(std::numeric_limits<float>::max()) {}
};

// Convert Vec3 to SFML Color
inline sf::Color toColor(const Vec3& v, float exposure = 1.0f) {
    auto gamma = [](float x) { return std::pow(std::clamp(x, 0.0f, 1.0f), 1.0f / 2.2f); };
    
    Vec3 exposed = v * exposure;
    return sf::Color(
        static_cast<uint8_t>(gamma(exposed.x) * 255),
        static_cast<uint8_t>(gamma(exposed.y) * 255),
        static_cast<uint8_t>(gamma(exposed.z) * 255)
    );
}