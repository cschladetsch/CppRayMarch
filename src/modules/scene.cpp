module;

#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <limits>

export module scene;

import common;

export namespace rm {

class SDF {
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

class Sphere : public SDF {
public:
    Sphere(const Vec3& center, float radius) : center(center), radius(radius) {}
    
    float distance(const Vec3& point) const override {
        return (point - center).length() - radius;
    }
    
private:
    Vec3 center;
    float radius;
};

class Box : public SDF {
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

class Torus : public SDF {
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

class Plane : public SDF {
public:
    Plane(const Vec3& normal, float distance) : normal(normal.normalize()), distanceFromOrigin(distance) {}
    
    float distance(const Vec3& point) const override {
        return normal.dot(point) + distanceFromOrigin;
    }
    
private:
    Vec3 normal;
    float distanceFromOrigin;
};

class Cylinder : public SDF {
public:
    Cylinder(const Vec3& center, float radius, float height) : center(center), radius(radius), height(height) {}
    
    float distance(const Vec3& point) const override {
        // Vector from center to point in xz-plane
        Vec3 p = point - center;
        
        // Distance in xz-plane
        float d = std::sqrt(p.x * p.x + p.z * p.z) - radius;
        
        // Distance in y-direction
        float y = std::abs(p.y) - height * 0.5f;
        
        // Combine distances
        d = std::max(d, y);
        
        return d;
    }
    
private:
    Vec3 center;
    float radius;
    float height;
};

class Union : public SDF {
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

class Subtraction : public SDF {
public:
    Subtraction(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b) : a(a), b(b) {}
    
    float distance(const Vec3& point) const override {
        return std::max(a->distance(point), -b->distance(point));
    }
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
};

class Intersection : public SDF {
public:
    Intersection(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b) : a(a), b(b) {}
    
    float distance(const Vec3& point) const override {
        return std::max(a->distance(point), b->distance(point));
    }
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
};

// Smooth minimum for blending
class SmoothUnion : public SDF {
public:
    SmoothUnion(std::shared_ptr<SDF> a, std::shared_ptr<SDF> b, float k) : a(a), b(b), k(k) {}
    
    float distance(const Vec3& point) const override {
        float distA = a->distance(point);
        float distB = b->distance(point);
        
        float h = std::clamp(0.5f + 0.5f * (distB - distA) / k, 0.0f, 1.0f);
        float result = distB * (1.0f - h) + distA * h - k * h * (1.0f - h);
        
        // Track which shape is closest for material
        if (h > 0.5f) {
            closestShape = a;
        } else {
            closestShape = b;
        }
        
        return result;
    }
    
    Material getMaterial() const override {
        return closestShape ? closestShape->getMaterial() : material;
    }
    
private:
    std::shared_ptr<SDF> a;
    std::shared_ptr<SDF> b;
    float k; // Smoothing factor
    mutable std::shared_ptr<SDF> closestShape;
};

// Domain repetition (infinite repetition)
class RepetitionSDF : public SDF {
public:
    RepetitionSDF(std::shared_ptr<SDF> shape, const Vec3& spacing) 
        : shape(shape), spacing(spacing) {}
    
    float distance(const Vec3& point) const override {
        Vec3 modPoint = Vec3(
            spacing.x > 0 ? std::fmod(point.x + 0.5f * spacing.x, spacing.x) - 0.5f * spacing.x : point.x,
            spacing.y > 0 ? std::fmod(point.y + 0.5f * spacing.y, spacing.y) - 0.5f * spacing.y : point.y,
            spacing.z > 0 ? std::fmod(point.z + 0.5f * spacing.z, spacing.z) - 0.5f * spacing.z : point.z
        );
        return shape->distance(modPoint);
    }
    
    Material getMaterial() const override {
        return shape->getMaterial();
    }
    
private:
    std::shared_ptr<SDF> shape;
    Vec3 spacing;
};

class Scene {
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

} // namespace rm