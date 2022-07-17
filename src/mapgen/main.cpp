#define GLEW_STATIC
#include <iostream>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "interface/ui.h"
#include "mapmaker.h"
#include "operator.h"
#include "operators/perlin.h"
#include "operators/invert.hpp"
#include "renders.h"
#include "shader.h"

GLuint quadVAO;

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void makeQuad()
{
    static const GLfloat vertexData[] = {
        // positions          // texture coords
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    GLuint quadEBO;
    glGenBuffers(1, &quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// class PerlinNoiseUI
// {
// public:
//     void Draw(PerlinNoiseShader *noise)
//     {
//         if (!noise)
//             return;

//         float freq = noise->Frequency();
//         if (ImGui::SliderFloat("Frequency", &freq, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
//             noise->SetFrequency(freq);

//         glm::ivec2 offset = noise->Offset();
//         if (ImGui::DragInt2("Offset", (int *)&offset))
//             noise->SetOffset(offset);
//     }
// };

class Application
{
protected:
    UI *m_ui;
    MapMaker *m_mapmaker;
    PixelPreview m_pixelPreview;

    void OnMouseMoved(unsigned int xpos, unsigned int ypos)
    {
        // TODO: Which framebuffer? Might be simpler once threaded
        glReadPixels(xpos, ypos, 1, 1, GL_RGBA, GL_FLOAT, &m_pixelPreview.value);
        m_pixelPreview.pos = glm::ivec2(xpos, ypos);
    }

    void OnKeyChanged(int key, int scancode, int action, int mode)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            Close();
    }

public:
    Application(MapMaker *mapmaker, UI *ui) : m_mapmaker(mapmaker), m_ui(ui)
    {
        m_ui->mapPosChanged.connect(this, &Application::OnMouseMoved);
        m_ui->closeRequested.connect(this, &Application::Close);
        m_ui->keyChanged.connect(this, &Application::OnKeyChanged);
        m_ui->SetPixelPreview(&m_pixelPreview);
    }
    void Exec()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        m_mapmaker->ProcessAll();

        while (!m_ui->IsClosed())
        {
            glfwPollEvents();
            m_ui->Draw(m_mapmaker->GetRenderSet());
            m_ui->Display();
        }
    }
    void Close()
    {
        m_ui->Close();
    }
};

int main()
{
    // GLFW init
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    unsigned int width = 1280;
    unsigned int height = 720;
    // TODO: context and GLEW still need initialising without the UI... right?
    UI ui = UI(width, height);
    if (!ui.IsInitialised())
        return 1;

    makeQuad();
    MapMaker mapmaker(width, height);
    Application app = Application(&mapmaker, &ui);
    app.Exec();

    // XXX: Seeing output "Glfw Error 65537: The GLFW library is not initialized"
    // Think this fails because the window is already destroyed, non-critical
    glfwTerminate();
    return 0;
}