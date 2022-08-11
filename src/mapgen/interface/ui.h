#pragma once
#include <glm/glm.hpp>

#include "../constants.h"
#include "../operator.h"
#include "../renders.h"
#include "../scene.h"
#include "../shader.h"
#include "bounds.hpp"
#include "nodegraph.hpp"
#include "viewport.hpp"
#include "window.h"
#include "signal.hpp"

struct PixelPreview
{
    glm::vec4 value = glm::vec4(0);
    glm::ivec2 pos = glm::ivec2(0);
};

const char *getIsolateChannelName(IsolateChannel channel);

/*
Class for handling User Interface. Responsible for layout and drawing the
interactive widgets but not for rendering the map.
*/
class UI : public Window
{
protected:
    float m_opPropertiesWidthPercent = 0.25f;
    float m_viewPropertiesHeightPercent = 0.05f;
    PixelPreview *m_pixelPreview;
    Scene *m_scene = nullptr;
    std::string m_selectedLayer = DEFAULT_LAYER;
    Nodegraph *m_nodegraph;
    Viewport *m_viewport;

    // Only emits the signal if the UI didn't capture it
    virtual void OnMouseMoved(double xpos, double ypos);
    virtual void OnMouseButtonChanged(int button, int action, int mods);
    virtual void OnMouseScrolled(double xoffset, double yoffset);

    void DrawViewportProperties();
    void DrawOperatorProperties();

    void drawBoolSetting(Node *node, const Setting &setting);
    void drawFloatSetting(Node *node, const Setting &setting);
    void drawFloat2Setting(Node *node, const Setting &setting);
    void drawFloat3Setting(Node *node, const Setting &setting);
    void drawFloat4Setting(Node *node, const Setting &setting);
    void drawIntSetting(Node *node, const Setting &setting);
    void drawInt2Setting(Node *node, const Setting &setting);
    void drawUIntSetting(Node *node, const Setting &setting);
    void drawNodeSettings(Node *node);

public:
    Signal<unsigned int, unsigned int> mapPosChanged;
    Signal<Node *, std::string, SettingValue> opSettingChanged;
    Signal<bool> pauseToggled;
    Signal<std::string> layerChanged;
    Signal<IsolateChannel> channelChanged;

    UI(unsigned int width, unsigned int height, const char *name = "MapMakerUI", Context *sharedContext = nullptr);
    ~UI();

    Viewport *viewport();
    Nodegraph *nodegraph();

    std::string selectedLayer() const;

    void setScene(Scene *scene);
    void SetPixelPreview(PixelPreview *preview);
    void Draw();

    Bounds getViewportBounds() const;
    Bounds getViewportPropertiesBounds() const;
    Bounds getOperatorPropertiesBounds() const;
    Bounds getNodegraphBounds() const;

    glm::vec2 ScreenToWorldPos(glm::vec2 screenPos);
    glm::vec2 WorldToScreenPos(glm::vec2 mapPos);

    // Overriding OnWindowResized didn't work for some reason
    void recalculateLayout();
};
