#include <sstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Properties.h"

const SettingChoices CHANNEL_CHOICES{
    {"red", 0},
    {"green", 1},
    {"blue", 2},
    {"alpha", 3}};
const float MAX_FLOAT_SLIDER_SPEED = 1.0f;

// TODO: Figure out why the CMake include for this source file isn't working
namespace ImGui
{
    // ImGui::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    IMGUI_API bool InputText(const char *label, std::string *str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void *user_data = NULL);
    IMGUI_API bool InputTextMultiline(const char *label, std::string *str, const ImVec2 &size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void *user_data = NULL);
    IMGUI_API bool InputTextWithHint(const char *label, const char *hint, std::string *str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void *user_data = NULL);
}

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

float sliderSpeed(float min, float max)
{
    return std::min(MAX_FLOAT_SLIDER_SPEED, min + (max - min) * 0.01f);
}

Properties::Properties(Window *window, Bounds bounds) : Panel(window, bounds) {}

void Properties::setScene(RenderScene *scene) { m_scene = scene; }
void Properties::draw()
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos(ImVec2(pos().x, pos().y));
    ImGui::SetNextWindowSize(ImVec2(size().x, size().y));
    ImGui::Begin("Mapmaker Properties", &p_open, flags);

    if (m_scene)
    {
        ImGui::TextUnformatted("Scene Settings");
        glm::ivec2 sceneImageSize = m_scene->defaultImageSize();
        if (ImGui::DragInt2("defaultImageSize", (int *)&sceneImageSize))
            sceneSizeChanged.emit(sceneImageSize);

        ImGui::Separator();

        ImGui::TextUnformatted("Operator Settings");
        Node *node = m_scene->getSelectedNode();
        if (node)
        {
            drawNodeSettings(node);
        }

        drawGlobalProperties();
    }

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void Properties::drawGlobalProperties()
{
    ImGui::SetCursorPos({ImGui::GetCursorPosX(), m_bounds.max().y - 70});
    ImGui::Separator();
    ImGui::InputText("Filepath##Scene", &m_saveLoadPath, ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button("New"))
    {
        newSceneRequested.emit();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        saveRequested.emit(m_saveLoadPath);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        loadRequested.emit(m_saveLoadPath);
    }
    ImGui::SameLine();
    bool isPaused = m_scene->isPaused();
    if (ImGui::Button(isPaused ? "Play" : "Pause"))
    {
        pauseToggled.emit(!isPaused);
    }
}

void Properties::drawNodeSettings(Node *node)
{
    if (!node)
    {
        return;
    }

    for (auto it = node->settings()->cbegin(); it != node->settings()->cend(); ++it)
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
        case SettingType_String:
            drawStringSetting(node, *it);
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
    float min = setting.min<float>();
    float max = setting.max<float>();
    glm::vec2 value = setting.value<glm::vec2>();
    if (ImGui::DragFloat2(setting.name().c_str(), (float *)&value, sliderSpeed(min, max), min, max, "%.3f", sliderFlags(setting)))
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

    float min = setting.min<float>();
    float max = setting.max<float>();
    if (ImGui::DragFloat3(setting.name().c_str(), (float *)&value, sliderSpeed(min, max), min, max, "%.3f", sliderFlags(setting)))
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

    float min = setting.min<float>();
    float max = setting.max<float>();
    if (ImGui::DragFloat4(setting.name().c_str(), (float *)&value, sliderSpeed(min, max), min, max, "%.3f", sliderFlags(setting)))
        opSettingChanged.emit(node, setting.name(), value);
}
void Properties::drawIntSetting(Node *node, const Setting &setting)
{
    int value = setting.value<int>();
    if (setting.hints() & SettingHint_Channel)
    {
        drawChoices(node, setting.name().c_str(), rangedChannelChoices(setting), currentChoice(CHANNEL_CHOICES, value).c_str());
    }
    else if (setting.hints() & SettingHint_ChannelMask)
    {
        // TODO: bitset?
        ChannelMask mask = ChannelMask(value);
        bool red = bool(mask & ChannelMask_Red);
        bool green = bool(mask & ChannelMask_Green);
        bool blue = bool(mask & ChannelMask_Blue);
        bool alpha = bool(mask & ChannelMask_Alpha);

        using namespace std::string_literals;

        bool modified = ImGui::Checkbox(("r##"s + setting.name()).c_str(), &red);
        ImGui::SameLine();
        modified |= ImGui::Checkbox(("g##"s + setting.name()).c_str(), &green);
        ImGui::SameLine();
        modified |= ImGui::Checkbox(("b##"s + setting.name()).c_str(), &blue);
        ImGui::SameLine();
        modified |= ImGui::Checkbox(("a##"s + setting.name()).c_str(), &alpha);
        ImGui::SameLine();
        if (modified)
        {
            int newMask = ChannelMask_None;
            if (red)
                newMask |= ChannelMask_Red;
            if (green)
                newMask |= ChannelMask_Green;
            if (blue)
                newMask |= ChannelMask_Blue;
            if (alpha)
                newMask |= ChannelMask_Alpha;
            opSettingChanged.emit(node, setting.name(), newMask);
        }

        ImGui::TextUnformatted(setting.name().c_str());
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
void Properties::drawStringSetting(Node *node, const Setting &setting)
{
    std::string value = setting.value<std::string>();
    if (ImGui::InputText(setting.name().c_str(), &value, ImGuiInputTextFlags_EnterReturnsTrue))
        opSettingChanged.emit(node, setting.name(), value);
}

void Properties::drawSettingChoices(Node *node, const Setting &setting)
{
    drawChoices(node, setting.name().c_str(), setting.choices(), setting.currentChoice().c_str());
}

void Properties::drawChoices(Node *node, const char *name, const SettingChoices &choices, const char *currChoice)
{
    std::ostringstream s;
    s << name << "##Setting" << node->type().c_str();
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
