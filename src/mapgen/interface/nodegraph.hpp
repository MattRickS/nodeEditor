#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../nodegraph/node.h"
#include "../scene.h"
#include "panel.hpp"
#include "signal.hpp"

class Nodegraph : public Panel
{
public:
    Signal<Node *> selectedNodeChanged;

    Scene *m_scene;
    float m_viewScale = 1.0f;
    glm::vec2 m_viewOffset{0, 0};
    float m_nodeRounding = 0.0f;
    Node *m_selectedNode = nullptr;

    const ImU32 COLOR_UNPROCESSED = IM_COL32(100, 100, 100, 255);
    const ImU32 COLOR_PROCESSING = IM_COL32(100, 150, 100, 255);
    const ImU32 COLOR_PROCESSED = IM_COL32(100, 255, 100, 255);
    const ImU32 COLOR_ERROR = IM_COL32(255, 100, 100, 255);

    Nodegraph(glm::ivec2 pos, glm::ivec2 size) : Panel(pos, size) {}

    Node *getSelectedNode() const { return m_selectedNode; }
    void setSelectedNode(Node *node)
    {
        m_selectedNode = node;
        selectedNodeChanged.emit(node);
    }
    void setScene(Scene *scene) { m_scene = scene; }
    void pan(glm::vec2 offset) { m_viewOffset += offset; }
    void zoom(float scale) { m_viewScale *= scale; }

    ImU32 nodeColor(const Node *node) const
    {
        switch (node->state())
        {
        case State::Unprocessed:
            return COLOR_UNPROCESSED;
        case State::Preprocessing:
        case State::Processing:
            return COLOR_PROCESSING;
        case State::Processed:
            return COLOR_PROCESSED;
        case State::Error:
            return COLOR_ERROR;
        default:
            return COLOR_UNPROCESSED;
        }
    }

    void drawNode(ImDrawList *drawList, const Node *node)
    {
        glm::vec2 boundsMin = glm::vec2(m_pos) + node->pos() * m_viewScale + m_viewOffset;
        glm::vec2 boundsMax = glm::vec2(m_pos) + (node->pos() + node->size()) * m_viewScale + m_viewOffset;

        // Literally just copying shit in and will then see what's needed
        drawList->AddRectFilled(ImVec2(boundsMin.x, boundsMin.y), ImVec2(boundsMax.x, boundsMax.y), nodeColor(node), m_nodeRounding);
        drawList->AddText(ImVec2(boundsMin.x, boundsMin.y), ImColor(0, 0, 0, 255), node->name().c_str());
    }

    void draw()
    {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

        ImGui::SetNextWindowPos(ImVec2(m_pos.x, m_pos.y));
        ImGui::SetNextWindowSize(ImVec2(m_size.x, m_size.y));
        ImGui::Begin("Nodegraph", nullptr, flags);

        if (m_scene)
        {
            float fontScale = ImGui::GetCurrentWindow()->FontWindowScale;
            ImGui::SetWindowFontScale(m_viewScale);
            ImDrawList *drawList = ImGui::GetWindowDrawList();

            Graph *graph = m_scene->getCurrentGraph();
            for (auto it = graph->begin(); it != graph->end(); ++it)
            {
                drawNode(drawList, &(*it));
            }

            ImGui::SetWindowFontScale(fontScale);
        }

        ImGui::End();
    }
};
