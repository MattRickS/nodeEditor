#define GLEW_STATIC
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/OpenGL.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include "quadshader.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "MapGeneration");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Failed to initialise glew: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    QuadShader shader("src/mapgen/shaders/noise/perlin.fs");

    float noiseFrequency = 0.01;

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Mapmaker UI");
        if (ImGui::SliderFloat("Frequency", &noiseFrequency, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
            shader.setFloat("frequency", noiseFrequency);
        ImGui::End();

        window.clear();
        shader.draw();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}