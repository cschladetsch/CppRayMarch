module;

#include <numbers>
#include <cmath>

export module camera;

import common;

export class Camera {
public:
    Camera(float fov, float aspect) : fov(fov), aspect(aspect) {
        tanHalfFov = std::tan(fov * 0.5f * std::numbers::pi_v<float> / 180.0f);
        updateVectors();
    }
    
    void setPosition(const Vec3& position) {
        this->position = position;
        updateVectors();
    }
    
    void setTarget(const Vec3& target) {
        this->target = target;
        updateVectors();
    }
    
    void setAspectRatio(float aspect) {
        this->aspect = aspect;
    }
    
    void setFOV(float fov) {
        this->fov = fov;
        tanHalfFov = std::tan(fov * 0.5f * std::numbers::pi_v<float> / 180.0f);
    }
    
    Ray getRay(float u, float v) const {
        // Convert from [0,1] to [-1,1]
        float nx = (2.0f * u - 1.0f) * aspect * tanHalfFov;
        float ny = (1.0f - 2.0f * v) * tanHalfFov;
        
        Vec3 direction = forward + right * nx + up * ny;
        return Ray(position, direction.normalize());
    }
    
    const Vec3& getPosition() const { return position; }
    const Vec3& getForward() const { return forward; }
    const Vec3& getRight() const { return right; }
    const Vec3& getUp() const { return up; }

private:
    void updateVectors() {
        forward = (target - position).normalize();
        
        // Handle the case where camera is looking straight up or down
        if (std::abs(forward.y) > 0.99999f) {
            right = Vec3(1.0f, 0.0f, 0.0f);
            up = Vec3(0.0f, forward.y > 0 ? 0.0f : 1.0f, forward.y > 0 ? -1.0f : 0.0f);
        } else {
            // Use world up as reference to compute right and up vectors
            Vec3 worldUp(0.0f, 1.0f, 0.0f);
            right = worldUp.cross(forward).normalize();
            up = forward.cross(right).normalize();
        }
    }

    Vec3 position{0.0f, 0.0f, 5.0f};
    Vec3 target{0.0f, 0.0f, 0.0f};
    
    Vec3 forward{0.0f, 0.0f, -1.0f};
    Vec3 right{1.0f, 0.0f, 0.0f};
    Vec3 up{0.0f, 1.0f, 0.0f};
    
    float fov;
    float aspect;
    float tanHalfFov;
};