
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../renders.h"
#include "../shader.h"
#include "window.h"
#include "ui.h"

const float CAM_NEAR = 0.1f;
const float CAM_FAR = 100.0f;

unsigned int UI::uiWidth() { return m_width * m_uiScreenWidthPercent; }
void UI::UpdateView()
{
    glm::vec3 pos = glm::vec3(transformOffset, -1.0f);
    view = glm::mat4(1.0f);
    view = glm::translate(view, pos);
}
void UI::UpdateProjection()
{
    glm::ivec4 viewportRegion = GetMapViewportRegion();
    float hAperture = (float)viewportRegion.z / (float)viewportRegion.w;
    std::cout << hAperture << std::endl;
    static float vAperture = 1.0f;
    projection = glm::ortho(-hAperture * focal, hAperture * focal, -vAperture * focal, vAperture * focal, CAM_NEAR, CAM_FAR);
}

void UI::OnMouseMoved(double xpos, double ypos)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    // TODO: transform the position to get the pixel pos on the map
    int imagePosX = xpos - uiWidth();
    if (imagePosX >= 0)
    {
        // glfw measures y from top-left, image positions are referenced from
        // bot-left, invert the y-axis
        mapPosChanged.emit(imagePosX, m_height - ypos);
    }

    if (isPanning)
    {
        glm::vec2 cursorPos = CursorPos();
        glm::vec2 offset = cursorPos - lastCursorPos;
        // Invert y-axis as openGL is inverse
        transformOffset += glm::vec2(offset.x / m_width, -offset.y / m_height) * 2.0f;
        lastCursorPos = cursorPos;
        UpdateView();
    }
}

void UI::OnMouseButtonChanged(int button, int action, int mods)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            lastCursorPos = CursorPos();
            isPanning = true;
        }
        else if (action == GLFW_RELEASE)
        {
            isPanning = false;
        }
    }
}

void UI::OnMouseScrolled(double xoffset, double yoffset)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    focal *= (1.0f - yoffset * 0.1f);
    UpdateProjection();
}

void UI::FuckSake(int width, int height)
{
    UpdateProjection();
}

UI::UI(unsigned int width, unsigned int height, const char *name) : Window(name, width, height),
                                                                    viewShader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/texture.fs")
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    UpdateView();
    UpdateProjection();
    sizeChanged.connect(this, &UI::FuckSake);
}

void UI::SetPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
void UI::Draw(const RenderSet *const renderSet)
{
    // Draws the texture into the window slot
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glm::ivec4 mapRegion = GetMapViewportRegion();
    glViewport(mapRegion.x, mapRegion.y, mapRegion.z, mapRegion.w);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderSet->GetLayer(LAYER_HEIGHTMAP)->ID);
    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    viewShader.use();
    viewShader.setMat4("view", view);
    viewShader.setMat4("projection", projection);
    viewShader.setInt("renderTexture", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Draws the UI around it
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(uiWidth(), m_height));
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
glm::ivec4 UI::GetMapViewportRegion()
{
    return glm::ivec4(uiWidth(), 0, m_width - uiWidth(), m_height);
}
