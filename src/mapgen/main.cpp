#define GLEW_STATIC
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "quadshader.h"
#include "window.h"

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

class PerlinNoise
{
protected:
    QuadShader m_shader;
    float m_frequency = 0.01f;
    glm::ivec2 m_offset = glm::ivec2(0);

public:
    PerlinNoise() : m_shader("src/mapgen/shaders/noise/perlin.fs") {}

    float Frequency() { return m_frequency; }
    void SetFrequency(float frequency)
    {
        m_shader.use();
        m_frequency = frequency;
        m_shader.setFloat("frequency", m_frequency);
    }

    glm::ivec2 Offset() { return m_offset; }
    void SetOffset(glm::ivec2 offset)
    {
        m_shader.use();
        m_offset = offset;
        m_shader.setInt2("offset", m_offset.x, m_offset.y);
    }

    void Draw()
    {
        m_shader.draw();
    }
};

class PerlinNoiseUI
{
public:
    void Draw(PerlinNoise *noise)
    {
        if (!noise)
            return;

        float freq = noise->Frequency();
        if (ImGui::SliderFloat("Frequency", &freq, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
            noise->SetFrequency(freq);

        glm::ivec2 offset = noise->Offset();
        if (ImGui::DragInt2("Offset", (int *)&offset))
            noise->SetOffset(offset);
    }
};

struct PixelPreview
{
    glm::vec4 value = glm::vec4(0);
    glm::ivec2 pos = glm::ivec2(0);
};

/*
Class for handling User Interface. Responsible for layout and drawing the
interactive widgets but not for rendering the map.
*/
class UI : public Window
{
protected:
    float m_uiScreenWidthPercent = 0.25f;
    PixelPreview *m_pixelPreview;
    PerlinNoiseUI m_perlinUI;

    unsigned int uiWidth() { return m_width * m_uiScreenWidthPercent; }

    // Only emits the signal if the UI didn't capture it
    virtual void OnMouseMoved(double xpos, double ypos)
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;

        int imagePosX = xpos - uiWidth();
        if (imagePosX >= 0)
        {
            // glfw measures y from top-left, image positions are referenced from
            // bot-left, invert the y-axis
            mapPosChanged.emit(imagePosX, m_height - ypos);
        }
    }

public:
    Signal<unsigned int, unsigned int> mapPosChanged;

    UI(unsigned int width, unsigned int height, const char *name) : Window(width, height, name)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void SetPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
    void Draw()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool p_open = NULL;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(uiWidth(), m_height));
        ImGui::Begin("Mapmaker UI", &p_open, flags);

        if (m_pixelPreview)
        {
            ImGui::BeginDisabled();
            ImGui::ColorEdit4("Pixel Value", (float *)&(*m_pixelPreview).value);
            ImGui::InputInt2("Pixel Position", (int *)&(*m_pixelPreview).pos);
            ImGui::EndDisabled();
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    glm::ivec4 GetMapViewportRegion()
    {
        return glm::ivec4(uiWidth(), 0, m_width - uiWidth(), m_height);
    }
};

class MapMaker
{
protected:
    UI *m_ui;
    PixelPreview m_pixelPreview;
    // This will eventually be a more structured series of operations to produce
    // a final image, with multiple intermediate images.
    PerlinNoise m_noise;
    unsigned int m_width = 1280;
    unsigned int m_height = 720;

    void OnMouseMoved(unsigned int xpos, unsigned int ypos)
    {
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
        m_ui->mapPosChanged.connect(this, &MapMaker::OnMouseMoved);
        m_ui->closeRequested.connect(this, &MapMaker::Close);
        m_ui->keyChanged.connect(this, &MapMaker::OnKeyChanged);
        m_ui->SetPixelPreview(&m_pixelPreview);
    }
    void Exec()
    {
        // Generating a texture as a render target. Each operator will need to
        // do this for as many layers as it defines.
        GLuint MapTextureBuffer = 0;
        glGenFramebuffers(1, &MapTextureBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, MapTextureBuffer);
        GLuint renderTexture;
        glGenTextures(1, &renderTexture);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // Bind the texture to a buffer. Increment the COLOR_ATTACHMENT suffix
        // for multiple textures (and set the number of DrawBuffers)
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);
        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Failed to generate texture buffer" << std::endl;
            return;
        }

        QuadShader display("src/mapgen/shaders/tex.fs");

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        while (!m_ui->IsClosed())
        {
            glfwPollEvents();

            // Render to texture
            glBindFramebuffer(GL_FRAMEBUFFER, MapTextureBuffer);
            glViewport(0, 0, m_width, m_height);
            m_noise.Draw();

            // Render texture into window section
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glm::ivec4 mapRegion = m_ui->GetMapViewportRegion();
            glViewport(mapRegion.x, mapRegion.y, mapRegion.z, mapRegion.w);
            display.setInt("renderTexture", renderTexture);
            display.draw();

            // Draw the UI around it
            m_ui->Draw();

            // Display the buffer that was drawn
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

    UI ui = UI(1280, 720, "MapMaker");
    if (!ui.IsInitialised())
        return 1;

    MapMaker app = MapMaker(&ui);
    app.Exec();

    glfwTerminate();
    return 0;
}