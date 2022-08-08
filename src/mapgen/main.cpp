#define GLEW_STATIC
#include <iostream>
#include <map>
#include <string>
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
#include "renders.h"
#include "shader.h"
#include "util.hpp"

// TODO: will likely have to move this onto the context/owning class and bind each time the context is switched
GLuint quadVAO_UI;
const float CAM_NEAR = 0.1f;
const float CAM_FAR = 100.0f;

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

class Application
{
protected:
    MapMaker *m_mapmaker;
    UI *m_ui;
    PixelPreview m_pixelPreview;

    bool isPanning = false;
    glm::vec2 lastCursorPos;
    float *buffer;

    // FPS limiting as vsync does not appear to be working
    double lastFrameTime = 0;
    double fpsLimit = 1.0 / 60.0;

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

    const Texture *currentTexture() const
    {
        auto renderSet = m_mapmaker->GetRenderSet();
        Layer layer = m_ui->GetCurrentLayer();
        auto it = renderSet->find(layer);
        if (it == renderSet->end())
        {
            return nullptr;
        }
        return it->second;
    }

    void UpdatePixelPreview(double xpos, double ypos)
    {
        glm::vec2 worldPos = m_ui->ScreenToWorldPos({xpos, ypos});
        if (worldPos.x >= 0 && worldPos.x < 1 && worldPos.y >= 0 && worldPos.y < 1)
        {
            int x = worldPos.x * m_mapmaker->Width();
            int y = worldPos.y * m_mapmaker->Height();
            m_pixelPreview.pos = {x, y};
            // TODO: Only reads from buffer. Current framebuffer only has 0-1 values.
            //       Could mount the texture to a storage buffer, but not sure if that
            //       will improve retrieved values/performance
            // glReadPixels(xpos, ypos, 1, 1, GL_RGBA, GL_FLOAT, &m_pixelPreview.value);

            // TODO: If keeping this method, only read the texture data once when
            // - requested
            // - active texture has changed / was processed further
            glActiveTexture(GL_TEXTURE0);
            const Texture *texptr = currentTexture();
            if (texptr)
            {
                glBindTexture(GL_TEXTURE_2D, texptr->ID);
                glGetTexImage(GL_TEXTURE_2D, 0, texptr->format, GL_FLOAT, buffer);
                int index = (y * m_mapmaker->Width() + x) * texptr->numChannels();
                for (int i = 0; i < 4; ++i)
                    m_pixelPreview.value[i] = i < texptr->numChannels() ? buffer[index + i] : 0.0f;
            }
        }
    }

    void OnMouseMoved(double xpos, double ypos)
    {
        UpdatePixelPreview(xpos, ypos);

        if (isPanning)
        {
            glm::vec2 cursorPos = glm::vec2(xpos, ypos);
            glm::vec2 offset = m_ui->ScreenToWorldPos(cursorPos) - m_ui->ScreenToWorldPos(lastCursorPos);
            m_ui->camera.view = glm::translate(m_ui->camera.view, glm::vec3(2.0f * offset, 0.0f));
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
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                Close();
                break;
            case GLFW_KEY_F:
                m_ui->camera.view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1));
                m_ui->camera.focal = 1.0f;
                UpdateProjection();
                break;
            case GLFW_KEY_R:
                m_ui->ToggleIsolateChannel(ISOLATE_RED);
                break;
            case GLFW_KEY_G:
                m_ui->ToggleIsolateChannel(ISOLATE_GREEN);
                break;
            case GLFW_KEY_B:
                m_ui->ToggleIsolateChannel(ISOLATE_BLUE);
                break;
            case GLFW_KEY_A:
                m_ui->ToggleIsolateChannel(ISOLATE_ALPHA);
                break;
            case GLFW_KEY_UP:
                DecrementActiveOperator();
                break;
            case GLFW_KEY_DOWN:
                IncrementActiveOperator();
                break;
            case GLFW_KEY_RIGHT:
                m_mapmaker->processOne();
                break;
            case GLFW_KEY_SPACE:
                TogglePause(!m_mapmaker->isPaused());
                break;
            }
        }
    }

    void DecrementActiveOperator()
    {
        size_t index = m_mapmaker->GetTargetIndex();
        if (index > 0)
        {
            SetActiveOperator(index - 1);
            glm::vec2 pos = m_ui->CursorPos();
            UpdatePixelPreview(pos.x, pos.y);
        }
    }

    void IncrementActiveOperator()
    {
        size_t index = m_mapmaker->GetTargetIndex();
        if (index < m_mapmaker->operators.size() - 1)
        {
            SetActiveOperator(index + 1);
            // If the next operator isn't processed yet, this won't be
            // able to read the texture, wasted call
            glm::vec2 pos = m_ui->CursorPos();
            UpdatePixelPreview(pos.x, pos.y);
        }
    }

    void OnResize(int width, int height)
    {
        UpdateProjection();
        // m_mapmaker->Resize(width, height);
        delete[] buffer;
        buffer = new float[m_mapmaker->Width() * m_mapmaker->Height() * 4];
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

    void SetActiveOperator(size_t index)
    {
        m_mapmaker->setTargetIndex(index);
    }

    void UpdateSetting(size_t index, std::string key, SettingValue value)
    {
        if (!m_mapmaker->updateSetting(index, key, value))
        {
            std::cerr << "Unable to update setting '" << key << "' for operator: " << m_mapmaker->operators[index]->name() << std::endl;
        }
    }

    void TogglePause(bool pause)
    {
        m_mapmaker->setPaused(pause);
    }

public:
    Application(MapMaker *mapmaker, UI *ui) : m_mapmaker(mapmaker), m_ui(ui)
    {
        // prep a buffer for reading the image values
        buffer = new float[m_mapmaker->Width() * m_mapmaker->Height() * 4];

        m_ui->SetMapMaker(mapmaker);
        m_ui->SetPixelPreview(&m_pixelPreview);

        // TODO: I'm being too lazy to work out the actual matrix for the definition
        m_ui->camera.view = glm::translate(m_ui->camera.view, glm::vec3(0, 0, -1));
        UpdateProjection();

        m_ui->cursorMoved.connect(this, &Application::OnMouseMoved);
        m_ui->mouseButtonChanged.connect(this, &Application::OnMouseButtonChanged);
        m_ui->mouseScrolled.connect(this, &Application::OnMouseScrolled);
        m_ui->closeRequested.connect(this, &Application::Close);
        m_ui->sizeChanged.connect(this, &Application::OnResize);
        m_ui->keyChanged.connect(this, &Application::OnKeyChanged);
        m_ui->activeOperatorChanged.connect(this, &Application::SetActiveOperator);
        m_ui->opSettingChanged.connect(this, &Application::UpdateSetting);
        m_ui->pauseToggled.connect(this, &Application::TogglePause);
    }
    ~Application()
    {
        delete[] buffer;
    }
    void Exec()
    {
        m_mapmaker->startProcessing();

        m_ui->use();
        // Make a quad to be used by the UI context - VAOs are not shared
        makeQuad(&quadVAO_UI);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        while (!m_ui->IsClosed())
        {
            glfwPollEvents();

            double now = glfwGetTime();
            if ((now - lastFrameTime) >= fpsLimit)
            {
                m_ui->Draw(m_mapmaker->GetRenderSet());
                m_ui->Display();
                lastFrameTime = now;
            }
        }

        m_mapmaker->stopProcessing();
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

    // TODO: The quad transform/vertices need to be scaled in the x-axis to match the image ratio
    MapMaker mapmaker(1024, 1024);
    if (!mapmaker.context.IsInitialised())
        return 1;

    UI ui = UI(1280, 720, "MapMakerUI", &mapmaker.context);
    if (!ui.IsInitialised())
        return 1;

    Application app = Application(&mapmaker, &ui);
    app.Exec();

    // XXX: Seeing output "Glfw Error 65537: The GLFW library is not initialized"
    // Think this fails because the window is already destroyed, non-critical
    glfwTerminate();
    return 0;
}