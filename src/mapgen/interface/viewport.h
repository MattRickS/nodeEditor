#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../bounds.hpp"
#include "../constants.h"
#include "../nodegraph/node.h"
#include "../shader.h"
#include "panel.hpp"

struct Camera
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    float focal = 1.0f;
};

class Viewport : public Panel
{
public:
    Viewport(Bounds bounds);

    Camera &camera();
    void setLayer(std::string layer);
    void setNode(Node *node);
    void toggleIsolateChannel(Channel channel);
    Channel isolatedChannel() const;
    void draw() override;

protected:
    Shader m_viewShader;
    Camera m_camera;
    Channel m_isolateChannel = Channel_All;
    Node *m_node = nullptr;
    std::string m_layer = DEFAULT_LAYER;
};