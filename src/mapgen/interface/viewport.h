#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../bounds.hpp"
#include "../constants.h"
#include "../nodegraph/node.h"
#include "../scene.h"
#include "../shader.h"
#include "panel.hpp"
#include "window.h"

struct Camera
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    float focal = 1.0f;
};

class Viewport : public Panel
{
public:
    Viewport(Window *window, Bounds bounds);

    Camera &camera();
    void setLayer(std::string layer);
    void setNode(NodeID nodeID);
    void setScene(Scene *scene);
    void toggleIsolateChannel(Channel channel);
    Channel isolatedChannel() const;
    void draw() override;

protected:
    Shader m_viewShader;
    Camera m_camera;
    Channel m_isolateChannel = Channel_All;
    Scene *m_scene = nullptr;
    NodeID m_nodeID = 0;
    std::string m_layer = DEFAULT_LAYER;
};