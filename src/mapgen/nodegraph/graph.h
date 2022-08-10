#pragma once
#include <string>
#include <vector>

#include "node.h"

class Graph
{
protected:
    static NodeID lastID;
    std::vector<Node> m_nodes;

public:
    typedef std::vector<Node>::iterator iterator;
    typedef std::vector<Node>::const_iterator const_iterator;

    iterator begin();
    iterator end();
    const_iterator cbegin();
    const_iterator cend();

    NodeID createNode(std::string name);
    bool deleteNode(Node *node);
    Node *node(NodeID nodeID);
    size_t numNodes() const;
};