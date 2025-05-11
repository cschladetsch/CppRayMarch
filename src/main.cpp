#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <format>
#include <cmath>
#include <array>
#include <string>

import common;
import camera;
import scene;
import renderer;

// Helper function to draw text
void drawText(sf::RenderWindow& window, const std::string& text, const sf::Vector2f& position,
              const sf::Font& font, unsigned int size = 16, sf::Color color = sf::Color::White) {
    sf::Text textObject;
    textObject.setFont(font);
    textObject.setString(text);
    textObject.setCharacterSize(size);
    textObject.setFillColor(color);
    textObject.setPosition(position);
    window.draw(textObject);
}

int main() {
    // Window setup
    const int width = 1280;
    const int height = 720;
    sf::RenderWindow window(sf::VideoMode(width, height), "C++23 Ray Marching");
    window.setFramerateLimit(60);

    // Setup camera
    rm::Camera camera(45.0f, static_cast<float>(width) / height);
    camera.setPosition(rm::Vec3(0.0f, 2.0f, 10.0f));
    camera.setTarget(rm::Vec3(0.0f, 0.0f, 0.0f));

    // Create a scene
    rm::Scene scene;

    // Add a ground plane
    auto ground = std::make_shared<rm::Plane>(rm::Vec3(0.0f, 1.0f, 0.0f), 1.0f);
    ground->setMaterial(rm::Material(rm::Vec3(0.4f, 0.4f, 0.4f), 0.1f, 0.9f));
    scene.add(ground);

    // Create a row of pillars
    for (int i = -4; i <= 4; i += 2) {
        auto pillar = std::make_shared<rm::Cylinder>(rm::Vec3(i, 0.0f, -5.0f), 0.5f, 3.0f);
        pillar->setMaterial(rm::Material(rm::Vec3(0.7f, 0.7f, 0.7f), 0.2f, 0.5f));
        scene.add(pillar);

        // Add a sphere on top of each pillar
        auto sphere = std::make_shared<rm::Sphere>(rm::Vec3(i, 2.0f, -5.0f), 0.6f);

        // Alternate colors - more vibrant with emissive properties
        if (i % 4 == 0) {
            sphere->setMaterial(rm::Material(rm::Vec3(0.9f, 0.2f, 0.2f), 0.9f, 0.05f, 0.1f));
        } else {
            sphere->setMaterial(rm::Material(rm::Vec3(0.2f, 0.2f, 0.9f), 0.9f, 0.05f, 0.1f));
        }

        scene.add(sphere);
    }

    // Create some tori
    auto torus1 = std::make_shared<rm::Torus>(rm::Vec3(-3.0f, 0.5f, 0.0f), 1.0f, 0.25f);
    torus1->setMaterial(rm::Material(rm::Vec3(0.9f, 0.5f, 0.2f), 0.7f, 0.1f));
    scene.add(torus1);

    auto torus2 = std::make_shared<rm::Torus>(rm::Vec3(3.0f, 0.5f, 0.0f), 1.0f, 0.25f);
    torus2->setMaterial(rm::Material(rm::Vec3(0.2f, 0.9f, 0.5f), 0.7f, 0.1f));
    scene.add(torus2);

    // Create a central structure
    auto centralBox = std::make_shared<rm::Box>(rm::Vec3(0.0f, 1.0f, 0.0f), rm::Vec3(2.0f, 2.0f, 2.0f));
    centralBox->setMaterial(rm::Material(rm::Vec3(0.3f, 0.3f, 0.3f), 0.8f, 0.05f));

    auto centralSphere = std::make_shared<rm::Sphere>(rm::Vec3(0.0f, 1.0f, 0.0f), 1.4f);
    centralSphere->setMaterial(rm::Material(rm::Vec3(0.95f, 0.9f, 0.1f), 0.9f, 0.05f, 0.15f));

    auto centralCSG = std::make_shared<rm::Intersection>(centralBox, centralSphere);
    scene.add(centralCSG);

    // Add dramatic light setup for darker atmosphere
    scene.setAmbientLight(rm::Vec3(0.02f, 0.02f, 0.04f)); // Very dim bluish ambient

    // Main directional light - warm but less intense
    scene.addLight(rm::Vec3(15.0f, 12.0f, 10.0f), rm::Vec3(1.0f, 0.85f, 0.7f), 1.8f);

    // Cold rim light
    scene.addLight(rm::Vec3(-12.0f, 8.0f, 5.0f), rm::Vec3(0.4f, 0.4f, 1.0f), 1.0f);

    // Dramatic red highlight
    scene.addLight(rm::Vec3(0.0f, 3.0f, -15.0f), rm::Vec3(0.9f, 0.2f, 0.2f), 0.8f);

    // Create renderer
    rm::Renderer renderer(width, height);
    renderer.setExposure(1.8f); // Increased exposure to balance the darker scene
    renderer.setSamplesPerPixel(1);  // Low for interactive performance

    // Set darker sky and ground colors
    renderer.setSkyColors(
        rm::Vec3(0.2f, 0.2f, 0.3f),  // Horizon (dark blue-gray)
        rm::Vec3(0.05f, 0.1f, 0.2f)   // Zenith (very deep blue)
    );

    renderer.setGroundColors(
        rm::Vec3(0.2f, 0.2f, 0.15f), // Horizon (dark ground)
        rm::Vec3(0.05f, 0.05f, 0.02f)  // Nadir (nearly black)
    );

    // Camera control variables
    bool autoCamera = true;
    float time = 0.0f;
    bool needsRender = true;

    // Manual camera control
    float cameraSpeed = 0.2f;
    float rotationSpeed = 0.05f;
    rm::Vec3 cameraPos = camera.getPosition();
    rm::Vec3 cameraTarget = rm::Vec3(0.0f, 0.5f, 0.0f);

    // For FPS calculation
    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;

    // SFML sprites and textures for display
    sf::Sprite renderSprite;

    // Load font for UI text
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        // Fallback if default font not found
        std::cout << "Warning: Could not load font. UI text will not be displayed." << std::endl;
    }

    std::cout << "Starting ray marching renderer..." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Arrow Keys - Rotate camera" << std::endl;
    std::cout << "  Q/E - Move up/down" << std::endl;
    std::cout << "  Space - Toggle auto camera" << std::endl;
    std::cout << "  R - Increase samples per pixel" << std::endl;
    std::cout << "  F - Decrease samples per pixel" << std::endl;
    std::cout << "  Esc - Exit" << std::endl;

    // Main loop
    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                else if (event.key.code == sf::Keyboard::Space) {
                    autoCamera = !autoCamera;
                    needsRender = true;
                }
                else if (event.key.code == sf::Keyboard::R) {
                    int samples = std::min(renderer.getSamplesPerPixel() * 2, 16);
                    renderer.setSamplesPerPixel(samples);
                    needsRender = true;
                    std::cout << std::format("Samples per pixel: {}\n", samples);
                }
                else if (event.key.code == sf::Keyboard::F) {
                    int samples = std::max(renderer.getSamplesPerPixel() / 2, 1);
                    renderer.setSamplesPerPixel(samples);
                    needsRender = true;
                    std::cout << std::format("Samples per pixel: {}\n", samples);
                }
            }
            else if (event.type == sf::Event::Resized) {
                // Adjust viewport
                sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
                camera.setAspectRatio(static_cast<float>(event.size.width) / event.size.height);
                needsRender = true;
            }
        }

        if (autoCamera) {
            // Automatic camera movement in a more interesting pattern
            time += 0.01f;

            float radius = 15.0f; // Wider orbit
            float camX = radius * std::sin(time * 0.2f);
            float camZ = radius * std::cos(time * 0.2f);
            float camY = 3.5f + std::sin(time * 0.3f) * 2.0f; // More dramatic height changes

            cameraPos = rm::Vec3(camX, camY, camZ);

            // Look at a point that moves slightly
            float targetX = std::sin(time * 0.15f) * 3.0f;
            float targetZ = std::cos(time * 0.15f) * 3.0f;
            cameraTarget = rm::Vec3(targetX, 0.5f + std::sin(time * 0.4f) * 0.5f, targetZ);
        }
        else {
            // Manual camera controls
            rm::Vec3 forward = (cameraTarget - cameraPos).normalize();
            rm::Vec3 right = rm::Vec3(0.0f, 1.0f, 0.0f).cross(forward).normalize();
            rm::Vec3 up = forward.cross(right).normalize();

            // Check keyboard for camera movement
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                cameraPos = cameraPos + forward * cameraSpeed;
                cameraTarget = cameraTarget + forward * cameraSpeed;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                cameraPos = cameraPos - forward * cameraSpeed;
                cameraTarget = cameraTarget - forward * cameraSpeed;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                cameraPos = cameraPos - right * cameraSpeed;
                cameraTarget = cameraTarget - right * cameraSpeed;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                cameraPos = cameraPos + right * cameraSpeed;
                cameraTarget = cameraTarget + right * cameraSpeed;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                cameraPos = cameraPos + up * cameraSpeed;
                cameraTarget = cameraTarget + up * cameraSpeed;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                cameraPos = cameraPos - up * cameraSpeed;
                cameraTarget = cameraTarget - up * cameraSpeed;
                needsRender = true;
            }

            // Camera rotation with arrow keys
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                // Rotate left around up vector
                rm::Vec3 direction = cameraTarget - cameraPos;
                float length = direction.length();

                direction = direction - right * rotationSpeed;
                direction = direction.normalize() * length;

                cameraTarget = cameraPos + direction;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                // Rotate right around up vector
                rm::Vec3 direction = cameraTarget - cameraPos;
                float length = direction.length();

                direction = direction + right * rotationSpeed;
                direction = direction.normalize() * length;

                cameraTarget = cameraPos + direction;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                // Rotate up around right vector
                rm::Vec3 direction = cameraTarget - cameraPos;
                float length = direction.length();

                direction = direction + up * rotationSpeed;
                direction = direction.normalize() * length;

                cameraTarget = cameraPos + direction;
                needsRender = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                // Rotate down around right vector
                rm::Vec3 direction = cameraTarget - cameraPos;
                float length = direction.length();

                direction = direction - up * rotationSpeed;
                direction = direction.normalize() * length;

                cameraTarget = cameraPos + direction;
                needsRender = true;
            }
        }

        // Update camera position and target
        camera.setPosition(cameraPos);
        camera.setTarget(cameraTarget);

        // Render if needed
        if (needsRender) {
            auto startRender = std::chrono::high_resolution_clock::now();

            renderer.render(scene, camera);
            renderSprite.setTexture(renderer.getTexture());

            auto endRender = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> renderTime = endRender - startRender;
            std::cout << std::format("Render time: {:.2f}ms\n", renderTime.count() * 1000.0);

            needsRender = false;
        }

        // Clear and draw
        window.clear(sf::Color::Black);
        window.draw(renderSprite);

        // Draw UI text if font was loaded
        if (font.getInfo().family != "") {
            std::string cameraMode = autoCamera ? "Auto Camera: ON" : "Manual Camera: ON";
            std::string controlsText = "WASD - Move  |  Arrow Keys - Look  |  Space - Toggle Camera";
            std::string qualityText = std::format("Samples: {}  |  Press R/F to adjust quality", renderer.getSamplesPerPixel());

            drawText(window, cameraMode, sf::Vector2f(10, 10), font, 18, sf::Color(255, 255, 255, 200));
            drawText(window, controlsText, sf::Vector2f(10, height - 50), font, 16, sf::Color(255, 255, 255, 180));
            drawText(window, qualityText, sf::Vector2f(10, height - 25), font, 16, sf::Color(255, 255, 255, 180));
        }

        window.display();

        // FPS calculation
        frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;

        if (elapsed.count() >= 1.0) {
            double fps = frameCount / elapsed.count();
            frameCount = 0;
            lastTime = currentTime;

            window.setTitle(std::format("C++23 Ray Marching - FPS: {:.1f}", fps));
        }
    }

    return 0;
}