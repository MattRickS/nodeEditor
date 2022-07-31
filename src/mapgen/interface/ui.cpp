
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../operators/perlin.h"
#include "../operators/voronoi.hpp"
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

void UI::SetMapMaker(MapMaker *mapmaker) { m_mapmaker = mapmaker; }
void UI::SetPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
void UI::Draw(const RenderSet *const renderSet)
{
    DrawViewport(renderSet);

    // Draws the UI around it
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    DrawOperatorProperties();
    DrawViewportProperties(renderSet);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void UI::DrawViewport(const RenderSet *const renderSet)
{
    // Draws the texture into the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glm::ivec4 mapRegion = GetViewportRegion();
    glViewport(mapRegion.x, mapRegion.y, mapRegion.z, mapRegion.w);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    // Not binding a texture may result in garbage in the render, but that's fine for now
    if (renderSet->find(m_selectedLayer) != renderSet->end())
        glBindTexture(GL_TEXTURE_2D, renderSet->at(m_selectedLayer)->ID);
    else
        std::cout << "No texture to draw" << std::endl;
    viewShader.use();
    viewShader.setMat4("view", camera.view);
    viewShader.setMat4("projection", camera.projection);
    viewShader.setInt("renderTexture", 0);
    viewShader.setInt("isolateChannel", (int)m_isolateChannel);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
void UI::DrawViewportProperties(const RenderSet *const renderSet)
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    glm::ivec4 propertiesRegion = GetViewportPropertiesRegion();
    ImGui::SetNextWindowPos(ImVec2(propertiesRegion.x, propertiesRegion.y));
    ImGui::SetNextWindowSize(ImVec2(propertiesRegion.z, propertiesRegion.w));
    ImGui::Begin("Viewport Properties", &p_open, flags);

    ImGui::PushItemWidth(150.0f);
    if (ImGui::BeginCombo("Layer", getLayerName(m_selectedLayer)))
    {
        for (auto it = renderSet->cbegin(); it != renderSet->cend(); ++it)
        {
            bool isSelected = (m_selectedLayer == it->first);
            if (ImGui::Selectable(getLayerName(it->first), isSelected))
            {
                m_selectedLayer = it->first;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::PushItemWidth(100.0f);
    if (ImGui::BeginCombo("IsolateChannel", getIsolateChannelName(m_isolateChannel)))
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

    if (!m_mapmaker)
    {
        return;
    }

    ImGui::BeginListBox("##Operators");
    for (size_t i = 0; i < m_mapmaker->operators.size(); ++i)
    {
        if (ImGui::Selectable(m_mapmaker->operators[i]->name().c_str(), i == m_selectedOpIndex))
        {
            m_selectedOpIndex = i;
            activeOperatorChanged.emit(i);
        }
    }
    ImGui::EndListBox();
    ImGui::Text("Operator Settings");
    if (m_selectedOpIndex != -1)
    {
        switch (m_mapmaker->operators[m_selectedOpIndex]->type())
        {
        case OP_PERLIN:
            DrawPerlinControls();
            break;
        case OP_VORONOI:
            DrawVoronoiControls();
            break;
        }
    }

    ImGui::End();
}
void UI::DrawPerlinControls()
{
    PerlinNoiseOperator *perlinOp = static_cast<PerlinNoiseOperator *>(m_mapmaker->operators[m_selectedOpIndex]);

    float freq = perlinOp->settings.Get<float>("frequency");
    if (ImGui::SliderFloat("Frequency", &freq, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
        opSettingChanged.emit(m_selectedOpIndex, "frequency", freq);

    glm::ivec2 offset = perlinOp->settings.Get<glm::ivec2>("offset");
    if (ImGui::DragInt2("Offset", (int *)&offset))
        opSettingChanged.emit(m_selectedOpIndex, "offset", offset);
}
void UI::DrawVoronoiControls()
{
    VoronoiNoiseOperator *voronoiOp = static_cast<VoronoiNoiseOperator *>(m_mapmaker->operators[m_selectedOpIndex]);

    glm::ivec2 offset = voronoiOp->settings.Get<glm::ivec2>("offset");
    if (ImGui::DragInt2("Offset", (int *)&offset))
        opSettingChanged.emit(m_selectedOpIndex, "offset", offset);

    float size = voronoiOp->settings.Get<float>("size");
    if (ImGui::SliderFloat("Cell size", &size, 0.1, 1000, "%.3f", ImGuiSliderFlags_Logarithmic))
        opSettingChanged.emit(m_selectedOpIndex, "size", size);

    float skew = voronoiOp->settings.Get<float>("skew");
    if (ImGui::SliderFloat("Cell offset", &skew, 0.0f, 1.0f, "%.3f"))
        opSettingChanged.emit(m_selectedOpIndex, "skew", skew);

    float blend = voronoiOp->settings.Get<float>("blend");
    if (ImGui::SliderFloat("Blend", &blend, 0.0f, 1.0f, "%.3f"))
        opSettingChanged.emit(m_selectedOpIndex, "blend", blend);
}

glm::ivec4 UI::GetViewportRegion() const
{
    float panelWidth = m_width * m_opPropertiesWidthPercent;
    float panelHeight = m_height * m_viewPropertiesHeightPercent;
    // Remember, positions start from (0,0) at top-left
    return glm::ivec4(panelWidth, 0, m_width - panelWidth, m_height - panelHeight);
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
