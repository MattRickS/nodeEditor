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

enum Layer
{
    LAYER_HEIGHTMAP
};

class Texture
{
protected:
    void LoadOnGPU()
    {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // If the data's on the GPU it's likely being modified so do not keep a local
        // copy of the old data. Retrieve the data from the GPU if requested.
        DeleteLocalData();
    }
    void DeleteLocalData()
    {
        if (data)
        {
            delete[] data;
        }
    }

public:
    unsigned int width, height;
    GLenum format = GL_RGBA;
    float *data = 0;
    GLuint ID = 0;

    Texture(unsigned int width, unsigned int height, GLenum format = GL_RGBA, float *data = 0) : width(width), height(height), format(format), data(data) {}
    ~Texture()
    {
        if (ID)
        {
            glDeleteTextures(1, &ID);
        }
    }
    // TODO: Texture(const char* path) {} stbi load

    void Resize(unsigned int width, unsigned int height)
    {
        this->width = width;
        this->height = height;
        glBindTexture(GL_TEXTURE_2D, ID);
        // TODO: Should probably just restructure the data so it's in the same pixel
        // positions, but either truncate or pad with black. For purposes of this tool,
        // deleting it should be fine.
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, 0);
        DeleteLocalData();
    }
    bool IsOnGPU() { return ID != 0; }
    void EnsureOnGPU()
    {
        if (!IsOnGPU())
        {
            glGenTextures(1, &ID);
            LoadOnGPU();
        }
    }

    // TODO: Read/Write pixel methods, unload from GPU. This is to support CPU operators.
};

class RenderSet
{
protected:
    std::map<Layer, Texture *> layers;

public:
    RenderSet() {}

    void Reset() { layers.clear(); }
    void AddLayer(Layer layer, Texture *texture)
    {
        layers[layer] = texture;
    }
    // bool HasLayer(Layer layer) {}
    Texture *GetLayer(Layer layer) const
    {
        return layers.at(layer);
    }
};

class Operator
{
protected:
    std::vector<Texture> outputs;
    unsigned int m_width, m_height;
    GLuint FBO;

public:
    // TODO: Should have a virtual destructor so that it can be stored as the base class
    void init(unsigned int width, unsigned int height)
    {
        m_width = width;
        m_height = height;

        // Generate an output texture of the right size for each
        auto layers = outLayers();
        outputs = std::vector<Texture>(layers.size(), {m_width, m_height});

        // TODO: FBO/GPU setup is only required for shader operators, move to subclass
        for (Texture &texture : outputs)
        {
            texture.EnsureOnGPU();
        }

        // Generate the FBO with textures bound in order
        glGenFramebuffers(1, &FBO);
        // TODO: Binding only the draw buffer here causes it to fail. More research into this required.
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        std::vector<GLenum> drawBuffers(outputs.size());
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, outputs[i].ID, 0);
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        glDrawBuffers(drawBuffers.size(), drawBuffers.data());
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Failed to generate texture buffer" << std::endl;
        }
    }
    void resize(unsigned int width, unsigned int height)
    {
        m_width = width;
        m_height = height;
        // Resize each output texture
        for (Texture &tex : outputs)
        {
            tex.Resize(width, height);
        }
    }

    virtual std::vector<Layer> inLayers() const = 0;
    virtual std::vector<Layer> outLayers() const = 0;
    virtual void process(RenderSet *renders) = 0;

    /*
    Convenience method for populating the renderset that assumes texture outputs
    are 1:1 with the outLayers.
    */
    virtual void PopulateRenderSet(RenderSet *renderSet)
    {
        auto layers = outLayers();
        for (size_t i = 0; i < layers.size(); ++i)
        {
            renderSet->AddLayer(layers[i], &outputs[i]);
        }
    }
};

class PerlinNoiseShader : public Shader
{
protected:
    float m_frequency = 0.01f;
    glm::ivec2 m_offset = glm::ivec2(0);

public:
    PerlinNoiseShader() : Shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/noise/perlin.fs") {}

    float Frequency() { return m_frequency; }
    void SetFrequency(float frequency)
    {
        use();
        m_frequency = frequency;
        setFloat("frequency", m_frequency);
    }

    glm::ivec2 Offset() { return m_offset; }
    void SetOffset(glm::ivec2 offset)
    {
        use();
        m_offset = offset;
        setInt2("offset", m_offset.x, m_offset.y);
    }
};

class PerlinNoiseOperator : public Operator
{
public:
    PerlinNoiseShader shader;

    PerlinNoiseOperator() : shader() {}

    virtual std::vector<Layer> inLayers() const
    {
        return {};
    }
    virtual std::vector<Layer> outLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual void process(RenderSet *renders)
    {
        shader.use();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glViewport(0, 0, m_width, m_height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        PopulateRenderSet(renders);
    }
};

class InvertOperator : public Operator
{
public:
    Shader shader;

    InvertOperator() : shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/invert.fs")
    {
    }
    virtual std::vector<Layer> inLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual std::vector<Layer> outLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual void process(RenderSet *renders)
    {
        // Setup shader
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renders->GetLayer(LAYER_HEIGHTMAP)->ID);
        shader.setInt("inImage", 0);

        // Render
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glViewport(0, 0, m_width, m_height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Put outputs into render set
        PopulateRenderSet(renders);
    }
};

class PerlinNoiseUI
{
public:
    void Draw(PerlinNoiseShader *noise)
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

        // TODO: transform the position to get the pixel pos on the map
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
    void Draw(const RenderSet *const renderSet)
    {
        // Draws the texture into the window slot
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glm::ivec4 mapRegion = GetMapViewportRegion();
        glViewport(mapRegion.x, mapRegion.y, mapRegion.z, mapRegion.w);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderSet->GetLayer(LAYER_HEIGHTMAP)->ID);
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

class MapMaker
{
protected:
    UI *m_ui;
    PixelPreview m_pixelPreview;
    std::vector<Operator *> operators;
    unsigned int m_width = 1280;
    unsigned int m_height = 720;

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

        // MapMaker owns no textures, each operator owns the textures it generates.
        // MapMaker instead owns a running mapping of Layer: Texture* as a RenderSet
        // that it passes as input to each operator. If rewinding to a previous operator,
        // this must be reset and re-populated by each prior operator in sequence.
        RenderSet renderSet;

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
    UI ui = UI(1280, 720, "MapMaker");
    if (!ui.IsInitialised())
        return 1;

    makeQuad();
    MapMaker app = MapMaker(&ui);
    app.Exec();

    glfwTerminate();
    return 0;
}