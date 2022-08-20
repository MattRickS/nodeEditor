#define GLEW_STATIC
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "application.h"
#include "constants.h"
#include "interface/panel.hpp"
#include "interface/viewport.h"
#include "interface/ui.h"
#include "nodegraph/graphelement.h"
#include "operator.h"
#include "renders.h"
#include "scene.h"
#include "settings.h"
#include "shader.h"
#include "util.h"

Application::Application(Scene *mapmaker, UI *ui) : m_scene(mapmaker), m_ui(ui)
{
    // prep a buffer for reading the image values
    buffer = new float[m_scene->Width() * m_scene->Height() * 4];

    m_ui->setScene(mapmaker);
    m_ui->viewportProperties()->setPixelPreview(&m_pixelPreview);

    // TODO: I'm being too lazy to work out the actual matrix for the definition
    Camera &camera = m_ui->viewport()->camera();
    camera.view = glm::translate(camera.view, glm::vec3(0, 0, -1));
    updateProjection();

    m_ui->keyChanged.connect(this, &Application::onKeyChanged);
    m_ui->mouseButtonChanged.connect(this, &Application::onMouseButtonChanged);
    m_ui->cursorMoved.connect(this, &Application::onMouseMoved);
    m_ui->mouseScrolled.connect(this, &Application::onMouseScrolled);
    m_ui->sizeChanged.connect(this, &Application::onResize);
    m_ui->closeRequested.connect(this, &Application::close);
    m_ui->nodegraph()->newNodeRequested.connect(this, &Application::createNode);
    m_ui->properties()->opSettingChanged.connect(this, &Application::updateSetting);
    m_ui->properties()->pauseToggled.connect(this, &Application::togglePause);
    m_ui->viewportProperties()->channelChanged.connect(this, &Application::onChannelChanged);
    m_ui->viewportProperties()->layerChanged.connect(this, &Application::onLayerChanged);
}

Application::~Application()
{
    delete[] buffer;
}

void Application::exec()
{
    m_scene->startProcessing();

    m_ui->use();
    // Make a quad to be used by the UI context - VAOs are not shared
    makeQuad(&quadVAO_UI);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    while (!m_ui->isClosed())
    {
        glfwPollEvents();

        double now = glfwGetTime();
        if ((now - lastFrameTime) >= fpsLimit)
        {
            m_ui->draw();
            m_ui->display();
            lastFrameTime = now;
        }
    }

    m_scene->stopProcessing();
}

void Application::close()
{
    m_ui->close();
}

// Signals
void Application::onChannelChanged(Channel channel)
{
    m_ui->viewport()->toggleIsolateChannel(channel);
}

void Application::onLayerChanged(std::string layerName)
{
    m_ui->viewport()->setLayer(layerName);
}

void Application::onKeyChanged(int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mode)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            close();
            break;
        case GLFW_KEY_F:
            if (m_ui->viewport()->bounds().contains(m_ui->cursorPos()))
            {
                m_ui->viewport()->camera().view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1));
                m_ui->viewport()->camera().focal = 1.0f;
                updateProjection();
            }
            else if (m_ui->nodegraph()->bounds().contains(m_ui->cursorPos()))
            {
                Bounds b = m_scene->getCurrentGraph()->bounds();
                m_ui->nodegraph()->fitBounds(b);
            }
            break;
        case GLFW_KEY_R:
            m_ui->viewport()->toggleIsolateChannel(Channel_Red);
            break;
        case GLFW_KEY_G:
            m_ui->viewport()->toggleIsolateChannel(Channel_Green);
            break;
        case GLFW_KEY_B:
            m_ui->viewport()->toggleIsolateChannel(Channel_Blue);
            break;
        case GLFW_KEY_A:
            m_ui->viewport()->toggleIsolateChannel(Channel_Alpha);
            break;
        case GLFW_KEY_RIGHT:
            m_scene->processOne();
            break;
        case GLFW_KEY_SPACE:
            togglePause(!m_scene->isPaused());
            break;
        case GLFW_KEY_TAB:
            if (m_ui->nodegraph()->bounds().contains(m_ui->cursorPos()))
            {
                m_ui->nodegraph()->startNodeSelection(m_ui->cursorPos());
            }
            break;
        case GLFW_KEY_DELETE:
            deleteSelectedNode();
            break;
        }
    }
}

void Application::onMouseButtonChanged(int button, int action, [[maybe_unused]] int mods)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            lastCursorPos = m_ui->cursorPos();
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
            // The node selection will capture it's own input, clicking outside of it should close it
            m_ui->nodegraph()->finishNodeSelection();

            GraphElement *el = getElementAtPos(m_ui->cursorPos());
            if (el)
            {
                if (Node *node = dynamic_cast<Node *>(el))
                {
                    setSelectedNode(node);
                    if (mods & GLFW_MOD_CONTROL)
                    {
                        setViewNode(node);
                    }
                    else
                    {
                        m_isDragging = true;
                    }
                }
                else if (Connector *conn = dynamic_cast<Connector *>(el))
                {
                    if (conn->type() == Connector::Input && conn->numConnections() > 0)
                    {
                        conn->disconnectAll();
                        m_scene->setDirty();
                    }
                    m_ui->nodegraph()->startConnection(conn);
                }
            }
        }
        else if (action == GLFW_RELEASE)
        {
            m_isDragging = false;
            if (m_ui->nodegraph()->activeConnection())
            {
                GraphElement *el = getElementAtPos(m_ui->cursorPos());
                if (Connector *conn = dynamic_cast<Connector *>(el))
                {
                    if (m_ui->nodegraph()->activeConnection()->connect(conn))
                    {
                        // A state has changed, so let it reevaluate
                        m_scene->setDirty();
                    }
                    else
                    {
                        // Eg, same conncector, not output to input, connection is full, etc...
                        LOG_DEBUG("Connectors found but failed to connect");
                    }
                }
                m_ui->nodegraph()->finishConnection();
            }
        }
    }
}

void Application::onMouseMoved(double xpos, double ypos)
{
    updatePixelPreview(xpos, ypos);

    glm::vec2 cursorPos = glm::vec2(xpos, ypos);
    if (m_panningPanel)
    {
        if (m_panningPanel == m_ui->viewport())
        {
            // offset will have an inverted y as screenPos uses topleft=(0,0) but world uses botleft=(0,0)
            glm::vec2 offset = m_ui->screenToWorldPos(cursorPos) - m_ui->screenToWorldPos(lastCursorPos);
            m_ui->viewport()->camera().view = glm::translate(m_ui->viewport()->camera().view, glm::vec3(2.0f * offset.x, 2.0f * -offset.y, 0.0f));
        }
        else if (m_panningPanel == m_ui->nodegraph())
        {
            m_ui->nodegraph()->pan(cursorPos - lastCursorPos);
        }
    }

    if (m_isDragging)
    {
        Node *node = m_scene->getSelectedNode();
        if (node)
        {
            glm::vec2 worldOffset = m_ui->nodegraph()->screenToWorldPos(cursorPos) - m_ui->nodegraph()->screenToWorldPos(lastCursorPos);
            node->move(worldOffset);
        }
    }

    if (m_ui->nodegraph()->activeConnection())
    {
        m_ui->nodegraph()->updateConnection(m_ui->cursorPos());
    }

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

void Application::onMouseScrolled([[maybe_unused]] double xoffset, double yoffset)
{
    Panel *panel = panelAtPos(m_ui->cursorPos());
    if (panel == m_ui->nodegraph())
    {
        m_ui->nodegraph()->scaleFromPos(m_ui->cursorPos(), 1.0f + yoffset * 0.1f);
    }
    else if (panel == m_ui->viewport())
    {
        m_ui->viewport()->camera().focal *= (1.0f - yoffset * 0.1f);
    }
    updateProjection();
}

void Application::onResize([[maybe_unused]] int width, [[maybe_unused]] int height)
{
    updateProjection();
    m_ui->recalculateLayout();
}

void Application::setSelectedNode(Node *node)
{
    Node *selectedNode = m_scene->getSelectedNode();
    if (selectedNode)
    {
        selectedNode->clearSelectFlag(SelectFlag_Select);
    }
    // Might be no new selected node
    if (node)
    {
        node->setSelectFlag(SelectFlag_Select);
        m_ui->properties()->setNode(node->id());
    }
}

// Viewport
const Texture *Application::currentTexture() const
{
    Node *node = m_scene->getViewNode();
    if (!node)
    {
        return nullptr;
    }
    std::string layer = m_ui->viewportProperties()->selectedLayer();
    auto it = node->renderSet()->find(layer);
    if (it == node->renderSet()->end())
    {
        return nullptr;
    }
    return it->second;
}

void Application::togglePause(bool pause)
{
    m_scene->setPaused(pause);
}

void Application::updatePixelPreview(double xpos, double ypos)
{
    // Invert the screen y-pos to get world position
    glm::vec2 worldPos = m_ui->screenToWorldPos({xpos, m_ui->height() - ypos});
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

void Application::updateProjection()
{
    Bounds viewportBounds = m_ui->getViewportBounds();
    float hAperture = viewportBounds.size().x / viewportBounds.size().y;
    static float vAperture = 1.0f;
    Camera &camera = m_ui->viewport()->camera();
    camera.projection = glm::ortho(-hAperture * camera.focal,
                                   hAperture * camera.focal,
                                   -vAperture * camera.focal,
                                   vAperture * camera.focal,
                                   m_camNear,
                                   m_camFar);
}

// Nodegraph
bool Application::elementContainsPos(GraphElement *el, glm::vec2 pos) const
{
    return m_ui->nodegraph()->graphElementBounds(el).contains(pos);
}

GraphElement *Application::getElementAtPos(glm::vec2 pos)
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

Panel *Application::panelAtPos(glm::vec2 pos)
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

void Application::setHoverState(GraphElement *el, glm::vec2 cursorPos) const
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

// Scene
void Application::createNode(glm::ivec2 screenPos, std::string nodeType)
{
    LOG_INFO("Creating: %s", nodeType.c_str());
    NodeID nodeID = m_scene->getCurrentGraph()->createNode(nodeType);
    glm::vec2 worldPos = m_ui->nodegraph()->screenToWorldPos(screenPos);
    m_scene->getNode(nodeID)->setPos(worldPos);
}

void Application::deleteSelectedNode()
{
    Node *selectedNode = m_scene->getSelectedNode();
    if (selectedNode)
    {
        setSelectedNode(nullptr);
        m_scene->getCurrentGraph()->deleteNode(selectedNode->id());
        m_scene->setDirty();
    }
}

void Application::setViewNode(Node *node)
{
    m_scene->setViewNode(node);
    m_ui->viewport()->setNode(node->id());
}

void Application::updateSetting(Node *node, std::string key, SettingValue value)
{
    node->updateSetting(key, value);
    // Must also set the scene as dirty so that the graph is re-evaluated
    m_scene->setDirty();
}
