#pragma once
#include <glm/glm.hpp>
#include <imgui.h>

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

/*
Class for handling User Interface. Responsible for layout and drawing the
interactive widgets but not for rendering the map.
*/
class UI : public Window
{
protected:
    Shader viewShader;
    float m_uiScreenWidthPercent = 0.25f;
    PixelPreview *m_pixelPreview;

    // Only emits the signal if the UI didn't capture it
    virtual void OnMouseMoved(double xpos, double ypos);
    virtual void OnMouseButtonChanged(int button, int action, int mods);
    virtual void OnMouseScrolled(double xoffset, double yoffset);

public:
    Camera camera;

    Signal<unsigned int, unsigned int> mapPosChanged;

    UI(unsigned int width, unsigned int height, const char *name = "MapMaker");

    void SetPixelPreview(PixelPreview *preview);
    void Draw(const RenderSet *const renderSet);

    glm::ivec4 GetViewportRegion();
    glm::ivec4 GetPropertiesRegion();

    glm::vec2 ScreenToWorldPos(glm::vec2 screenPos);
    glm::vec2 WorldToScreenPos(glm::vec2 mapPos);
};
