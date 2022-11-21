#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "../Bounds.hpp"
#include "../constants.h"
#include "../nodegraph/Node.h"
#include "../gl/RenderSetOperator.h"
#include "../gl/Texture.h"
#include "Panel.hpp"
#include "Window.h"

#include "Viewport.h"

Viewport::Viewport(Window *window, Bounds bounds) : Panel(window, bounds), m_viewShader("src/nodeeditor/shaders/posUV.vs", "src/nodeeditor/shaders/texture.fs") {}

Camera &Viewport::camera() { return m_camera; }
void Viewport::setChannel(Channel channel) { m_isolateChannel = channel; }
void Viewport::setLayer(std::string layer) { m_layer = layer; }
void Viewport::setScene(Scene *scene) { m_scene = scene; }

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
    if (m_scene && !m_layer.empty())
    {
        Node *node = m_scene->getViewNode();
        if (node)
        {
            Op::RenderSetOperator const *op = dynamic_cast<Op::RenderSetOperator const *>(node->op());
            if (op)
            {
                Texture const *texture = op->layer(m_layer);
                if (texture)
                {
                    glBindTexture(GL_TEXTURE_2D, texture->id());
                    model = glm::scale(model, glm::vec3(float(texture->width()) / texture->height(), 1, 1));
                }
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

glm::vec2 Viewport::screenToWorldPos(glm::vec2 screenPos)
{
    glm::vec2 ndcPos = glm::vec2(
                           float(screenPos.x - m_bounds.min().x) / m_bounds.size().x,
                           // GL uses inverted Y axis
                           float(screenPos.y - (m_window->height() - m_bounds.max().y)) / m_bounds.size().y) *
                           2.0f -
                       1.0f;
    // This needs inverse matrices
    glm::vec4 worldPos = glm::inverse(camera().view) * glm::inverse(camera().projection) * glm::vec4(ndcPos, 0, 1);
    worldPos /= worldPos.w;
    return glm::vec2(worldPos.x, worldPos.y) * 0.5f + 0.5f;
}
glm::vec2 Viewport::worldToScreenPos(glm::vec2 mapPos)
{
    glm::vec2 ndcPos = glm::vec2(mapPos.x / (float)m_window->width(), mapPos.y / (float)m_window->height());
    // TODO: Convert to actual screen pos
    return ndcPos;
}
