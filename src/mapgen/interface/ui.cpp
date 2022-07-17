
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

UI::UI(unsigned int width, unsigned int height, const char *name) : Window(name, width, height),
                                                                    viewShader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/texture.fs")
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UI::SetPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
void UI::Draw(const RenderSet *const renderSet)
{
    // Draws the texture into the window slot
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glm::ivec4 mapRegion = GetViewportRegion();
    glViewport(mapRegion.x, mapRegion.y, mapRegion.z, mapRegion.w);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderSet->GetLayer(LAYER_HEIGHTMAP)->ID);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    viewShader.use();
    viewShader.setMat4("view", camera.view);
    viewShader.setMat4("projection", camera.projection);
    viewShader.setInt("renderTexture", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Draws the UI around it
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    glm::ivec4 propertiesRegion = GetPropertiesRegion();
    ImGui::SetNextWindowPos(ImVec2(propertiesRegion.x, propertiesRegion.y));
    ImGui::SetNextWindowSize(ImVec2(propertiesRegion.z, propertiesRegion.w));
    ImGui::Begin("Mapmaker UI", &p_open, flags);

    if (m_pixelPreview)
    {
        ImGui::BeginDisabled();
        ImGui::ColorEdit4("Pixel Value", (float *)&(*m_pixelPreview).value);
        ImGui::InputInt2("Pixel Position", (int *)&(*m_pixelPreview).pos);
        ImGui::EndDisabled();
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

glm::ivec4 UI::GetViewportRegion()
{
    float uiWidth = m_width * m_uiScreenWidthPercent;
    return glm::ivec4(uiWidth, 0, m_width - uiWidth, m_height);
}
glm::ivec4 UI::GetPropertiesRegion()
{
    return glm::ivec4(0, 0, m_width * m_uiScreenWidthPercent, m_height);
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
