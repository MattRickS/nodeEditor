#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../bounds.hpp"
#include "../constants.h"
#include "../nodegraph/node.h"
#include "../shader.h"
#include "panel.hpp"
#include "window.h"

#include "viewport.h"

Viewport::Viewport(Window *window, Bounds bounds) : Panel(window, bounds), m_viewShader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/texture.fs") {}

Camera &Viewport::camera() { return m_camera; }
void Viewport::setLayer(std::string layer) { m_layer = layer; }
void Viewport::setNode(NodeID nodeID) { m_nodeID = nodeID; }
void Viewport::setScene(Scene *scene) { m_scene = scene; }
void Viewport::toggleIsolateChannel(Channel channel)
{
    m_isolateChannel = (m_isolateChannel == channel) ? Channel_All : channel;
}
Channel Viewport::isolatedChannel() const { return m_isolateChannel; }

void Viewport::draw()
{
    // Draws the texture into the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // GL uses inverted Y axis so use the maximum bound for the starting y pos
    glViewport(pos().x, m_window->height() - bounds().max().y, size().x, size().y);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    Channel channel = m_isolateChannel;
    glm::mat4 model{1.0f};
    // Takes node and layer so it can read the texture live as it's processed
    if (m_scene && m_nodeID && !m_layer.empty())
    {
        Node *node = m_scene->getCurrentGraph()->node(m_nodeID);
        if (node)
        {
            auto it = node->renderSet()->find(m_layer);
            if (it != node->renderSet()->end())
            {
                glBindTexture(GL_TEXTURE_2D, it->second->ID);
                model = glm::scale(model, glm::vec3(float(it->second->width) / it->second->height, 1, 1));
            }
        }
    }

    m_viewShader.use();
    m_viewShader.setMat4("model", model);
    m_viewShader.setMat4("view", m_camera.view);
    m_viewShader.setMat4("projection", m_camera.projection);
    m_viewShader.setInt("renderTexture", 0);
    m_viewShader.setInt("isolateChannel", (int)channel);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
