
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
    glActiveTexture(GL_TEXTURE0);
    // Not binding a texture may result in garbage in the render, but that's fine for now
    if (renderSet->find(m_selectedLayer) != renderSet->end())
        glBindTexture(GL_TEXTURE_2D, renderSet->at(m_selectedLayer)->ID);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    viewShader.use();
    viewShader.setMat4("view", camera.view);
    viewShader.setMat4("projection", camera.projection);
    viewShader.setInt("renderTexture", 0);
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

    ImGui::PushItemWidth(350.0f);
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

    if (m_pixelPreview)
    {
        ImGui::BeginDisabled();

        ImGui::SameLine();
        ImGui::PushItemWidth(250.0f);
        ImGui::ColorEdit4("##PixelColor", (float *)&(*m_pixelPreview).value);
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
        case OP_TERRAIN_GEN:
            PerlinNoiseOperator *noiseOp = static_cast<PerlinNoiseOperator *>(m_mapmaker->operators[m_selectedOpIndex]);

            float freq = noiseOp->settings.Get<float>("frequency");
            if (ImGui::SliderFloat("Frequency", &freq, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
                opSettingChanged.emit(m_mapmaker->operators[m_selectedOpIndex], "frequency", freq);

            glm::ivec2 offset = noiseOp->settings.Get<glm::ivec2>("offset");
            if (ImGui::DragInt2("Offset", (int *)&offset))
                opSettingChanged.emit(m_mapmaker->operators[m_selectedOpIndex], "offset", offset);

            break;
        }
    }

    ImGui::End();
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
