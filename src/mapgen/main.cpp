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

class MapMaker
{
protected:
    UI *m_ui;
    PixelPreview m_pixelPreview;
    std::vector<Operator *> operators;
    unsigned int m_width = 1280;
    unsigned int m_height = 720;

    // MapMaker owns no textures, each operator owns the textures it generates.
    // MapMaker instead owns a running mapping of Layer: Texture* as a RenderSet
    // that it passes as input to each operator. If rewinding to a previous operator,
    // this must be reset and re-populated by each prior operator in sequence.
    RenderSet renderSet;

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
    MapMaker(UI *ui) : m_ui(ui)
    {
        // TODO: Switch to smart pointers
        operators.push_back(new PerlinNoiseOperator);
        operators.push_back(new InvertOperator);
        for (auto op : operators)
        {
            op->init(m_width, m_height);
        }
        m_ui->mapPosChanged.connect(this, &MapMaker::OnMouseMoved);
        m_ui->closeRequested.connect(this, &MapMaker::Close);
        m_ui->keyChanged.connect(this, &MapMaker::OnKeyChanged);
        m_ui->SetPixelPreview(&m_pixelPreview);
    }
    ~MapMaker()
    {
        for (size_t i = operators.size() - 1; i >= 0; --i)
        {
            delete operators[i];
        }
    }
    void Exec()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // Run the first operator on the renderSet
        for (auto op : operators)
        {
            op->process(&renderSet);
        }

        while (!m_ui->IsClosed())
        {
            glfwPollEvents();

            // Draw and display the buffer
            m_ui->Draw(&renderSet);
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

    // TODO: context and GLEW still need initialising without the UI... right?
    UI ui = UI();
    if (!ui.IsInitialised())
        return 1;

    makeQuad();
    MapMaker app = MapMaker(&ui);
    app.Exec();

    // XXX: Seeing output "Glfw Error 65537: The GLFW library is not initialized"
    // Think this fails because the window is already destroyed, non-critical
    glfwTerminate();
    return 0;
}