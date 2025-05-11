module;

#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>

export module scene;

import common;

export class SDF {
public:
    virtual ~SDF() = default;
    virtual float distance(const Vec3& point) const = 0;
    
    Vec3 normal(const Vec3& point) const {
        const float h = 0.0001f;
        const Vec3 dx(h, 0, 0);
        const Vec3 dy(0, h, 0);
        const Vec3 dz(0, 0, h);
        
        return Vec3(
            distance(point + dx) - distance(point - dx),
            distance(point + dy) - distance(point - dy),
            distance(point + dz) - distance(point - dz)
        ).normalize();
    }
    
    virtual Material getMaterial() const { return material; }
    
    void setMaterial(const Material& mat) { material = mat; }
    
protected:
    Material material;
};

export class Sphere : public SDF {
public:
    Sphere(const Vec3& center, float radius) : center(center), radius(radius) {}
    
    float distance(const Vec3& point) const override {
        return (point - center).length() - radius;
    }
    
private:
    Vec3 center;
    float radius;
};

export class Box : public SDF {
public:
    Box(const Vec3& center, const Vec3& dimensions) : center(center), dimensions(dimensions) {}
    
    float distance(const Vec3& point) const override {
        Vec3 q = Vec3(
            std::abs(point.x - center.x) - dimensions.x * 0.5f,
            std::abs(point.y - center.y) - dimensions.y * 0.5f,
            std::abs(point.z - center.z) - dimensions.z * 0.5f
        );
        
        return std::min(std::max(q.x, std::max(q.y, q.z)), 0.0f) +
               Vec3(std::max(q.x, 0.0f), std::max(q.y, 0.0f), std::max(q.z, 0.0f)).length();
    }
    
private:
    Vec3 center;
    Vec3 dimensions;
};

export class Torus : public SDF {
public:
    Torus(const Vec3& center, float majorRadius, float minorRadius)
        : center(center), majorRadius(majorRadius), minorRadius(minorRadius) {}
    
    float distance(const Vec3& point) const override {
        Vec3 p = point - center;
        Vec3 q = Vec3(Vec3(p.x, 0.0f, p.z).length() - majorRadius, p.y, 0.0f);
        return q.length() - minorRadius;
    }
    
private:
    Vec3 center;
    float majorRadius;
    float minorRadius;
};

export class Union : public SDF {
public:
    Union(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b) : a(a), b(b) {}
    
    float distance(const Vec3& point) const override {
        float distA = a->distance(point);
        float distB = b->distance(point);
        
        if (distA < distB) {
            closestShape = a;
            return distA;
        } else {
            closestShape = b;
            return distB;
        }
    }
    
    Material getMaterial() const override {
        return closestShape ? closestShape->getMaterial() : material;
    }
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
    mutable std::shared_ptr<SDF> closestShape;
};

export class Subtraction : public SDF {
public:
    Subtraction(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b) : a(a), b(b) {}
    
    float distance(const Vec3& point) const override {
        return std::max(a->distance(point), -b->distance(point));
    }
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
};

export class Intersection : public SDF {
public:
    Intersection(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b) : a(a), b(b) {}
    
    float distance(const Vec3& point) const override {
        return std::max(a->distance(point), b->distance(point));
    }
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
};

export class Scene {
public:
    Scene() {}
    
    void add(std::shared_ptr<SDF> object) {
        objects.push_back(object);
    }
    
    bool march(const Ray& ray, Hit& hit, float maxDist = 100.0f, float epsilon = 0.001f) const {
        float t = 0.0f;
        
        for (int i = 0; i < 100; ++i) {
            Vec3 pos = ray.at(t);
            
            float minDist = std::numeric_limits<float>::max();
            std::shared_ptr<SDF> closestObject = nullptr;
            
            for (const auto& object : objects) {
                float d = object->distance(pos);
                if (d < minDist) {
                    minDist = d;
                    closestObject = object;
                }
            }
            
            if (minDist < epsilon) {
                hit.distance = t;
                hit.position = pos;
                hit.normal = closestObject->normal(pos);
                hit.material = closestObject->getMaterial();
                return true;
            }
            
            t += minDist;
            
            if (t > maxDist) {
                break;
            }
        }
        
        return false;
    }
    
    void setAmbientLight(const Vec3& color) { ambientLight = color; }
    
    void addLight(const Vec3& position, const Vec3& color, float intensity = 1.0f) {
        lights.push_back({position, color, intensity});
    }
    
    Vec3 calculateLighting(const Hit& hit, const Ray& ray) const {
        Vec3 color = hit.material.albedo * ambientLight;
        
        for (const auto& light : lights) {
            Vec3 lightDir = (light.position - hit.position).normalize();
            float diffuse = std::max(0.0f, lightDir.dot(hit.normal));
            
            // Shadow check
            Ray shadowRay(hit.position + hit.normal * 0.001f, lightDir);
            Hit shadowHit;
            bool inShadow = march(shadowRay, shadowHit, (light.position - hit.position).length());
            
            if (!inShadow) {
                // Diffuse component
                color = color + hit.material.albedo * light.color * diffuse * light.intensity;
                
                // Specular component for metals
                if (hit.material.metallic > 0.0f) {
                    Vec3 reflectDir = ray.direction - hit.normal * 2.0f * ray.direction.dot(hit.normal);
                    float spec = std::pow(std::max(0.0f, reflectDir.dot(lightDir)), 
                                         32.0f * (1.0f - hit.material.roughness));
                    color = color + hit.material.albedo * light.color * spec * hit.material.metallic * light.intensity;
                }
            }
        }
        
        // Add emissive component
        if (hit.material.emissive > 0.0f) {
            color = color + hit.material.albedo * hit.material.emissive;
        }
        
        return color;
    }
    
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