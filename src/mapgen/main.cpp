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

#include "shader.h"

int main()
{
    glm::vec3 empty(0);
    std::cout << "Hello, World!" << std::endl;

    sf::RenderWindow window(sf::VideoMode(1280, 720), "ImGui + SFML = <3");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
    glewInit();

    Shader shader("src/mapgen/shaders/quad.vs", "src/mapgen/shaders/example.fs");

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

        ImGui::ShowDemoWindow();

        window.clear();
        shader.use();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}