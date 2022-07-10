#define GLEW_STATIC
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "quadshader.h"

int width = 1280;
int height = 720;
int uiWidth;
glm::vec4 pixelValue = glm::vec4(0);
glm::ivec2 pixelPos = glm::ivec2(0);

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    int imagePos = xpos - uiWidth;
    if (imagePos >= 0)
    {
        glReadPixels(xpos, ypos, 1, 1, GL_RGBA, GL_FLOAT, &pixelValue);
        pixelPos = glm::ivec2(imagePos, height - ypos);
    }
    else
    {
        pixelPos = glm::ivec2(0);
        pixelValue = glm::vec4(0);
    }
}

int main()
{
    // GLFW init
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    GLFWwindow *window = glfwCreateWindow(width, height, "MapMaker", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // GLEW init
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Failed to initialise glew: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    // Must set before initialising ImGui
    glfwSetCursorPosCallback(window, mouse_callback);

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //  Setup
    QuadShader shader("src/mapgen/shaders/noise/perlin.fs");
    float noiseFrequency = 0.01;
    glm::ivec2 offset = glm::ivec2(0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwGetWindowSize(window, &width, &height);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool p_open = NULL;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        // Draw shader
        uiWidth = width * 0.25;
        glViewport(uiWidth, 0, width - uiWidth, height);
        shader.setFloat("frequency", noiseFrequency);
        shader.draw();

        // Draw UI
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(uiWidth, height));
        ImGui::Begin("Mapmaker UI", &p_open, flags);
        if (ImGui::SliderFloat("Frequency", &noiseFrequency, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
            shader.setFloat("frequency", noiseFrequency);
        if (ImGui::DragInt2("Offset", (int *)&offset))
            shader.setInt2("offset", offset.x, offset.y);

        ImGui::BeginDisabled();
        ImGui::ColorEdit4("Pixel Value", (float *)&pixelValue);
        ImGui::InputInt2("Pixel Position", (int *)&pixelPos);
        ImGui::EndDisabled();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}