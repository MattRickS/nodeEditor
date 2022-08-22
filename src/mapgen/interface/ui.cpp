#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../constants.h"
#include "../operator.h"
#include "../renders.h"
#include "../shader.h"
#include "../util.h"
#include "window.h"
#include "ui.h"

void UI::onMouseMoved(double xpos, double ypos)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    cursorMoved.emit(xpos, ypos);
}

void UI::onMouseButtonChanged(int button, int action, int mods)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    mouseButtonChanged.emit(button, action, mods);
}

void UI::onMouseScrolled(double xoffset, double yoffset)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    mouseScrolled.emit(xoffset, yoffset);
}

// TODO: Get this working as a virtual override of onWindowResized
void UI::recalculateLayout()
{
    if (m_nodegraph)
    {
        m_nodegraph->setBounds(getNodegraphBounds());
    }
    if (m_viewport)
    {
        m_viewport->setBounds(getViewportBounds());
    }
    if (m_properties)
    {
        m_properties->setBounds(getOperatorPropertiesBounds());
    }
    if (m_viewProperties)
    {
        m_viewProperties->setBounds(getViewportPropertiesBounds());
    }
}

UI::UI(unsigned int width, unsigned int height, const char *name, const Context *sharedContext) : Window(name, width, height, sharedContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_nodegraph = new Nodegraph(this, getNodegraphBounds());
    m_viewport = new Viewport(this, getViewportBounds());
    m_properties = new Properties(this, getOperatorPropertiesBounds());
    m_viewProperties = new ViewportProperties(this, getViewportPropertiesBounds());
}
UI::~UI()
{
    if (m_nodegraph)
    {
        delete m_nodegraph;
    }
    if (m_viewport)
    {
        delete m_viewport;
    }
    if (m_properties)
    {
        delete m_properties;
    }
    if (m_viewProperties)
    {
        delete m_viewProperties;
    }
}

Viewport *UI::viewport() { return m_viewport; }
Nodegraph *UI::nodegraph() { return m_nodegraph; }
Properties *UI::properties() { return m_properties; }
ViewportProperties *UI::viewportProperties() { return m_viewProperties; }

void UI::setScene(Scene *scene)
{
    m_scene = scene;
    m_nodegraph->setScene(scene);
    m_properties->setScene(scene);
    m_viewport->setScene(scene);
    m_viewProperties->setScene(scene);
}
void UI::draw()
{
    if (m_viewport)
    {
        m_viewport->draw();
    }

    // Draws the UI around it
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_viewProperties)
    {
        m_viewProperties->draw();
    }
    if (m_nodegraph)
    {
        m_nodegraph->draw();
    }
    if (m_properties)
    {
        m_properties->draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Bounds UI::getViewportBounds() const
{
    return Bounds(m_width * m_opPropertiesWidthPercent, m_viewPropertiesHeight, m_width, m_height * m_viewportHeightPercent);
}
Bounds UI::getViewportPropertiesBounds() const
{
    return Bounds(m_width * m_opPropertiesWidthPercent, 0, m_width, m_viewPropertiesHeight);
}
Bounds UI::getOperatorPropertiesBounds() const
{
    return Bounds(0, 0, m_width * m_opPropertiesWidthPercent, m_height);
}
Bounds UI::getNodegraphBounds() const
{
    return Bounds(m_width * m_opPropertiesWidthPercent, m_height * m_viewportHeightPercent, m_width, m_height);
}
