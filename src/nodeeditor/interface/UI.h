#pragma once
#include "../Bounds.hpp"
#include "../gl/Context.hpp"
#include "../gl/RenderScene.h"
#include "Nodegraph.h"
#include "Properties.h"
#include "Viewport.h"
#include "ViewportProperties.h"
#include "Window.h"

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

    void setScene(RenderScene *scene);
    void draw();

    Bounds getViewportBounds() const;
    Bounds getViewportPropertiesBounds() const;
    Bounds getOperatorPropertiesBounds() const;
    Bounds getNodegraphBounds() const;

    // Overriding onWindowResized didn't work for some reason
    void recalculateLayout();

protected:
    float m_opPropertiesWidthPercent = 0.25f;
    float m_viewportHeightPercent = 0.6f;
    size_t m_viewPropertiesHeight = 20;
    RenderScene *m_scene = nullptr;
    Nodegraph *m_nodegraph;
    Viewport *m_viewport;
    Properties *m_properties;
    ViewportProperties *m_viewProperties;

    // Only emits the signal if the UI didn't capture it
    virtual void onMouseMoved(double xpos, double ypos);
    virtual void onMouseButtonChanged(int button, int action, int mods);
    virtual void onMouseScrolled(double xoffset, double yoffset);
    virtual void onKeyChanged(int key, int scancode, int action, int mods);
};
