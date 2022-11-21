#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "GraphElement.h"

// Defined in node.h
class Node;

class Connector : public GraphElement
{
public:
    enum Type
    {
        Input,
        Output
    };

    Connector(Node *node, Type type, size_t index, const std::string &name, int maxConnections = -1, bool isRequired = true);

    bool connect(Connector *connector);
    bool disconnect(Connector *connector);
    void disconnectAll();
    Type type() const;
    size_t index() const;
    size_t numConnections() const;
    int maxConnections() const;
    Connector *connection(size_t index) const;
    Node *node() const;
    bool isFull() const;
    bool isRequired() const;
    const std::string &name() const;
    Bounds bounds() const override;

protected:
    Node *m_node;
    Type m_type;
    size_t m_index;
    std::string m_name;
    int m_maxConnections = -1;
    std::vector<Connector *> m_connected;
    bool m_required = true;

    bool disconnectConnection(std::vector<Connector *>::reverse_iterator it);

    friend class Node;
};
