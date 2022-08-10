#pragma once
#include <string>
#include <vector>

#include "../constants.h"

// Defined in node.h
class Node;

class Connector
{
public:
    enum Type
    {
        INPUT,
        OUTPUT
    };

    Connector(Node *node, Type type, int maxConnections = -1);

    bool connect(Connector *connector);
    bool disconnect(Connector *connector);
    Type type() const;
    size_t numConnections() const;
    int maxConnections() const;
    const Connector *connection(size_t index) const;
    Node *node() const;
    bool isFull() const;
    const std::string &layer() const;
    void setLayer(const std::string &layer);

protected:
    Node *m_node;
    Type m_type;
    int m_maxConnections = -1;
    std::vector<Connector *> m_connected;
    std::string m_layer = DEFAULT_LAYER;
};

class InputConnector : public Connector
{
public:
    InputConnector(Node *node, std::string name, bool required = true);

    const std::string &name() const;
    bool isRequired() const;

protected:
    std::string m_name;
    bool m_required = true;
};

class OutputConnector : public Connector
{
public:
    OutputConnector(Node *node, std::string layerName);
};

struct Connection
{
    InputConnector *input;
    OutputConnector *output;
};