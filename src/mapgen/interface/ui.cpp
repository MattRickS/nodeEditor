
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
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

UI::UI(unsigned int width, unsigned int height, const char *name, Context *sharedContext) : Window(name, width, height, sharedContext),
                                                                                            viewShader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/texture.fs")
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

std::string UI::GetCurrentLayer() const { return m_selectedLayer; }

void UI::ToggleIsolateChannel(IsolateChannel channel)
{
    m_isolateChannel = (m_isolateChannel == channel) ? ISOLATE_NONE : channel;
}
void UI::setScene(Scene *scene) { m_scene = scene; }
void UI::SetPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
void UI::Draw()
{
    DrawViewport();

    // Draws the UI around it
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DrawOperatorProperties();
    DrawViewportProperties();
    drawNodegraph();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void UI::DrawViewport()
{
    // Draws the texture into the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glm::ivec4 mapRegion = GetViewportRegion();
    glViewport(mapRegion.x, mapRegion.y, mapRegion.z, mapRegion.w);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    IsolateChannel channel = m_isolateChannel;
    if (m_selectedNode)
    {
        // Not binding a texture may result in garbage in the render, but that's fine for now
        const auto it = m_selectedNode->renderSet()->find(m_selectedLayer);
        if (it != m_selectedNode->renderSet()->end())
        {
            glBindTexture(GL_TEXTURE_2D, it->second->ID);
            // TODO: Tidy this up
            if (it->second->numChannels() == 1)
            {
                channel = ISOLATE_RED;
            }
        }
        else
        {
            std::cout << "No texture to draw" << std::endl;
        }
    }
    viewShader.use();
    viewShader.setMat4("view", camera.view);
    viewShader.setMat4("projection", camera.projection);
    viewShader.setInt("renderTexture", 0);
    viewShader.setInt("isolateChannel", (int)channel);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
void UI::DrawViewportProperties()
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    glm::ivec4 propertiesRegion = GetViewportPropertiesRegion();
    ImGui::SetNextWindowPos(ImVec2(propertiesRegion.x, propertiesRegion.y));
    ImGui::SetNextWindowSize(ImVec2(propertiesRegion.z, propertiesRegion.w));
    ImGui::Begin("Viewport Properties", &p_open, flags);

    ImGui::PushItemWidth(150.0f);
    if (ImGui::BeginCombo("##Layer", m_selectedLayer.c_str()))
    {
        if (m_selectedNode)
        {
            for (auto it = m_selectedNode->renderSet()->cbegin(); it != m_selectedNode->renderSet()->cend(); ++it)
            {
                bool isSelected = (m_selectedLayer == it->first);
                if (ImGui::Selectable(it->first.c_str(), isSelected))
                {
                    m_selectedLayer = it->first;
                    if (it->second->numChannels() == 1)
                    {
                        m_isolateChannel = ISOLATE_RED;
                    }
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
    if (ImGui::BeginCombo("##IsolateChannel", getIsolateChannelName(m_isolateChannel)))
    {
        // TODO: Tidy this up
        const auto it = m_selectedNode->renderSet()->find(m_selectedLayer);
        if (it != m_selectedNode->renderSet()->end() && it->second->numChannels() == 1)
        {
            ImGui::Selectable(getIsolateChannelName(ISOLATE_RED), true);
        }
        else
        {
            for (int channel = ISOLATE_NONE; channel != ISOLATE_LAST; ++channel)
            {
                bool isSelected = (m_isolateChannel == channel);
                if (ImGui::Selectable(getIsolateChannelName(IsolateChannel(channel)), isSelected))
                {
                    m_isolateChannel = IsolateChannel(channel);
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

    glm::ivec4 propertiesRegion = GetOperatorPropertiesRegion();
    ImGui::SetNextWindowPos(ImVec2(propertiesRegion.x, propertiesRegion.y));
    ImGui::SetNextWindowSize(ImVec2(propertiesRegion.z, propertiesRegion.w));
    ImGui::Begin("Mapmaker Properties", &p_open, flags);

    if (!m_scene)
    {
        return;
    }

    ImGui::BeginListBox("##Operators");
    // TODO: Doesn't need current node, needs selected - tracked separately
    Node *selectedNode = m_scene->getCurrentNode();
    // for (size_t i = 0; i < m_scene->operators.size(); ++i)
    // {
    //     if (ImGui::Selectable(m_scene->operators[i]->name().c_str(), i == selectedIndex))
    //     {
    //         selectedIndex = i;
    //         activeOperatorChanged.emit(i);
    //     }
    // }
    ImGui::EndListBox();
    ImGui::Text("Operator Settings");
    drawNodeSettings(selectedNode);

    bool isPaused = m_scene->isPaused();
    if (ImGui::Button(isPaused ? "Play" : "Pause"))
    {
        pauseToggled.emit(!isPaused);
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

glm::ivec4 UI::GetViewportRegion() const
{
    float panelWidth = m_width * m_opPropertiesWidthPercent;
    float panelHeight = m_height * m_viewPropertiesHeightPercent;
    // Remember, positions start from (0,0) at top-left
    return glm::ivec4(panelWidth, m_height * 0.5f, m_width - panelWidth, m_height * 0.5f - panelHeight);
}
glm::ivec4 UI::GetViewportPropertiesRegion() const
{
    float panelHeight = m_height * m_viewPropertiesHeightPercent;
    float panelWidth = m_width * m_opPropertiesWidthPercent;
    // Remember, positions start from (0,0) at top-left
    return glm::ivec4(panelWidth, 0, m_width - panelWidth, panelHeight);
}
glm::ivec4 UI::GetOperatorPropertiesRegion() const
{
    return glm::ivec4(0, 0, m_width * m_opPropertiesWidthPercent, m_height);
}
glm::ivec4 UI::getNodegraphRegion() const
{
    float panelWidth = m_width * m_opPropertiesWidthPercent;
    float panelHeight = m_height * 0.5f;
    return glm::ivec4(panelWidth, 0, m_width - panelWidth, panelHeight);
}

glm::vec2 UI::ScreenToWorldPos(glm::vec2 screenPos)
{
    glm::ivec4 viewportRegion = GetViewportRegion();
    glm::vec2 ndcPos = glm::vec2(
                           float(screenPos.x - viewportRegion.x) / viewportRegion.z,
                           float(screenPos.y - viewportRegion.y) / viewportRegion.w) *
                           2.0f -
                       1.0f;
    // This needs inverse matrices
    glm::vec4 worldPos = glm::inverse(camera.view) * glm::inverse(camera.projection) * glm::vec4(ndcPos, 0, 1);
    worldPos /= worldPos.w;
    return glm::vec2(worldPos.x, worldPos.y) * 0.5f + 0.5f;
}
glm::vec2 UI::WorldToScreenPos(glm::vec2 mapPos)
{
    glm::vec2 ndcPos = glm::vec2(mapPos.x / (float)m_width, mapPos.y / (float)m_height);
    // TODO: Convert to actual screen pos
    return ndcPos;
}

void UI::drawNodegraph()
{
    // TODO: Look into how to do this with ImGui. Even nodes should be possible
    // ImDrawList
    // drawList->AddRect(m_Bounds.Min, m_Bounds.Max,
    //     color, m_Rounding, 15, thickness);
    // drawList->AddRectFilled(
    //     m_Bounds.Min,
    //     m_Bounds.Max,
    //     m_Color, m_Rounding);
    // It's effectively what I thought, vertices and a draw call for each object.
    // Clever use of glScissor to minimise affected pixels.

    // glm::ivec4 nodeRegion = getNodegraphRegion();
    // glViewport(nodeRegion.x, nodeRegion.y, nodeRegion.z, nodeRegion.w);
    // glClearColor(0.3, 0.3, 0.3, 1.0);
    // glClear(GL_COLOR_BUFFER_BIT);

    // Try just getting a filled rect drawn on screen...
    auto drawList = ImGui::GetWindowDrawList();
    ImRect rect = ImRect(0, 0, 100, 100);
    drawList->AddRectFilled(rect.Min, rect.Max, 0, 0.0f);
}
