#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../constants.h"
#include "../bounds.hpp"
#include "../nodegraph/node.h"
#include "../scene.h"
#include "panel.hpp"
#include "signal.hpp"

#include "nodegraph.h"

Nodegraph::Nodegraph(Bounds bounds) : Panel(bounds) {}

Node *Nodegraph::getSelectedNode() const { return m_selectedNode; }
void Nodegraph::setSelectedNode(Node *node)
{
    m_selectedNode = node;
    selectedNodeChanged.emit(node);
}
void Nodegraph::setScene(Scene *scene) { m_scene = scene; }
void Nodegraph::pan(glm::vec2 offset) { m_viewOffset += offset; }
void Nodegraph::zoom(float scale) { m_viewScale *= scale; }

void Nodegraph::startConnection(Connector *conn)
{
    m_startConnector = conn;
    m_currentLineStart = graphElementBounds(conn).center();
    m_currentLineEnd = m_currentLineStart;
}
void Nodegraph::updateConnection(glm::vec2 pos)
{
    m_currentLineEnd = pos;
}
void Nodegraph::finishConnection()
{
    m_startConnector = nullptr;
}
Connector *Nodegraph::activeConnection()
{
    return m_startConnector;
}

void Nodegraph::startTextInput(glm::vec2 pos)
{
    m_shouldDrawTextbox = true;
    m_inputText = "";
    glm::vec2 panelPos = pos - bounds().pos();
    m_inputTextboxPos = ImVec2(panelPos.x, panelPos.y);
}
void Nodegraph::finishTextInput()
{
    m_shouldDrawTextbox = false;
}

glm::vec2 Nodegraph::screenToWorldPos(glm::vec2 screenPos)
{
    return (screenPos - glm::vec2(pos()) - m_viewOffset) / m_viewScale;
}

glm::vec2 Nodegraph::worldToScreenPos(glm::vec2 worldPos)
{
    return glm::vec2(pos()) + worldPos * m_viewScale + m_viewOffset;
}

ImU32 Nodegraph::nodeColor(const Node *node) const
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

ImU32 Nodegraph::connColor(const Connector *connector) const
{
    if (connector->hasSelectFlag(SelectFlag_Hover))
    {
        return COLOR_HOVER;
    }
    return COLOR_CONNECTOR;
}

/* GraphElement bounds within the screen window, respecting view transforms */
Bounds Nodegraph::graphElementBounds(GraphElement *el)
{
    return {worldToScreenPos(el->bounds().min()), worldToScreenPos(el->bounds().max())};
}

void Nodegraph::drawNode(ImDrawList *drawList, Node *node)
{
    if (!node)
    {
        return;
    }

    Bounds bounds = graphElementBounds(node);

    if (node->hasSelectFlag(SelectFlag_View))
    {
        drawList->AddRect(ImVec2(bounds.min().x, bounds.min().y), ImVec2(bounds.max().x, bounds.max().y),
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

        drawList->AddRectFilled(ImVec2(b.min().x, b.min().y), ImVec2(b.max().x, b.max().y), connColor(conn), m_connectorRounding);
    }

    for (size_t i = 0; i < node->numOutputs(); ++i)
    {
        Connector *conn = node->output(i);
        Bounds b = graphElementBounds(conn);
        drawList->AddRectFilled(ImVec2(b.min().x, b.min().y), ImVec2(b.max().x, b.max().y), connColor(conn), m_connectorRounding);
    }

    drawList->AddRectFilled(ImVec2(bounds.min().x, bounds.min().y), ImVec2(bounds.max().x, bounds.max().y), nodeColor(node), m_nodeRounding);
    drawList->AddText(ImVec2(bounds.min().x, bounds.min().y), ImColor(0, 0, 0, 255), node->name().c_str());

    if (node == m_selectedNode)
    {
        drawList->AddRect(ImVec2(bounds.min().x, bounds.min().y), ImVec2(bounds.max().x, bounds.max().y),
                          COLOR_SELECTED, m_nodeRounding, ImDrawFlags_Closed, m_selectionThickness);
    }
}

void Nodegraph::drawTextBox()
{
    ImGui::SetCursorPos(m_inputTextboxPos);
    ImGui::PushItemWidth(150.0f);
    // TODO: allow inputting text to filter options
    if (ImGui::BeginCombo("##NewNodeInput", m_inputText.c_str()))
    {
        for (auto it = Op::OperatorRegistry::begin(); it != Op::OperatorRegistry::end(); ++it)
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

void Nodegraph::draw()
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
    drawList->AddRect(ImVec2(m_bounds.min().x + 3, m_bounds.min().y), ImVec2(m_bounds.max().x - 3, m_bounds.max().y),
                      COLOR_LINE, m_nodeRounding, ImDrawFlags_Closed, 2.0f);

    ImGui::End();
}
