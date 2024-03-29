#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include "../Bounds.hpp"
#include "../nodegraph/Connector.h"
#include "../nodegraph/Node.h"
#include "../nodegraph/Scene.h"
#include "Panel.hpp"
#include "Signal.hpp"
#include "Window.h"

constexpr size_t MAX_NODE_SEARCH_SIZE = 64;

class Nodegraph : public Panel
{
public:
    Signal<glm::ivec2, std::string> newNodeRequested;

    Nodegraph(Window *window, Bounds bounds);

    void setScene(Scene *scene);

    void pan(glm::vec2 offset);
    void scaleFromPos(const glm::vec2 screenPos, float scale);
    void fitBounds(const Bounds &worldBounds);

    void startConnection(Connector *conn);
    void updateConnection(glm::vec2 pos);
    void finishConnection();
    Connector *activeConnection();

    void startNodeSelection(glm::vec2 screenPos);
    bool hasNodeSelection() const;
    void finishNodeSelection();

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
    char m_inputText[MAX_NODE_SEARCH_SIZE]{""};

    ImU32 nodeColor(const Node *node) const;
    ImU32 connColor(const Connector *connector) const;

    void drawNode(ImDrawList *drawList, Node *node);
    void drawNodeSelection();
};
