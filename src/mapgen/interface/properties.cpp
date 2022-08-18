#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "properties.h"

Properties::Properties(Bounds bounds) : Panel(bounds) {}

void Properties::setScene(Scene *scene) { m_scene = scene; }
void Properties::setNode(NodeID nodeID) { m_nodeID = nodeID; }
void Properties::draw()
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos(ImVec2(pos().x, pos().y));
    ImGui::SetNextWindowSize(ImVec2(size().x, size().y));
    ImGui::Begin("Mapmaker Properties", &p_open, flags);

    if (m_scene)
    {
        ImGui::Text("Operator Settings");
        Node *node = m_scene->getCurrentGraph()->node(m_nodeID);
        if (node)
        {
            drawNodeSettings(node);
        }

        bool isPaused = m_scene->isPaused();
        if (ImGui::Button(isPaused ? "Play" : "Pause"))
        {
            pauseToggled.emit(!isPaused);
        }
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void Properties::drawNodeSettings(Node *node)
{
    if (!node)
    {
        return;
    }
    for (auto it = node->settings()->begin(); it != node->settings()->end(); ++it)
    {
        switch (it->type())
        {
        case SettingType_Bool:
            drawBoolSetting(node, *it);
            break;
        case SettingType_Float:
            drawFloatSetting(node, *it);
            break;
        case SettingType_Float2:
            drawFloat2Setting(node, *it);
            break;
        case SettingType_Float3:
            drawFloat3Setting(node, *it);
            break;
        case SettingType_Float4:
            drawFloat4Setting(node, *it);
            break;
        case SettingType_Int:
            drawIntSetting(node, *it);
            break;
        case SettingType_Int2:
            drawInt2Setting(node, *it);
            break;
        case SettingType_UInt:
            drawUIntSetting(node, *it);
            break;
        }
    }
}

void Properties::drawBoolSetting(Node *node, const Setting &setting)
{
    bool value = setting.value<bool>();
    if (ImGui::Checkbox(setting.name().c_str(), &value))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawFloatSetting(Node *node, const Setting &setting)
{
    float value = setting.value<float>();
    // TODO: Setting options for min/max
    if (ImGui::SliderFloat(setting.name().c_str(), &value, 0, 100, "%.3f", ImGuiSliderFlags_Logarithmic))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawFloat2Setting(Node *node, const Setting &setting)
{
    glm::vec2 value = setting.value<glm::vec2>();
    if (ImGui::DragFloat2(setting.name().c_str(), (float *)&value))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawFloat3Setting(Node *node, const Setting &setting)
{
    glm::vec3 value = setting.value<glm::vec3>();
    if (ImGui::DragFloat3(setting.name().c_str(), (float *)&value))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawFloat4Setting(Node *node, const Setting &setting)
{
    glm::vec4 value = setting.value<glm::vec4>();
    if (ImGui::DragFloat4(setting.name().c_str(), (float *)&value))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawIntSetting(Node *node, const Setting &setting)
{
    int value = setting.value<int>();
    if (ImGui::InputInt(setting.name().c_str(), &value))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawInt2Setting(Node *node, const Setting &setting)
{
    glm::ivec2 offset = setting.value<glm::ivec2>();
    if (ImGui::DragInt2(setting.name().c_str(), (int *)&offset))
        opSettingChanged.emit(node, setting.name(), offset);
}
void Properties::drawUIntSetting(Node *node, const Setting &setting)
{
    unsigned int value = setting.value<unsigned int>();
    if (ImGui::InputScalar(setting.name().c_str(), ImGuiDataType_U32, &value))
        opSettingChanged.emit(node, setting.name(), value);
}
