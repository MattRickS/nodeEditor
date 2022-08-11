#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"
#include "window.h"
#include "ui.h"

const char *getIsolateChannelName(IsolateChannel channel)
{
    switch (channel)
    {
    case ISOLATE_NONE:
        return "RGBA";
    case ISOLATE_RED:
        return "Red";
    case ISOLATE_GREEN:
        return "Green";
    case ISOLATE_BLUE:
        return "Blue";
    case ISOLATE_ALPHA:
        return "Alpha";
    default:
        return "";
    }
}

void UI::OnMouseMoved(double xpos, double ypos)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    // Invert the ypos so botleft is (0,0) instead of GL's topleft
    ypos = m_height - ypos;
    cursorMoved.emit(xpos, ypos);
}

void UI::OnMouseButtonChanged(int button, int action, int mods)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    mouseButtonChanged.emit(button, action, mods);
}

void UI::OnMouseScrolled(double xoffset, double yoffset)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    mouseScrolled.emit(xoffset, yoffset);
}

// TODO: Get this working as a virtual override
void UI::resizeInternals([[maybe_unused]] int width, [[maybe_unused]] int height)
{
    if (m_nodegraph)
    {
        Bounds bounds = getNodegraphBounds();
        m_nodegraph->setPos(bounds.min);
        m_nodegraph->setSize(bounds.size());
    }
}

UI::UI(unsigned int width, unsigned int height, const char *name, Context *sharedContext) : Window(name, width, height, sharedContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_nodegraph = new Nodegraph(getNodegraphBounds());
    sizeChanged.connect(this, &UI::resizeInternals);

    m_viewport = new Viewport(getViewportBounds());
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
}

Viewport *UI::viewport() { return m_viewport; }
Nodegraph *UI::nodegraph() { return m_nodegraph; }

std::string UI::selectedLayer() const
{
    return m_selectedLayer;
}

void UI::setScene(Scene *scene)
{
    m_scene = scene;
    m_nodegraph->setScene(scene);
}
void UI::SetPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
void UI::Draw()
{
    if (m_viewport)
    {
        m_viewport->draw();
    }

    // Draws the UI around it
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DrawOperatorProperties();
    DrawViewportProperties();
    if (m_nodegraph)
    {
        m_nodegraph->draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void UI::DrawViewportProperties()
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    Bounds propertiesBounds = getViewportPropertiesBounds();
    ImGui::SetNextWindowPos(ImVec2(propertiesBounds.min.x, propertiesBounds.min.y));
    ImGui::SetNextWindowSize(ImVec2(propertiesBounds.size().x, propertiesBounds.size().y));
    ImGui::Begin("Viewport Properties", &p_open, flags);

    ImGui::PushItemWidth(150.0f);
    if (ImGui::BeginCombo("##Layer", m_selectedLayer.c_str()))
    {
        if (m_nodegraph && m_nodegraph->getSelectedNode())
        {
            const RenderSet *renderSet = m_nodegraph->getSelectedNode()->renderSet();
            for (auto it = renderSet->cbegin(); it != renderSet->cend(); ++it)
            {
                bool isSelected = (m_selectedLayer == it->first);
                if (ImGui::Selectable(it->first.c_str(), isSelected))
                {
                    layerChanged.emit(it->first);
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::PushItemWidth(100.0f);
    if (ImGui::BeginCombo("##IsolateChannel", getIsolateChannelName(m_viewport->isolatedChannel())))
    {
        for (int channel = ISOLATE_NONE; channel != ISOLATE_LAST; ++channel)
        {
            bool isSelected = (m_viewport->isolatedChannel() == channel);
            if (ImGui::Selectable(getIsolateChannelName(IsolateChannel(channel)), isSelected))
            {
                channelChanged.emit(IsolateChannel(channel));
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
void UI::DrawOperatorProperties()
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    Bounds propertiesBounds = getOperatorPropertiesBounds();
    ImGui::SetNextWindowPos(ImVec2(propertiesBounds.min.x, propertiesBounds.min.y));
    ImGui::SetNextWindowSize(ImVec2(propertiesBounds.size().x, propertiesBounds.size().y));
    ImGui::Begin("Mapmaker Properties", &p_open, flags);

    if (m_scene)
    {
        ImGui::Text("Operator Settings");
        drawNodeSettings(m_nodegraph->getSelectedNode());

        bool isPaused = m_scene->isPaused();
        if (ImGui::Button(isPaused ? "Play" : "Pause"))
        {
            pauseToggled.emit(!isPaused);
        }
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void UI::drawBoolSetting(Node *node, const Setting &setting)
{
    bool value = setting.value<bool>();
    if (ImGui::Checkbox(setting.name().c_str(), &value))
        opSettingChanged.emit(node, setting.name(), value);
}
void UI::drawFloatSetting(Node *node, const Setting &setting)
{
    float value = setting.value<float>();
    // TODO: Setting options for min/max
    if (ImGui::SliderFloat(setting.name().c_str(), &value, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
        opSettingChanged.emit(node, setting.name(), value);
}
void UI::drawFloat2Setting(Node *node, const Setting &setting)
{
    glm::vec2 value = setting.value<glm::vec2>();
    if (ImGui::DragFloat2(setting.name().c_str(), (float *)&value))
        opSettingChanged.emit(node, setting.name(), value);
}
void UI::drawFloat3Setting(Node *node, const Setting &setting)
{
    glm::vec3 value = setting.value<glm::vec3>();
    if (ImGui::DragFloat3(setting.name().c_str(), (float *)&value))
        opSettingChanged.emit(node, setting.name(), value);
}
void UI::drawFloat4Setting(Node *node, const Setting &setting)
{
    glm::vec4 value = setting.value<glm::vec4>();
    if (ImGui::DragFloat4(setting.name().c_str(), (float *)&value))
        opSettingChanged.emit(node, setting.name(), value);
}
void UI::drawIntSetting(Node *node, const Setting &setting)
{
    int value = setting.value<int>();
    if (ImGui::InputInt(setting.name().c_str(), &value))
        opSettingChanged.emit(node, setting.name(), value);
}
void UI::drawInt2Setting(Node *node, const Setting &setting)
{
    glm::ivec2 offset = setting.value<glm::ivec2>();
    if (ImGui::DragInt2(setting.name().c_str(), (int *)&offset))
        opSettingChanged.emit(node, setting.name(), offset);
}
void UI::drawUIntSetting(Node *node, const Setting &setting)
{
    unsigned int value = setting.value<unsigned int>();
    if (ImGui::InputScalar(setting.name().c_str(), ImGuiDataType_U32, &value))
        opSettingChanged.emit(node, setting.name(), value);
}
void UI::drawNodeSettings(Node *node)
{
    if (!node)
    {
        return;
    }
    for (auto it = node->settings()->begin(); it != node->settings()->end(); ++it)
    {
        switch (it->type())
        {
        case S_BOOL:
            drawBoolSetting(node, *it);
            break;
        case S_FLOAT:
            drawFloatSetting(node, *it);
            break;
        case S_FLOAT2:
            drawFloat2Setting(node, *it);
            break;
        case S_FLOAT3:
            drawFloat3Setting(node, *it);
            break;
        case S_FLOAT4:
            drawFloat4Setting(node, *it);
            break;
        case S_INT:
            drawIntSetting(node, *it);
            break;
        case S_INT2:
            drawInt2Setting(node, *it);
            break;
        case S_UINT:
            drawUIntSetting(node, *it);
            break;
        }
    }
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

glm::vec2 UI::ScreenToWorldPos(glm::vec2 screenPos)
{
    Bounds viewportBounds = getViewportBounds();
    glm::vec2 ndcPos = glm::vec2(
                           float(screenPos.x - viewportBounds.min.x) / viewportBounds.size().x,
                           // GL uses inverted Y axis
                           float(screenPos.y - viewportBounds.max.y) / viewportBounds.size().y) *
                           2.0f -
                       1.0f;
    // This needs inverse matrices
    glm::vec4 worldPos = glm::inverse(m_viewport->camera().view) * glm::inverse(m_viewport->camera().projection) * glm::vec4(ndcPos, 0, 1);
    worldPos /= worldPos.w;
    return glm::vec2(worldPos.x, worldPos.y) * 0.5f + 0.5f;
}
glm::vec2 UI::WorldToScreenPos(glm::vec2 mapPos)
{
    glm::vec2 ndcPos = glm::vec2(mapPos.x / (float)m_width, mapPos.y / (float)m_height);
    // TODO: Convert to actual screen pos
    return ndcPos;
}
