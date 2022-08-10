#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../bounds.hpp"
#include "../nodegraph/node.h"
#include "../scene.h"
#include "panel.hpp"
#include "signal.hpp"

class Nodegraph : public Panel
{
public:
    Signal<Node *> selectedNodeChanged;
    Signal<std::string> newNodeRequested;

    Scene *m_scene;
    float m_viewScale = 1.0f;
    glm::vec2 m_viewOffset{0, 0};
    float m_nodeRounding = 0.0f;
    float m_connectorRounding = 0.0f;
    Node *m_selectedNode = nullptr;
    float m_selectionThickness = 1.0f;
    float m_lineThickness = 3.0f;
    float m_viewThickness = 8.0f;

    // Drawing a new connection
    Connector *m_startConnector = nullptr;
    glm::vec2 m_currentLineStart;
    glm::vec2 m_currentLineEnd;

    // New node input
    bool m_shouldDrawTextbox = false;
    ImVec2 m_inputTextboxPos;
    std::string m_inputText;

    const glm::vec2 CONNECTOR_SIZE = glm::vec2(15, 10);

    const ImU32 COLOR_HOVER = IM_COL32(100, 255, 255, 255);
    const ImU32 COLOR_LINE = IM_COL32(255, 255, 255, 255);
    const ImU32 COLOR_SELECTED = IM_COL32(255, 255, 0, 255);
    const ImU32 COLOR_VIEW = IM_COL32(255, 0, 255, 255);
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

    void startConnection(Connector *conn)
    {
        m_startConnector = conn;
        m_currentLineStart = graphElementBounds(conn).center();
        m_currentLineEnd = m_currentLineStart;
    }
    void updateConnection(glm::vec2 pos)
    {
        m_currentLineEnd = pos;
    }
    void finishConnection()
    {
        m_startConnector = nullptr;
    }
    Connector *activeConnection()
    {
        return m_startConnector;
    }

    void startTextInput(glm::vec2 pos)
    {
        m_shouldDrawTextbox = true;
        m_inputText = "";
        glm::vec2 panelPos = pos - bounds().pos();
        m_inputTextboxPos = ImVec2(panelPos.x, panelPos.y);
    }
    void finishTextInput()
    {
        m_shouldDrawTextbox = false;
    }

    glm::vec2 screenToWorldPos(glm::vec2 screenPos)
    {
        return (screenPos - glm::vec2(pos()) - m_viewOffset) / m_viewScale;
    }

    glm::vec2 worldToScreenPos(glm::vec2 worldPos)
    {
        return glm::vec2(pos()) + worldPos * m_viewScale + m_viewOffset;
    }

    ImU32 nodeColor(const Node *node) const
    {
        if (node->hasSelectFlag(SelectFlag_Hover))
        {
            return COLOR_HOVER;
        }
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

    ImU32 connColor(const Connector *connector) const
    {
        if (connector->hasSelectFlag(SelectFlag_Hover))
        {
            return COLOR_HOVER;
        }
        return COLOR_CONNECTOR;
    }

    /* GraphElement bounds within the screen window, respecting view transforms */
    Bounds graphElementBounds(GraphElement *el)
    {
        return {worldToScreenPos(el->bounds().min), worldToScreenPos(el->bounds().max)};
    }

    void drawNode(ImDrawList *drawList, Node *node)
    {
        if (!node)
        {
            return;
        }

        Bounds bounds = graphElementBounds(node);

        if (node->hasSelectFlag(SelectFlag_View))
        {
            drawList->AddRect(ImVec2(bounds.min.x, bounds.min.y), ImVec2(bounds.max.x, bounds.max.y),
                              COLOR_VIEW, m_nodeRounding, ImDrawFlags_Closed, m_viewThickness);
        }

        for (size_t i = 0; i < node->numInputs(); ++i)
        {
            Connector *conn = node->input(i);
            Bounds b = graphElementBounds(conn);

            for (size_t j = 0; j < conn->numConnections(); ++j)
            {
                glm::vec2 p1 = b.center();
                glm::vec2 p2 = graphElementBounds(conn->connection(j)).center();
                drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), COLOR_LINE, m_lineThickness);
            }

            drawList->AddRectFilled(ImVec2(b.min.x, b.min.y), ImVec2(b.max.x, b.max.y), connColor(conn), m_connectorRounding);
        }

        for (size_t i = 0; i < node->numOutputs(); ++i)
        {
            Connector *conn = node->output(i);
            Bounds b = graphElementBounds(conn);
            drawList->AddRectFilled(ImVec2(b.min.x, b.min.y), ImVec2(b.max.x, b.max.y), connColor(conn), m_connectorRounding);
        }

        drawList->AddRectFilled(ImVec2(bounds.min.x, bounds.min.y), ImVec2(bounds.max.x, bounds.max.y), nodeColor(node), m_nodeRounding);
        drawList->AddText(ImVec2(bounds.min.x, bounds.min.y), ImColor(0, 0, 0, 255), node->name().c_str());

        if (node == m_selectedNode)
        {
            drawList->AddRect(ImVec2(bounds.min.x, bounds.min.y), ImVec2(bounds.max.x, bounds.max.y),
                              COLOR_SELECTED, m_nodeRounding, ImDrawFlags_Closed, m_selectionThickness);
        }
    }

    void drawTextBox()
    {
        ImGui::SetCursorPos(m_inputTextboxPos);
        ImGui::PushItemWidth(150.0f);
        // TODO: allow inputting text to filter options
        if (ImGui::BeginCombo("##NewNodeInput", m_inputText.c_str()))
        {
            for (auto it = OperatorRegistry::begin(); it != OperatorRegistry::end(); ++it)
            {
                bool isSelected = (m_inputText == *it);
                if (ImGui::Selectable(it->c_str(), isSelected))
                {
                    newNodeRequested.emit(*it);
                    finishTextInput();
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
    }

    void draw()
    {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;

        ImGui::SetNextWindowPos(ImVec2(pos().x, pos().y));
        ImGui::SetNextWindowSize(ImVec2(size().x, size().y));
        ImGui::Begin("Nodegraph", nullptr, flags);

        ImDrawList *drawList = ImGui::GetWindowDrawList();

        if (m_scene)
        {
            float fontScale = ImGui::GetCurrentWindow()->FontWindowScale;
            ImGui::SetWindowFontScale(m_viewScale);

            if (m_startConnector)
            {
                drawList->AddLine(ImVec2(m_currentLineStart.x, m_currentLineStart.y),
                                  ImVec2(m_currentLineEnd.x, m_currentLineEnd.y),
                                  COLOR_LINE,
                                  m_lineThickness);
            }

            Graph *graph = m_scene->getCurrentGraph();
            for (auto it = graph->begin(); it != graph->end(); ++it)
            {
                drawNode(drawList, &(*it));
            }

            if (m_shouldDrawTextbox)
            {
                drawTextBox();
            }

            ImGui::SetWindowFontScale(fontScale);
        }

        // There's a 3px horizontal padding for some reason.
        drawList->AddRect(ImVec2(m_bounds.min.x + 3, m_bounds.min.y), ImVec2(m_bounds.max.x - 3, m_bounds.max.y),
                          COLOR_LINE, m_nodeRounding, ImDrawFlags_Closed, 2.0f);

        ImGui::End();
    }
};
