module;

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <format>
#include <iostream>
#include <cmath>   // Added for pow and other math functions

export module renderer;

import common;
import scene;
import camera;

export namespace rm {

class Renderer {
public:
    Renderer(int width, int height) : width(width), height(height) {
        image.create(width, height);
        textureNeedsUpdate = true;
    }
    
    void render(const Scene& scene, const Camera& camera) {
        // Multi-threaded rendering
        const int numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        std::atomic<int> nextRow(0);
        
        // Store thread-local pixel data
        std::vector<std::vector<sf::Color>> threadPixels(numThreads);
        std::vector<std::vector<int>> threadRows(numThreads);
        std::vector<std::vector<int>> threadCols(numThreads);
        
        for (int t = 0; t < numThreads; ++t) {
            // Pre-allocate memory for thread-local buffers
            threadPixels[t].reserve(width * height / numThreads);
            threadRows[t].reserve(width * height / numThreads);
            threadCols[t].reserve(width * height / numThreads);
            
            threads.emplace_back([&, t]() {
                int row;
                while ((row = nextRow.fetch_add(1)) < height) {
                    for (int x = 0; x < width; ++x) {
                        Vec3 pixelColor(0, 0, 0);
                        
                        // Supersampling
                        for (int s = 0; s < samplesPerPixel; ++s) {
                            float u = (x + (s % 2) * 0.5f) / float(width);
                            float v = (row + (s / 2) * 0.5f) / float(height);
                            
                            Ray ray = camera.getRay(u, v);
                            pixelColor = pixelColor + trace(ray, scene, maxBounces);
                        }
                        
                        // Average samples
                        pixelColor = pixelColor / float(samplesPerPixel);
                        
                        // Store pixel data in thread-local buffer
                        threadPixels[t].push_back(toColor(pixelColor, exposure));
                        threadRows[t].push_back(row);
                        threadCols[t].push_back(x);
                    }
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        // Combine all thread-local buffers into the final image
        for (int t = 0; t < numThreads; ++t) {
            for (size_t i = 0; i < threadPixels[t].size(); ++i) {
                image.setPixel(threadCols[t][i], threadRows[t][i], threadPixels[t][i]);
            }
        }
        
        textureNeedsUpdate = true;
    }
    
    const sf::Image& getImage() const { 
        return image; 
    }
    
    sf::Texture& getTexture() {
        if (textureNeedsUpdate) {
            texture.loadFromImage(image);
            textureNeedsUpdate = false;
        }
        return texture;
    }
    
    void setExposure(float value) { exposure = value; }
    void setMaxBounces(int bounces) { maxBounces = bounces; }
    void setSamplesPerPixel(int samples) { samplesPerPixel = samples; }
    int getSamplesPerPixel() const { return samplesPerPixel; }
    void setSkyColors(const Vec3& horizon, const Vec3& zenith) {
        skyHorizon = horizon;
        skyZenith = zenith;
    }
    void setGroundColors(const Vec3& horizon, const Vec3& nadir) {
        groundHorizon = horizon;
        groundNadir = nadir;
    }
    
private:
    Vec3 renderSky(const Ray& ray) const {
        float t = ray.direction.y;
        
        // For the sky (t >= 0)
        if (t >= 0) {
            // Smoother transition using a power curve
            float skyGradient = std::pow(t, 0.5f);
            return skyHorizon * (1.0f - skyGradient) + skyZenith * skyGradient;
        }
        // For the ground (t < 0)
        else {
            // Normalize t to [0,1] range for ground
            float groundGradient = std::pow(std::min(-t, 1.0f), 0.5f);
            return groundHorizon * (1.0f - groundGradient) + groundNadir * groundGradient;
        }
    }
    
    Vec3 trace(const Ray& ray, const Scene& scene, int depth = 0) {
        if (depth <= 0) {
            return Vec3(0, 0, 0); // Max depth reached
        }
        
        Hit hit;
        if (scene.march(ray, hit)) {
            Vec3 directLighting = scene.calculateLighting(hit, ray);
            
            // For mirror-like metals, calculate reflection
            if (hit.material.metallic > 0.9f && hit.material.roughness < 0.1f) {
                Vec3 reflectDir = ray.direction - hit.normal * 2.0f * ray.direction.dot(hit.normal);
                Ray reflectRay(hit.position + hit.normal * 0.001f, reflectDir);
                
                Vec3 reflectedColor = trace(reflectRay, scene, depth - 1);
                return directLighting + reflectedColor * hit.material.albedo * 0.8f;
            }
            
            return directLighting;
        }
        
        // Sky and ground rendering
        return renderSky(ray);
    }
    
    int width;
    int height;
    sf::Image image;
    sf::Texture texture;
    bool textureNeedsUpdate = true;
    
    float exposure = 1.0f;
    int maxBounces = 4;
    int samplesPerPixel = 1;
    
    // Sky and ground colors
    Vec3 skyHorizon = Vec3(0.8f, 0.9f, 1.0f);    // Light blue at horizon
    Vec3 skyZenith = Vec3(0.2f, 0.4f, 0.8f);     // Deep blue at zenith
    Vec3 groundHorizon = Vec3(0.7f, 0.75f, 0.6f); // Light ground at horizon
    Vec3 groundNadir = Vec3(0.3f, 0.35f, 0.2f);   // Dark ground at nadir
};

} // namespace rm