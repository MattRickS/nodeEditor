#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

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

    Nodegraph(Bounds bounds);

    Node *getSelectedNode() const;
    void setSelectedNode(Node *node);
    void setScene(Scene *scene);

    void pan(glm::vec2 offset);
    void zoom(float scale);

    void startConnection(Connector *conn);
    void updateConnection(glm::vec2 pos);
    void finishConnection();
    Connector *activeConnection();

    void startTextInput(glm::vec2 pos);
    void finishTextInput();

    glm::vec2 screenToWorldPos(glm::vec2 screenPos);
    glm::vec2 worldToScreenPos(glm::vec2 worldPos);

    /* GraphElement bounds within the screen window, respecting view transforms */
    Bounds graphElementBounds(GraphElement *el);

    void draw();

protected:
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

    ImU32 nodeColor(const Node *node) const;
    ImU32 connColor(const Connector *connector) const;

    void drawNode(ImDrawList *drawList, Node *node);
    void drawTextBox();
};
