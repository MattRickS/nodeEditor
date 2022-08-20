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

const glm::vec2 CONNECTOR_SIZE = glm::vec2(15, 10);

const ImU32 COLOR_TEXT = IM_COL32(125, 125, 125, 255);
const ImU32 COLOR_HOVER = IM_COL32(100, 255, 255, 255);
const ImU32 COLOR_LINE = IM_COL32(255, 255, 255, 255);
const ImU32 COLOR_SELECTED = IM_COL32(255, 255, 0, 255);
const ImU32 COLOR_VIEW = IM_COL32(255, 0, 255, 255);
const ImU32 COLOR_CONNECTOR = IM_COL32(150, 150, 150, 255);
const ImU32 COLOR_CONNECTOR_OPTIONAL = IM_COL32(100, 100, 100, 255);
const ImU32 COLOR_UNPROCESSED = IM_COL32(80, 80, 80, 255);
const ImU32 COLOR_PROCESSING = IM_COL32(100, 150, 100, 255);
const ImU32 COLOR_PROCESSED = IM_COL32(100, 255, 100, 255);
const ImU32 COLOR_ERROR = IM_COL32(255, 100, 100, 255);

Nodegraph::Nodegraph(Bounds bounds) : Panel(bounds) {}

void Nodegraph::setScene(Scene *scene) { m_scene = scene; }

void Nodegraph::pan(glm::vec2 offset) { m_viewOffset += offset; }
void Nodegraph::scaleFromPos(const glm::vec2 screenPos, float scale)
{
    glm::vec2 worldPos = screenToWorldPos(screenPos);
    m_viewScale *= scale;
    m_viewOffset += screenPos - worldToScreenPos(worldPos);
}
void Nodegraph::fitBounds(const Bounds &worldBounds)
{
    // Pad the screen space to keep slightly zoomed out, and pad the offset so it's an even spacing
    static float screenPadding = 100.0f;
    glm::vec2 scale = (bounds().size() - screenPadding) / worldBounds.size();
    m_viewScale = std::min(scale.x, scale.y);
    m_viewOffset = -worldBounds.pos() * m_viewScale + screenPadding * 0.5f;
}

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

void Nodegraph::startNodeSelection(glm::vec2 screenPos)
{
    m_inputText[0] = '\0';
    m_shouldDrawTextbox = true;
    m_inputTextboxPos = ImVec2(screenPos.x, screenPos.y);
}
bool Nodegraph::hasNodeSelection() const
{
    return m_shouldDrawTextbox;
}
void Nodegraph::finishNodeSelection()
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
    if (connector->type() == Connector::Input && !static_cast<const InputConnector *>(connector)->isRequired())
    {
        return COLOR_CONNECTOR_OPTIONAL;
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

    // Connectors text is half size
    float fontScale = ImGui::GetCurrentWindow()->FontWindowScale;
    ImGui::SetWindowFontScale(fontScale * 0.5f);

    for (size_t i = 0; i < node->numInputs(); ++i)
    {
        InputConnector *conn = node->input(i);
        Bounds b = graphElementBounds(conn);

        for (size_t j = 0; j < conn->numConnections(); ++j)
        {
            glm::vec2 p1 = b.center();
            glm::vec2 p2 = graphElementBounds(conn->connection(j)).center();
            drawList->AddLine(ImVec2(p1.x, p1.y), ImVec2(p2.x, p2.y), COLOR_LINE, m_lineThickness);
        }

        drawList->AddRectFilled(ImVec2(b.min().x, b.min().y), ImVec2(b.max().x, b.max().y), connColor(conn), m_connectorRounding);
        drawList->AddText(ImVec2(b.min().x, b.min().y - 10 * m_viewScale), COLOR_TEXT, conn->name().c_str());
    }

    for (size_t i = 0; i < node->numOutputs(); ++i)
    {
        Connector *conn = node->output(i);
        Bounds b = graphElementBounds(conn);
        drawList->AddRectFilled(ImVec2(b.min().x, b.min().y), ImVec2(b.max().x, b.max().y), connColor(conn), m_connectorRounding);
    }

    ImGui::SetWindowFontScale(fontScale);

    drawList->AddRectFilled(ImVec2(bounds.min().x, bounds.min().y), ImVec2(bounds.max().x, bounds.max().y), nodeColor(node), m_nodeRounding);
    drawList->AddText(ImVec2(bounds.min().x, bounds.min().y), COLOR_TEXT, node->name().c_str());

    if (node && node->hasSelectFlag(SelectFlag_Select))
    {
        drawList->AddRect(ImVec2(bounds.min().x, bounds.min().y), ImVec2(bounds.max().x, bounds.max().y),
                          COLOR_SELECTED, m_nodeRounding, ImDrawFlags_Closed, m_selectionThickness);
    }
}

void Nodegraph::drawNodeSelection()
{

    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;

    ImGui::SetNextWindowPos(m_inputTextboxPos);
    ImGui::SetNextWindowSize({165, 300});
    ImGui::Begin("NodeLookup", nullptr, flags);

    // Ensure the input gets focus, but not if currently selecting one of the nodes
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
    {
        ImGui::SetKeyboardFocusHere(0);
    }
    ImGui::PushItemWidth(150);
    if (ImGui::InputText("##NodeSelection", m_inputText, MAX_NODE_SEARCH_SIZE, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
    {
        LOG_DEBUG("Text entered: %s", m_inputText);
    }
    ImGui::PopItemWidth();

    for (auto it = Op::OperatorRegistry::begin(); it != Op::OperatorRegistry::end(); ++it)
    {
        if (containsTextCaseInsensitive(it->c_str(), m_inputText) && ImGui::Selectable(it->c_str()))
        {
            newNodeRequested.emit(glm::ivec2(m_inputTextboxPos.x, m_inputTextboxPos.y), *it);
            finishNodeSelection();
        }
    }

    ImGui::End();
}

void Nodegraph::draw()
{
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoInputs;

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
            drawNodeSelection();
        }

        ImGui::SetWindowFontScale(fontScale);
    }

    // There's a 3px horizontal padding for some reason.
    drawList->AddRect(ImVec2(m_bounds.min().x + 3, m_bounds.min().y), ImVec2(m_bounds.max().x - 3, m_bounds.max().y),
                      COLOR_LINE, m_nodeRounding, ImDrawFlags_Closed, 2.0f);

    ImGui::End();
}
