#pragma once
#include <glm/glm.hpp>

#include "../bounds.hpp"
#include "../scene.h"
#include "nodegraph.h"
#include "properties.h"
#include "viewport.h"
#include "viewproperties.h"
#include "window.h"

/*
Class for handling User Interface. Responsible for layout and drawing the
interactive widgets but not for rendering the map.
*/
class UI : public Window
{
public:
    UI(unsigned int width, unsigned int height, const char *name = "MapMakerUI", const Context *sharedContext = nullptr);
    ~UI();

    Viewport *viewport();
    Nodegraph *nodegraph();
    Properties *properties();
    ViewportProperties *viewportProperties();

    void setScene(Scene *scene);
    void draw();

    Bounds getViewportBounds() const;
    Bounds getViewportPropertiesBounds() const;
    Bounds getOperatorPropertiesBounds() const;
    Bounds getNodegraphBounds() const;

    glm::vec2 screenToWorldPos(glm::vec2 screenPos);
    glm::vec2 worldToScreenPos(glm::vec2 mapPos);

    // Overriding onWindowResized didn't work for some reason
    void recalculateLayout();

protected:
    float m_opPropertiesWidthPercent = 0.25f;
    float m_viewPropertiesHeightPercent = 0.05f;
    Scene *m_scene = nullptr;
    Nodegraph *m_nodegraph;
    Viewport *m_viewport;
    Properties *m_properties;
    ViewportProperties *m_viewProperties;

    // Only emits the signal if the UI didn't capture it
    virtual void onMouseMoved(double xpos, double ypos);
    virtual void onMouseButtonChanged(int button, int action, int mods);
    virtual void onMouseScrolled(double xoffset, double yoffset);
};
