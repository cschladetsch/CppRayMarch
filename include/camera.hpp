#pragma once

#include "common.hpp"

class Camera {
public:
    Camera(float fov, float aspect);
    
    void setPosition(const Vec3& position);
    void setTarget(const Vec3& target);
    void setAspectRatio(float aspect);
    void setFOV(float fov);
    
    Ray getRay(float u, float v) const;
    
    const Vec3& getPosition() const { return position; }
    const Vec3& getForward() const { return forward; }
    const Vec3& getRight() const { return right; }
    const Vec3& getUp() const { return up; }

private:
    void updateVectors();

    Vec3 position{0.0f, 0.0f, 5.0f};
    Vec3 target{0.0f, 0.0f, 0.0f};
    
    Vec3 forward{0.0f, 0.0f, -1.0f};
    Vec3 right{1.0f, 0.0f, 0.0f};
    Vec3 up{0.0f, 1.0f, 0.0f};
    
    float fov;
    float aspect;
    float tanHalfFov;
};