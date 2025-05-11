#pragma once

#include "common.hpp"
#include "scene.hpp"
#include "camera.hpp"

class Renderer {
public:
    Renderer(int width, int height);
    
    void render(const Scene& scene, const Camera& camera);
    const sf::Image& getImage() const { return image; }
    sf::Texture& getTexture();
    
    void setExposure(float value) { exposure = value; }
    void setMaxBounces(int bounces) { maxBounces = bounces; }
    void setSamplesPerPixel(int samples) { samplesPerPixel = samples; }
    int getSamplesPerPixel() const { return samplesPerPixel; }
    
private:
    Vec3 trace(const Ray& ray, const Scene& scene, int depth = 0);
    
    int width;
    int height;
    sf::Image image;
    sf::Texture texture;
    bool textureNeedsUpdate = true;
    
    float exposure = 1.0f;
    int maxBounces = 4;
    int samplesPerPixel = 1;
};