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
const float CAM_NEAR = 0.1f;
const float CAM_FAR = 100.0f;

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

    bool isPanning = false;
    glm::vec2 lastCursorPos;

    void OnMouseButtonChanged(int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                lastCursorPos = m_ui->CursorPos();
                isPanning = true;
            }
            else if (action == GLFW_RELEASE)
            {
                isPanning = false;
            }
        }
    }

    void OnMouseMoved(double xpos, double ypos)
    {
        // TODO: transform the position to get the pixel pos on the map
        glm::ivec4 viewportRegion = m_ui->GetViewportRegion();
        // TODO: These are entirely wrong
        m_pixelPreview.pos = {xpos - viewportRegion.x, ypos - viewportRegion.y};
        if (m_pixelPreview.pos.x >= 0)
        {
            // TODO: Which framebuffer? Might be simpler once threaded
            glReadPixels(m_pixelPreview.pos.x, m_pixelPreview.pos.y, 1, 1, GL_RGBA, GL_FLOAT, &m_pixelPreview.value);
        }

        if (isPanning)
        {
            glm::vec2 cursorPos = glm::vec2(xpos, ypos);
            // TODO: Get a 1:1 translation
            // glm::vec2 offset = m_ui->ScreenToMapPos(cursorPos) - m_ui->ScreenToMapPos(lastCursorPos);
            glm::vec2 offset = 2.0f * (cursorPos - lastCursorPos);
            offset = glm::vec2(offset.x / m_ui->Width(), offset.y / m_ui->Height());
            m_ui->camera.view = glm::translate(m_ui->camera.view, glm::vec3(offset, 0.0f));
            lastCursorPos = cursorPos;
        }
    }

    void OnMouseScrolled(double xoffset, double yoffset)
    {
        m_ui->camera.focal *= (1.0f - yoffset * 0.1f);
        UpdateProjection();
    }

    void OnKeyChanged(int key, int scancode, int action, int mode)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            Close();
    }

    void OnResize(int width, int height)
    {
        UpdateProjection();
    }

    void UpdateProjection()
    {
        glm::ivec4 viewportRegion = m_ui->GetViewportRegion();
        float hAperture = (float)viewportRegion.z / (float)viewportRegion.w;
        static float vAperture = 1.0f;
        m_ui->camera.projection = glm::ortho(-hAperture * m_ui->camera.focal,
                                             hAperture * m_ui->camera.focal,
                                             -vAperture * m_ui->camera.focal,
                                             vAperture * m_ui->camera.focal,
                                             CAM_NEAR,
                                             CAM_FAR);
    }

public:
    Application(MapMaker *mapmaker, UI *ui) : m_mapmaker(mapmaker), m_ui(ui)
    {
        // TODO: I'm being too lazy to work out the actual matrix for the definition
        m_ui->camera.view = glm::translate(m_ui->camera.view, glm::vec3(0, 0, -1));
        UpdateProjection();

        m_ui->cursorMoved.connect(this, &Application::OnMouseMoved);
        m_ui->mouseButtonChanged.connect(this, &Application::OnMouseButtonChanged);
        m_ui->mouseScrolled.connect(this, &Application::OnMouseScrolled);
        m_ui->closeRequested.connect(this, &Application::Close);
        m_ui->sizeChanged.connect(this, &Application::OnResize);
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