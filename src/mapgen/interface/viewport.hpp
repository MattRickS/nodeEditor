#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../renders.h"
#include "../shader.h"
#include "panel.hpp"

enum IsolateChannel
{
    ISOLATE_NONE = -1,
    ISOLATE_RED = 0,
    ISOLATE_GREEN = 1,
    ISOLATE_BLUE = 2,
    ISOLATE_ALPHA = 3,
    ISOLATE_LAST
};

struct Camera
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    float focal = 1.0f;
};

class Viewport : public Panel
{
public:
    Viewport(glm::ivec2 pos, glm::ivec2 size) : Panel(pos, size), m_viewShader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/texture.fs") {}

    Camera &camera() { return m_camera; }
    void setLayer(std::string layer) { m_layer = layer; }
    void setNode(Node *node) { m_node = node; }
    void ToggleIsolateChannel(IsolateChannel channel)
    {
        m_isolateChannel = (m_isolateChannel == channel) ? ISOLATE_NONE : channel;
    }
    IsolateChannel isolatedChannel() const { return m_isolateChannel; }

    void draw() override
    {
        // Draws the texture into the viewport
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(m_pos.x, m_pos.y, m_size.x, m_size.y);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        IsolateChannel channel = m_isolateChannel;
        // Takes node and layer so it can read the texture live as it's processed
        if (m_node && !m_layer.empty())
        {
            auto it = m_node->renderSet()->find(m_layer);
            if (it != m_node->renderSet()->end())
            {
                glBindTexture(GL_TEXTURE_2D, it->second->ID);
            }
        }

        m_viewShader.use();
        m_viewShader.setMat4("view", m_camera.view);
        m_viewShader.setMat4("projection", m_camera.projection);
        m_viewShader.setInt("renderTexture", 0);
        m_viewShader.setInt("isolateChannel", (int)channel);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

protected:
    Shader m_viewShader;
    Camera m_camera;
    IsolateChannel m_isolateChannel = ISOLATE_NONE;
    Node *m_node;
    std::string m_layer = DEFAULT_LAYER;
};