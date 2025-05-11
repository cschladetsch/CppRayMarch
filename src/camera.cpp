#include "camera.hpp"

Camera::Camera(float fov, float aspect) : fov(fov), aspect(aspect) {
    tanHalfFov = std::tan(fov * 0.5f * std::numbers::pi_v<float> / 180.0f);
    updateVectors();
}

void Camera::setPosition(const Vec3& position) {
    this->position = position;
    updateVectors();
}

void Camera::setTarget(const Vec3& target) {
    this->target = target;
    updateVectors();
}

void Camera::setAspectRatio(float aspect) {
    this->aspect = aspect;
}

void Camera::setFOV(float fov) {
    this->fov = fov;
    tanHalfFov = std::tan(fov * 0.5f * std::numbers::pi_v<float> / 180.0f);
}

Ray Camera::getRay(float u, float v) const {
    // Convert from [0,1] to [-1,1]
    float nx = (2.0f * u - 1.0f) * aspect * tanHalfFov;
    float ny = (1.0f - 2.0f * v) * tanHalfFov;
    
    Vec3 direction = forward + right * nx + up * ny;
    return Ray(position, direction.normalize());
}

void Camera::updateVectors() {
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