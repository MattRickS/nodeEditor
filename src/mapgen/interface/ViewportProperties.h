#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Bounds.hpp"
#include "../constants.h"
#include "../nodegraph/Scene.h"
#include "Panel.hpp"
#include "Signal.hpp"
#include "Window.h"

struct PixelPreview
{
    glm::vec4 value = glm::vec4(0);
    glm::ivec2 pos = glm::ivec2(0);
};

class ViewportProperties : public Panel
{
public:
    Signal<std::string> layerChanged;
    Signal<Channel> channelChanged;

    ViewportProperties(Window *window, Bounds bounds);

    void setPixelPreview(PixelPreview *preview);
    void setScene(Scene *scene);

    std::string selectedLayer() const;

    void draw() override;

protected:
    Scene *m_scene = nullptr;
    std::string m_layer = DEFAULT_LAYER;
    Channel m_channel = Channel_All;
    PixelPreview *m_pixelPreview;
};