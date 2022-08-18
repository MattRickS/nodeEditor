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
    if (io.WantCaptureMouse && !m_nodegraph->bounds().contains(cursorPos()))
        return;

    cursorMoved.emit(xpos, ypos);
}

void UI::onMouseButtonChanged(int button, int action, int mods)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse && !m_nodegraph->bounds().contains(cursorPos()))
        return;

    mouseButtonChanged.emit(button, action, mods);
}

void UI::onMouseScrolled(double xoffset, double yoffset)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse && !m_nodegraph->bounds().contains(cursorPos()))
        return;

    mouseScrolled.emit(xoffset, yoffset);
}

// TODO: Get this working as a virtual override of onWindowResized
void UI::recalculateLayout()
{
    if (m_nodegraph)
    {
        Bounds bounds = getNodegraphBounds();
        m_nodegraph->setPos(bounds.pos());
        m_nodegraph->setSize(bounds.size());
    }
    if (m_viewport)
    {
        Bounds bounds = getViewportBounds();
        m_viewport->setPos(bounds.pos());
        m_viewport->setSize(bounds.size());
    }
}

UI::UI(unsigned int width, unsigned int height, const char *name, const Context *sharedContext) : Window(name, width, height, sharedContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_nodegraph = new Nodegraph(getNodegraphBounds());
    m_viewport = new Viewport(getViewportBounds());
    m_properties = new Properties(getOperatorPropertiesBounds());
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
}

Viewport *UI::viewport() { return m_viewport; }
Nodegraph *UI::nodegraph() { return m_nodegraph; }
Properties *UI::properties() { return m_properties; }

std::string UI::selectedLayer() const
{
    return m_selectedLayer;
}

void UI::setScene(Scene *scene)
{
    m_scene = scene;
    m_nodegraph->setScene(scene);
    m_properties->setScene(scene);
    m_viewport->setScene(scene);
}
void UI::setPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
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

    drawViewportProperties();
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
void UI::drawViewportProperties()
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    Bounds propertiesBounds = getViewportPropertiesBounds();
    ImGui::SetNextWindowPos(ImVec2(propertiesBounds.pos().x, propertiesBounds.pos().y));
    ImGui::SetNextWindowSize(ImVec2(propertiesBounds.size().x, propertiesBounds.size().y));
    ImGui::Begin("Viewport Properties", &p_open, flags);

    ImGui::PushItemWidth(150.0f);
    if (ImGui::BeginCombo("##Layer", m_selectedLayer.c_str()))
    {
        if (m_scene)
        {
            Node *selectedNode = m_scene->getSelectedNode();
            if (selectedNode)
            {
                const RenderSet *renderSet = selectedNode->renderSet();
                for (auto it = renderSet->cbegin(); it != renderSet->cend(); ++it)
                {
                    bool isSelected = (m_selectedLayer == it->first);
                    if (ImGui::Selectable(it->first.c_str(), isSelected))
                    {
                        m_selectedLayer = it->first;
                        layerChanged.emit(it->first);
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::PushItemWidth(100.0f);
    if (ImGui::BeginCombo("##Channel", getChannelName(m_viewport->isolatedChannel())))
    {
        for (int channel = Channel_All; channel != Channel_Last; ++channel)
        {
            bool isSelected = (m_viewport->isolatedChannel() == channel);
            if (ImGui::Selectable(getChannelName(Channel(channel)), isSelected))
            {
                channelChanged.emit(Channel(channel));
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    if (m_pixelPreview)
    {
        ImGui::BeginDisabled();

        ImGui::SameLine();
        ImGui::PushItemWidth(250.0f);
        ImGui::DragFloat4("Pixel##PixelColor", (float *)&(*m_pixelPreview).value);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        ImGui::InputInt2("##PixelPos", (int *)&(*m_pixelPreview).pos);
        ImGui::PopItemWidth();

        ImGui::EndDisabled();
    }

    ImGui::End();
}

Bounds UI::getViewportBounds() const
{
    return Bounds(m_width * m_opPropertiesWidthPercent, m_height * m_viewPropertiesHeightPercent, m_width, m_height * 0.5f);
}
Bounds UI::getViewportPropertiesBounds() const
{
    return Bounds(m_width * m_opPropertiesWidthPercent, 0, m_width, m_height * m_viewPropertiesHeightPercent);
}
Bounds UI::getOperatorPropertiesBounds() const
{
    return Bounds(0, 0, m_width * m_opPropertiesWidthPercent, m_height);
}
Bounds UI::getNodegraphBounds() const
{
    return Bounds(m_width * m_opPropertiesWidthPercent, m_height * 0.5f, m_width, m_height);
}

glm::vec2 UI::screenToWorldPos(glm::vec2 screenPos)
{
    Bounds viewportBounds = getViewportBounds();
    glm::vec2 ndcPos = glm::vec2(
                           float(screenPos.x - viewportBounds.min().x) / viewportBounds.size().x,
                           // GL uses inverted Y axis
                           float(screenPos.y - viewportBounds.max().y) / viewportBounds.size().y) *
                           2.0f -
                       1.0f;
    // This needs inverse matrices
    glm::vec4 worldPos = glm::inverse(m_viewport->camera().view) * glm::inverse(m_viewport->camera().projection) * glm::vec4(ndcPos, 0, 1);
    worldPos /= worldPos.w;
    return glm::vec2(worldPos.x, worldPos.y) * 0.5f + 0.5f;
}
glm::vec2 UI::worldToScreenPos(glm::vec2 mapPos)
{
    glm::vec2 ndcPos = glm::vec2(mapPos.x / (float)m_width, mapPos.y / (float)m_height);
    // TODO: Convert to actual screen pos
    return ndcPos;
}
