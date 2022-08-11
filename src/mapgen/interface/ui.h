#pragma once
#include <glm/glm.hpp>

#include "../constants.h"
#include "../operator.h"
#include "../renders.h"
#include "../scene.h"
#include "../shader.h"
#include "nodegraph.hpp"
#include "window.h"
#include "signal.hpp"

struct PixelPreview
{
    glm::vec4 value = glm::vec4(0);
    glm::ivec2 pos = glm::ivec2(0);
};

struct Camera
{
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    float focal = 1.0f;
};

enum IsolateChannel
{
    ISOLATE_NONE = -1,
    ISOLATE_RED = 0,
    ISOLATE_GREEN = 1,
    ISOLATE_BLUE = 2,
    ISOLATE_ALPHA = 3,
    ISOLATE_LAST
};

const char *getIsolateChannelName(IsolateChannel channel);

/*
Class for handling User Interface. Responsible for layout and drawing the
interactive widgets but not for rendering the map.
*/
class UI : public Window
{
protected:
    Shader viewShader;
    float m_opPropertiesWidthPercent = 0.25f;
    float m_viewPropertiesHeightPercent = 0.05f;
    PixelPreview *m_pixelPreview;
    Scene *m_scene = nullptr;
    Node *m_selectedNode = nullptr;
    std::string m_selectedLayer = DEFAULT_LAYER;
    IsolateChannel m_isolateChannel = ISOLATE_NONE;
    Nodegraph *m_nodegraph;

    // Only emits the signal if the UI didn't capture it
    virtual void OnMouseMoved(double xpos, double ypos);
    virtual void OnMouseButtonChanged(int button, int action, int mods);
    virtual void OnMouseScrolled(double xoffset, double yoffset);
    virtual void OnWindowResized(int width, int height);

    void DrawViewport();
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
    Camera camera;

    Signal<unsigned int, unsigned int> mapPosChanged;
    Signal<Node *> selectedNodeChanged;
    Signal<Node *, std::string, SettingValue> opSettingChanged;
    Signal<bool> pauseToggled;

    UI(unsigned int width, unsigned int height, const char *name = "MapMakerUI", Context *sharedContext = nullptr);
    ~UI();

    std::string GetCurrentLayer() const;

    void setSelectedNode(Node *node);
    void ToggleIsolateChannel(IsolateChannel channel);
    void setScene(Scene *scene);
    void SetPixelPreview(PixelPreview *preview);
    void Draw();

    glm::ivec4 GetViewportRegion() const;
    glm::ivec4 GetViewportPropertiesRegion() const;
    glm::ivec4 GetOperatorPropertiesRegion() const;
    glm::ivec4 getNodegraphRegion() const;

    glm::vec2 ScreenToWorldPos(glm::vec2 screenPos);
    glm::vec2 WorldToScreenPos(glm::vec2 mapPos);
};
