#pragma once
#include <string>
#include <map>

#include "node.h"

class Graph
{
protected:
    static NodeID lastID;
    std::map<NodeID, Node> m_nodes;

    // TODO: make this templated so can be reused for settings/graph
    typedef std::map<NodeID, Node>::iterator pair_iterator;

    void validateUniqueSetting(const std::string &name) const;
    void validateKeyExists(const std::string &name) const;

public:
    class value_iterator : public pair_iterator
    {
    public:
        value_iterator() : pair_iterator() {}
        value_iterator(pair_iterator it) : pair_iterator(it) {}
        Node *operator->() { return &(pair_iterator::operator->()->second); }
        Node &operator*() { return pair_iterator::operator*().second; }
    };

    typedef std::vector<Node>::iterator iterator;
    typedef std::vector<Node>::const_iterator const_iterator;

    value_iterator begin();
    value_iterator end();

    NodeID createNode(std::string name);
    bool deleteNode(NodeID nodeID);
    Node *node(NodeID nodeID);
    size_t numNodes() const;
};