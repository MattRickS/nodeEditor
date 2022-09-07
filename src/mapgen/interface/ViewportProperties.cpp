#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../gl/RenderSetOperator.h"
#include "../gl/Texture.h"
#include "../util.h"
#include "ViewportProperties.h"

ViewportProperties::ViewportProperties(Window *window, Bounds bounds) : Panel(window, bounds) {}

void ViewportProperties::setChannel(Channel channel) { m_channel = channel; }
void ViewportProperties::setLayer(const std::string &layer) { m_layer = layer; }
void ViewportProperties::setPixelPreview(PixelPreview *preview) { m_pixelPreview = preview; }
void ViewportProperties::setScene(Scene *scene) { m_scene = scene; }

std::string ViewportProperties::selectedLayer() const { return m_layer; }

void ViewportProperties::draw()
{
    static bool p_open = NULL;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos(ImVec2(pos().x, pos().y));
    ImGui::SetNextWindowSize(ImVec2(size().x, size().y));
    ImGui::Begin("Viewport Properties", &p_open, flags);

    const RenderSet_c *renderSet = nullptr;
    if (m_scene)
    {
        Node *viewNode = m_scene->getViewNode();
        if (viewNode)
        {
            Op::RenderSetOperator const *op = dynamic_cast<Op::RenderSetOperator const *>(viewNode->op());
            if (op)
            {
                renderSet = op->renderSet();
            }
        }
    }
    ImGui::PushItemWidth(150.0f);
    if (ImGui::BeginCombo("##Layer", renderSet ? m_layer.c_str() : "--"))
    {
        if (renderSet)
        {
            for (auto it = renderSet->cbegin(); it != renderSet->cend(); ++it)
            {
                bool isSelected = (m_layer == it->first);
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
    if (ImGui::BeginCombo("##Channel", renderSet ? getChannelName(m_channel) : "--"))
    {
        for (int channel = Channel_All; channel != Channel_Last; ++channel)
        {
            bool isSelected = (m_channel == channel);
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
