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
    // PerlinNoiseUI m_perlinUI;
    bool isPanning = false;
    glm::vec2 lastCursorPos;

    // TODO: Resizing window should not distort the image
    glm::mat4 viewTransform = glm::mat4(1.0f);
    glm::vec2 transformOffset = glm::vec2(0.0f);
    float transformZoom = 1.0f;

    unsigned int uiWidth();
    void updateTransform();

    // Only emits the signal if the UI didn't capture it
    virtual void OnMouseMoved(double xpos, double ypos);
    virtual void OnMouseButtonChanged(int button, int action, int mods);
    virtual void OnMouseScrolled(double xoffset, double yoffset);

public:
    Signal<unsigned int, unsigned int> mapPosChanged;

    UI(const char *name = "MapMaker", unsigned int width = 1280, unsigned int height = 720);

    void SetPixelPreview(PixelPreview *preview);
    void Draw(const RenderSet *const renderSet);
    glm::ivec4 GetMapViewportRegion();
};
