#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../bounds.hpp"
#include "../constants.h"
#include "../scene.h"
#include "panel.hpp"
#include "signal.hpp"

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

    ViewportProperties(Bounds bounds);

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