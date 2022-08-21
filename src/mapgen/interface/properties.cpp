#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "properties.h"

const SettingChoices CHANNEL_CHOICES{
    {"red", 0},
    {"green", 1},
    {"blue", 2},
    {"alpha", 3}};
const float DEFAULT_FLOAT_SLIDER_SPEED = 0.01f;

SettingChoices rangedChannelChoices(const Setting &setting)
{
    SettingChoices choices;
    int min = setting.min<int>();
    int max = setting.max<int>();
    for (auto pair : CHANNEL_CHOICES)
    {
        if (min <= std::get<int>(pair.second) && std::get<int>(pair.second) <= max)
        {
            choices.emplace(pair);
        }
    }
    return choices;
}

ImGuiSliderFlags sliderFlags(const Setting &setting)
{
    ImGuiSliderFlags flags = ImGuiSliderFlags_NoRoundToFormat;
    if (setting.hints() & SettingHint_Logarithmic)
    {
        flags |= ImGuiSliderFlags_Logarithmic;
    }
    return flags;
}

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
    if (node->definesDimensions())
    {
        glm::ivec2 dimensions = node->dimensions();
        if (ImGui::DragInt2("size", (int *)&dimensions))
            nodeSizeChanged.emit(node, dimensions);
    }
    for (auto it = node->settings()->begin(); it != node->settings()->end(); ++it)
    {
        if (it->hasChoices())
        {
            drawSettingChoices(node, *it);
            continue;
        }

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
    if (ImGui::SliderFloat(setting.name().c_str(), &value, setting.min<float>(), setting.max<float>(), "%.3f", sliderFlags(setting)))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawFloat2Setting(Node *node, const Setting &setting)
{
    glm::vec2 value = setting.value<glm::vec2>();
    if (ImGui::DragFloat2(setting.name().c_str(), (float *)&value, DEFAULT_FLOAT_SLIDER_SPEED, setting.min<float>(), setting.max<float>(), "%.3f", sliderFlags(setting)))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawFloat3Setting(Node *node, const Setting &setting)
{
    glm::vec3 value = setting.value<glm::vec3>();
    if (setting.hints() & SettingHint_Color)
    {
        // XXX: Providing the flags doesn't seem to change behaviour but triggers a change every frame
        if (ImGui::ColorEdit3(setting.name().c_str(), (float *)&value)) // ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR
            opSettingChanged.emit(node, setting.name(), value);
        return;
    }

    if (ImGui::DragFloat3(setting.name().c_str(), (float *)&value, DEFAULT_FLOAT_SLIDER_SPEED, setting.min<float>(), setting.max<float>(), "%.3f", sliderFlags(setting)))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawFloat4Setting(Node *node, const Setting &setting)
{
    glm::vec4 value = setting.value<glm::vec4>();
    if (setting.hints() & SettingHint_Color)
    {
        // XXX: Providing the flags doesn't seem to change behaviour but triggers a change every frame
        if (ImGui::ColorEdit4(setting.name().c_str(), (float *)&value)) // ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR
        {
            opSettingChanged.emit(node, setting.name(), value);
        }
        return;
    }

    if (ImGui::DragFloat4(setting.name().c_str(), (float *)&value, DEFAULT_FLOAT_SLIDER_SPEED, setting.min<float>(), setting.max<float>(), "%.3f", sliderFlags(setting)))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawIntSetting(Node *node, const Setting &setting)
{
    int value = setting.value<int>();
    if (setting.hints() & SettingHint_Channel)
    {
        drawChoices(node, setting.name().c_str(), rangedChannelChoices(setting), currentChoice(CHANNEL_CHOICES, value).c_str());
    }
    else
    {
        if (ImGui::InputInt(setting.name().c_str(), &value, setting.min<int>(), setting.max<int>()))
            opSettingChanged.emit(node, setting.name(), value);
    }
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

void Properties::drawSettingChoices(Node *node, const Setting &setting)
{
    drawChoices(node, setting.name().c_str(), setting.choices(), setting.currentChoice().c_str());
}

void Properties::drawChoices(Node *node, const char *name, const SettingChoices &choices, const char *currChoice)
{
    std::ostringstream s;
    s << name << "##Setting" << node->name().c_str();
    if (ImGui::BeginCombo(s.str().c_str(), currChoice))
    {
        for (auto it = choices.cbegin(); it != choices.cend(); ++it)
        {
            if (ImGui::Selectable(it->first.c_str(), it->first == currChoice))
            {
                opSettingChanged.emit(node, name, it->second);
            }
        }
        ImGui::EndCombo();
    }
}
