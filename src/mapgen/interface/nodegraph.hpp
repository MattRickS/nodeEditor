#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../nodegraph/node.h"
#include "../scene.h"
#include "bounds.hpp"
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
    float m_connectorRounding = 0.0f;
    Node *m_selectedNode = nullptr;
    float m_selectionThickness = 1.0f;
    float m_lineThickness = 2.0f;

    const ImU32 COLOR_LINE = IM_COL32(255, 255, 255, 255);
    const ImU32 COLOR_SELECTED = IM_COL32(255, 255, 0, 255);
    const ImU32 COLOR_CONNECTOR = IM_COL32(150, 150, 150, 255);
    const ImU32 COLOR_UNPROCESSED = IM_COL32(100, 100, 100, 255);
    const ImU32 COLOR_PROCESSING = IM_COL32(100, 150, 100, 255);
    const ImU32 COLOR_PROCESSED = IM_COL32(100, 255, 100, 255);
    const ImU32 COLOR_ERROR = IM_COL32(255, 100, 100, 255);

    Nodegraph(Bounds bounds) : Panel(bounds) {}

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

    Bounds nodeBounds(Node *node)
    {
        return {
            glm::vec2(pos()) + node->pos() * m_viewScale + m_viewOffset,
            glm::vec2(pos()) + (node->pos() + node->size()) * m_viewScale + m_viewOffset};
    }

    Bounds connBounds(Connector *conn)
    {
        static glm::vec2 connSize = glm::vec2(15, 10);
        Bounds nodebounds = nodeBounds(conn->node());

        if (conn->type() == Connector::INPUT)
        {
            size_t numInputs = conn->node()->numInputs();
            float inputSpacing = (conn->node()->size().x - numInputs * connSize.x) / float(numInputs + 1);
            return {
                glm::vec2(nodebounds.min.x + inputSpacing, nodebounds.min.y - connSize.y),
                glm::vec2(nodebounds.min.x + inputSpacing + connSize.x, nodebounds.min.y)};
        }
        else
        {
            size_t numOutputs = conn->node()->numOutputs();
            float outputSpacing = (conn->node()->size().x - numOutputs * connSize.x) / float(numOutputs + 1);
            return {
                glm::vec2(nodebounds.min.x + outputSpacing, nodebounds.max.y),
                glm::vec2(nodebounds.min.x + outputSpacing + connSize.x, nodebounds.max.y + connSize.y)};
        }
    }

    void drawNode(ImDrawList *drawList, Node *node)
    {
        if (!node)
        {
            return;
        }

        Bounds bounds = nodeBounds(node);

        for (size_t i = 0; i < node->numInputs(); ++i)
        {
            InputConnector *input = node->input(i);
            Bounds b = connBounds(input);

            for (size_t i = 0; i < input->numConnections(); ++i)
            {
                glm::vec2 p1 = b.center();
                glm::vec2 p2 = connBounds(input->connection(i)).center();
                drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), COLOR_LINE, m_lineThickness);
            }

            drawList->AddRectFilled(ImVec2(b.min.x, b.min.y), ImVec2(b.max.x, b.max.y), COLOR_CONNECTOR, m_connectorRounding);
        }

        for (size_t i = 0; i < node->numOutputs(); ++i)
        {
            Bounds b = connBounds(node->output(i));
            drawList->AddRectFilled(ImVec2(b.min.x, b.min.y), ImVec2(b.max.x, b.max.y), COLOR_CONNECTOR, m_connectorRounding);
        }

        drawList->AddRectFilled(ImVec2(bounds.min.x, bounds.min.y), ImVec2(bounds.max.x, bounds.max.y), nodeColor(node), m_nodeRounding);
        drawList->AddText(ImVec2(bounds.min.x, bounds.min.y), ImColor(0, 0, 0, 255), node->name().c_str());

        if (node == m_selectedNode)
        {
            drawList->AddRect(ImVec2(bounds.min.x, bounds.min.y), ImVec2(bounds.max.x, bounds.max.y),
                              COLOR_SELECTED, m_nodeRounding, ImDrawFlags_Closed, m_selectionThickness);
        }
    }

    void draw()
    {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

        ImGui::SetNextWindowPos(ImVec2(pos().x, pos().y));
        ImGui::SetNextWindowSize(ImVec2(size().x, size().y));
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
