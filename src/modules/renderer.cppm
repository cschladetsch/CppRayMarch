module;

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <format>
#include <iostream>

export module renderer;

import common;
import scene;
import camera;

export class Renderer {
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
        std::mutex imageMutex;
        
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&]() {
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
                        
                        // Lock when writing to image
                        std::lock_guard<std::mutex> lock(imageMutex);
                        image.setPixel(x, row, toColor(pixelColor, exposure));
                    }
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
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
    
private:
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
        
        // Sky color - simple gradient
        float t = 0.5f * (ray.direction.y + 1.0f);
        return Vec3(1.0f, 1.0f, 1.0f) * (1.0f - t) + Vec3(0.5f, 0.7f, 1.0f) * t;
    }
    
    int width;
    int height;
    sf::Image image;
    sf::Texture texture;
    bool textureNeedsUpdate = true;
    
    float exposure = 1.0f;
    int maxBounces = 4;
    int samplesPerPixel = 1;
};