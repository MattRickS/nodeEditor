#define GLEW_STATIC
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "shader.h"
#include "window.h"

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

class PerlinNoise
{
protected:
    Shader m_shader;
    float m_frequency = 0.01f;
    glm::ivec2 m_offset = glm::ivec2(0);

public:
    PerlinNoise() : m_shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/noise/perlin.fs") {}

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
    Shader viewShader;
    float m_uiScreenWidthPercent = 0.25f;
    PixelPreview *m_pixelPreview;
    PerlinNoiseUI m_perlinUI;
    bool isPanning = false;
    glm::vec2 lastCursorPos;

    glm::mat4 viewTransform = glm::mat4(1.0f);
    glm::vec2 transformOffset = glm::vec2(0.0f);
    float transformZoom = 1.0f;

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

        if (isPanning)
        {
            glm::vec2 cursorPos = CursorPos();
            glm::vec2 offset = cursorPos - lastCursorPos;
            // Invert y-axis as openGL is inverse
            transformOffset += glm::vec2(offset.x / m_width, -offset.y / m_height);
            lastCursorPos = cursorPos;
            updateTransform();
        }
    }

    void updateTransform()
    {
        viewTransform = glm::mat4(1.0f);
        viewTransform = glm::translate(viewTransform, glm::vec3(transformOffset, 0.0f));
        viewTransform = glm::scale(viewTransform, glm::vec3(transformZoom, transformZoom, 1.0f));
        viewShader.use();
        viewShader.setMat4("transform", viewTransform);
    }

    virtual void OnMouseButtonChanged(int button, int action, int mods)
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;

        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                lastCursorPos = CursorPos();
                isPanning = true;
            }
            else if (action == GLFW_RELEASE)
            {
                isPanning = false;
            }
        }
    }

    virtual void OnMouseScrolled(double xoffset, double yoffset)
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;

        transformZoom += yoffset * 0.1f;
        updateTransform();
    }

public:
    Signal<unsigned int, unsigned int> mapPosChanged;

    UI(unsigned int width, unsigned int height, const char *name) : Window(width, height, name),
                                                                    viewShader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/texture.fs")
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void SetPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
    void Draw(GLuint renderTexture)
    {
        // Draws the texture into the window slot
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glm::ivec4 mapRegion = GetMapViewportRegion();
        glViewport(mapRegion.x, mapRegion.y, mapRegion.z, mapRegion.w);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glClearColor(1.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        viewShader.use();
        viewShader.setMat4("transform", viewTransform);
        viewShader.setInt("renderTexture", 0); // Why 0 and not renderTexture?
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draws the UI around it
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

GLuint makeTexture(unsigned int width, unsigned int height)
{
    GLuint renderTexture;
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    return renderTexture;
}

GLuint makeBuffer(GLuint textureID)
{
    // Generating a texture as a render target. Each operator will need to
    // do this for as many layers as it defines.
    GLuint MapTextureBuffer;
    glGenFramebuffers(1, &MapTextureBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, MapTextureBuffer);

    // Bind the texture to a buffer. Increment the COLOR_ATTACHMENT suffix
    // for multiple textures (and set the number of DrawBuffers)
    glBindFramebuffer(GL_FRAMEBUFFER, MapTextureBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureID, 0);
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Failed to generate texture buffer" << std::endl;
        return 0;
    }
    return MapTextureBuffer;
}

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
        GLuint renderTexture = makeTexture(m_width, m_height);
        GLuint bufferID = makeBuffer(renderTexture);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        Shader noise("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/noise/perlin.fs");
        Shader viewShader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/texture.fs");
        glm::mat4 identity(1.0f);

        noise.use();
        noise.setMat4("transform", identity);
        glViewport(0, 0, m_width, m_height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        while (!m_ui->IsClosed())
        {
            glfwPollEvents();

            // Draw and display the buffer
            m_ui->Draw(renderTexture);
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

    makeQuad();
    MapMaker app = MapMaker(&ui);
    app.Exec();

    glfwTerminate();
    return 0;
}