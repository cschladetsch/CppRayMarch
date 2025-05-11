#pragma once

#include "common.hpp"

// Forward declaration
class Scene;

// SDF (Signed Distance Function) base class
class SDF {
public:
    virtual ~SDF() = default;
    virtual float distance(const Vec3& point) const = 0;
    virtual Vec3 normal(const Vec3& point) const;
    virtual Material getMaterial() const { return material; }
    
    void setMaterial(const Material& mat) { material = mat; }
    
protected:
    Material material;
};

// Sphere SDF
class Sphere : public SDF {
public:
    Sphere(const Vec3& center, float radius);
    float distance(const Vec3& point) const override;
    
private:
    Vec3 center;
    float radius;
};

// Box SDF
class Box : public SDF {
public:
    Box(const Vec3& center, const Vec3& dimensions);
    float distance(const Vec3& point) const override;
    
private:
    Vec3 center;
    Vec3 dimensions;
};

// Torus SDF
class Torus : public SDF {
public:
    Torus(const Vec3& center, float majorRadius, float minorRadius);
    float distance(const Vec3& point) const override;
    
private:
    Vec3 center;
    float majorRadius;
    float minorRadius;
};

// Union operation
class Union : public SDF {
public:
    Union(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b);
    float distance(const Vec3& point) const override;
    Material getMaterial() const override;
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
    mutable std::shared_ptr<SDF> closestShape;
};

// Subtraction operation
class Subtraction : public SDF {
public:
    Subtraction(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b);
    float distance(const Vec3& point) const override;
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
};

// Intersection operation
class Intersection : public SDF {
public:
    Intersection(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b);
    float distance(const Vec3& point) const override;
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
};

// Scene class to manage objects and perform ray marching
class Scene {
public:
    Scene();
    
    void add(std::shared_ptr<SDF> object);
    bool march(const Ray& ray, Hit& hit, float maxDist = 100.0f, float epsilon = 0.001f) const;
    
    void setAmbientLight(const Vec3& color) { ambientLight = color; }
    void addLight(const Vec3& position, const Vec3& color, float intensity = 1.0f);
    
    Vec3 calculateLighting(const Hit& hit, const Ray& ray) const;
    
private:
    std::vector<std::shared_ptr<SDF>> objects;
    
    Vec3 ambientLight{0.1f, 0.1f, 0.1f};
    
    struct Light {
        Vec3 position;
        Vec3 color;
        float intensity;
    };
    
    std::vector<Light> lights;
};