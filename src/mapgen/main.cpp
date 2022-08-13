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
#include "operator.h"
#include "renders.h"
#include "scene.h"
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
    Scene *m_scene;
    UI *m_ui;
    PixelPreview m_pixelPreview;

    Panel *m_panningPanel = nullptr;
    glm::vec2 lastCursorPos;
    bool m_isDragging = false;
    float *buffer;

    // FPS limiting as vsync does not appear to be working
    double lastFrameTime = 0;
    double fpsLimit = 1.0 / 60.0;

    Panel *panelAtPos(glm::vec2 pos)
    {
        if (m_ui->nodegraph()->bounds().contains(pos))
        {
            return m_ui->nodegraph();
        }
        else if (m_ui->viewport()->bounds().contains(pos))
        {
            return m_ui->viewport();
        }
        else
        {
            return nullptr;
        }
    }

    GraphElement *getElementAtPos(glm::vec2 pos)
    {
        // Elements are drawn from first to last, so iterate backwards to find the first element that's on top
        for (auto it = m_scene->getCurrentGraph()->rbegin(); it != m_scene->getCurrentGraph()->rend(); ++it)
        {
            if (elementContainsPos(&(*it), pos))
            {
                return &(*it);
            }
            for (size_t i = 0; i < it->numInputs(); ++i)
            {
                if (elementContainsPos(it->input(i), pos))
                {
                    return it->input(i);
                }
            }
            for (size_t i = 0; i < it->numOutputs(); ++i)
            {
                if (elementContainsPos(it->output(i), pos))
                {
                    return it->output(i);
                }
            }
        }
        return nullptr;
    }

    void OnMouseButtonChanged(int button, int action, [[maybe_unused]] int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            if (action == GLFW_PRESS)
            {
                lastCursorPos = m_ui->CursorPos();
                m_panningPanel = panelAtPos(lastCursorPos);
            }
            else if (action == GLFW_RELEASE)
            {
                m_panningPanel = nullptr;
            }
        }
        // TODO: Multi-selection dragbox
        else if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                GraphElement *el = getElementAtPos(m_ui->CursorPos());
                if (!el)
                {
                    return;
                }
                el->setSelectFlag(SelectFlag_Select);
                if (Node *node = dynamic_cast<Node *>(el))
                {
                    // TODO: Start drag movement
                    // Selecting the nodegraph will emit the selection changed signal
                    m_ui->nodegraph()->setSelectedNode(node);
                    m_isDragging = true;
                }
                else if (Connector *conn = dynamic_cast<Connector *>(el))
                {
                    // TODO: Start drag connection
                }
            }
            else if (action == GLFW_RELEASE)
            {
                m_isDragging = false;
            }
        }
    }

    const Texture *currentTexture() const
    {
        Node *node = m_scene->getViewNode();
        if (!node)
        {
            return nullptr;
        }
        std::string layer = m_ui->selectedLayer();
        auto it = node->renderSet()->find(layer);
        if (it == node->renderSet()->end())
        {
            return nullptr;
        }
        return it->second;
    }

    void UpdatePixelPreview(double xpos, double ypos)
    {
        // Invert the screen y-pos to get world position
        glm::vec2 worldPos = m_ui->ScreenToWorldPos({xpos, m_ui->Height() - ypos});
        if (worldPos.x >= 0 && worldPos.x < 1 && worldPos.y >= 0 && worldPos.y < 1)
        {
            int x = worldPos.x * m_scene->Width();
            int y = worldPos.y * m_scene->Height();
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
                size_t index = (y * m_scene->Width() + x) * texptr->numChannels();
                for (size_t i = 0; i < 4; ++i)
                    m_pixelPreview.value[i] = i < texptr->numChannels() ? buffer[index + i] : 0.0f;
            }
        }
    }

    bool elementContainsPos(GraphElement *el, glm::vec2 pos) const
    {
        return m_ui->nodegraph()->graphElementBounds(el).contains(pos);
    }

    void setHoverState(GraphElement *el, glm::vec2 cursorPos) const
    {
        if (elementContainsPos(el, cursorPos))
        {
            el->setSelectFlag(SelectFlag_Hover);
        }
        else
        {
            el->clearSelectFlag(SelectFlag_Hover);
        }
    }

    void OnMouseMoved(double xpos, double ypos)
    {
        UpdatePixelPreview(xpos, ypos);

        glm::vec2 cursorPos = glm::vec2(xpos, ypos);
        if (m_panningPanel)
        {
            if (m_panningPanel == m_ui->viewport())
            {
                // offset will have an inverted y as screenPos uses topleft=(0,0) but world uses botleft=(0,0)
                glm::vec2 offset = m_ui->ScreenToWorldPos(cursorPos) - m_ui->ScreenToWorldPos(lastCursorPos);
                m_ui->viewport()->camera().view = glm::translate(m_ui->viewport()->camera().view, glm::vec3(2.0f * offset.x, 2.0f * -offset.y, 0.0f));
            }
            else if (m_panningPanel == m_ui->nodegraph())
            {
                m_ui->nodegraph()->pan(cursorPos - lastCursorPos);
            }
        }

        if (m_isDragging)
        {
            Node *node = m_ui->nodegraph()->getSelectedNode();
            if (node)
            {
                node->move(cursorPos - lastCursorPos);
            }
        }

        // TODO: Line hover (tracked via Connection)
        // Hover state
        for (auto it = m_scene->getCurrentGraph()->begin(); it != m_scene->getCurrentGraph()->end(); ++it)
        {
            setHoverState(&(*it), cursorPos);
            for (size_t i = 0; i < it->numInputs(); ++i)
            {
                setHoverState(it->input(i), cursorPos);
            }
            for (size_t i = 0; i < it->numOutputs(); ++i)
            {
                setHoverState(it->output(i), cursorPos);
            }
        }
        lastCursorPos = cursorPos;
    }

    void OnMouseScrolled([[maybe_unused]] double xoffset, double yoffset)
    {
        Panel *panel = panelAtPos(m_ui->CursorPos());
        if (panel == m_ui->nodegraph())
        {
            m_ui->nodegraph()->zoom(1.0f + yoffset * 0.1f);
        }
        else if (panel == m_ui->viewport())
        {
            m_ui->viewport()->camera().focal *= (1.0f - yoffset * 0.1f);
        }
        UpdateProjection();
    }

    void OnKeyChanged(int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mode)
    {
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                Close();
                break;
            case GLFW_KEY_F:
                m_ui->viewport()->camera().view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1));
                m_ui->viewport()->camera().focal = 1.0f;
                UpdateProjection();
                break;
            case GLFW_KEY_R:
                m_ui->viewport()->ToggleIsolateChannel(ISOLATE_RED);
                break;
            case GLFW_KEY_G:
                m_ui->viewport()->ToggleIsolateChannel(ISOLATE_GREEN);
                break;
            case GLFW_KEY_B:
                m_ui->viewport()->ToggleIsolateChannel(ISOLATE_BLUE);
                break;
            case GLFW_KEY_A:
                m_ui->viewport()->ToggleIsolateChannel(ISOLATE_ALPHA);
                break;
            case GLFW_KEY_RIGHT:
                m_scene->processOne();
                break;
            case GLFW_KEY_SPACE:
                TogglePause(!m_scene->isPaused());
                break;
            }
        }
    }

    void OnResize([[maybe_unused]] int width, [[maybe_unused]] int height)
    {
        UpdateProjection();
        m_ui->recalculateLayout();
    }

    void onLayerChanged(std::string layerName)
    {
        m_ui->viewport()->setLayer(layerName);
    }

    void onChannelChanged(IsolateChannel channel)
    {
        m_ui->viewport()->ToggleIsolateChannel(channel);
    }

    void UpdateProjection()
    {
        Bounds viewportBounds = m_ui->getViewportBounds();
        float hAperture = viewportBounds.size().x / viewportBounds.size().y;
        static float vAperture = 1.0f;
        Camera &camera = m_ui->viewport()->camera();
        camera.projection = glm::ortho(-hAperture * camera.focal,
                                       hAperture * camera.focal,
                                       -vAperture * camera.focal,
                                       vAperture * camera.focal,
                                       CAM_NEAR,
                                       CAM_FAR);
    }

    // TODO: This needs to change, difference between selection and view
    void onNodeSelectionChanged(Node *node)
    {
        m_scene->setViewNode(node);
        m_ui->viewport()->setNode(node);
    }

    void UpdateSetting(Node *node, std::string key, SettingValue value)
    {
        node->updateSetting(key, value);
        // Must also set the scene as dirty so that the graph is re-evaluated
        m_scene->setDirty();
    }

    void TogglePause(bool pause)
    {
        m_scene->setPaused(pause);
    }

public:
    Application(Scene *mapmaker, UI *ui) : m_scene(mapmaker), m_ui(ui)
    {
        // prep a buffer for reading the image values
        buffer = new float[m_scene->Width() * m_scene->Height() * 4];

        m_ui->setScene(mapmaker);
        m_ui->SetPixelPreview(&m_pixelPreview);

        // TODO: I'm being too lazy to work out the actual matrix for the definition
        Camera &camera = m_ui->viewport()->camera();
        camera.view = glm::translate(camera.view, glm::vec3(0, 0, -1));
        UpdateProjection();

        m_ui->cursorMoved.connect(this, &Application::OnMouseMoved);
        m_ui->mouseButtonChanged.connect(this, &Application::OnMouseButtonChanged);
        m_ui->mouseScrolled.connect(this, &Application::OnMouseScrolled);
        m_ui->closeRequested.connect(this, &Application::Close);
        m_ui->sizeChanged.connect(this, &Application::OnResize);
        m_ui->keyChanged.connect(this, &Application::OnKeyChanged);
        m_ui->opSettingChanged.connect(this, &Application::UpdateSetting);
        m_ui->pauseToggled.connect(this, &Application::TogglePause);
        m_ui->layerChanged.connect(this, &Application::onLayerChanged);
        m_ui->channelChanged.connect(this, &Application::onChannelChanged);
        m_ui->nodegraph()->selectedNodeChanged.connect(this, &Application::onNodeSelectionChanged);
    }
    ~Application()
    {
        delete[] buffer;
    }
    void Exec()
    {
        m_scene->startProcessing();

        m_ui->use();
        // Make a quad to be used by the UI context - VAOs are not shared
        makeQuad(&quadVAO_UI);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // XXX: Hack to get something rendering until we have an interactive graph
        m_ui->nodegraph()->setSelectedNode(m_scene->getCurrentGraph()->node(2));

        while (!m_ui->IsClosed())
        {
            glfwPollEvents();

            double now = glfwGetTime();
            if ((now - lastFrameTime) >= fpsLimit)
            {
                m_ui->Draw();
                m_ui->Display();
                lastFrameTime = now;
            }
        }

        m_scene->stopProcessing();
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
    Scene mapmaker(1024, 1024);
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