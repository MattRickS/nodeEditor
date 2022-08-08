#pragma once
#include <glm/glm.hpp>

#include "../mapmaker.h"
#include "../operator.h"
#include "../renders.h"
#include "../shader.h"
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
    MapMaker *m_mapmaker = nullptr;
    Layer m_selectedLayer = LAYER_HEIGHTMAP;
    IsolateChannel m_isolateChannel = ISOLATE_NONE;

    // Only emits the signal if the UI didn't capture it
    virtual void OnMouseMoved(double xpos, double ypos);
    virtual void OnMouseButtonChanged(int button, int action, int mods);
    virtual void OnMouseScrolled(double xoffset, double yoffset);

    void DrawViewport(const RenderSet *const renderSet);
    void DrawViewportProperties(const RenderSet *const renderSet);
    void DrawOperatorProperties();

    void drawBoolSetting(size_t index, const Setting &setting);
    void drawFloatSetting(size_t index, const Setting &setting);
    void drawFloat2Setting(size_t index, const Setting &setting);
    void drawFloat3Setting(size_t index, const Setting &setting);
    void drawFloat4Setting(size_t index, const Setting &setting);
    void drawIntSetting(size_t index, const Setting &setting);
    void drawInt2Setting(size_t index, const Setting &setting);
    void drawUIntSetting(size_t index, const Setting &setting);
    void drawOperatorSettings(size_t index);

public:
    Camera camera;

    Signal<unsigned int, unsigned int> mapPosChanged;
    Signal<size_t> activeOperatorChanged;
    Signal<size_t, std::string, SettingValue> opSettingChanged;
    Signal<bool> pauseToggled;

    UI(unsigned int width, unsigned int height, const char *name = "MapMakerUI", Context *sharedContext = nullptr);

    Layer GetCurrentLayer() const;

    void ToggleIsolateChannel(IsolateChannel channel);
    void SetMapMaker(MapMaker *mapmaker);
    void SetPixelPreview(PixelPreview *preview);
    void Draw(const RenderSet *const renderSet);

    glm::ivec4 GetViewportRegion() const;
    glm::ivec4 GetViewportPropertiesRegion() const;
    glm::ivec4 GetOperatorPropertiesRegion() const;

    glm::vec2 ScreenToWorldPos(glm::vec2 screenPos);
    glm::vec2 WorldToScreenPos(glm::vec2 mapPos);
};
