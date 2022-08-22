#pragma once
#include <string>
#include <map>

#include "node.h"

class Graph
{
public:
    // TODO: make this templated so can be reused for settings/graph
    typedef std::map<NodeID, Node>::iterator pair_iterator;
    typedef std::map<NodeID, Node>::const_iterator const_iterator;
    typedef std::map<NodeID, Node>::reverse_iterator reverse_iterator;

    class value_iterator : public pair_iterator
    {
    public:
        value_iterator() : pair_iterator() {}
        value_iterator(pair_iterator it) : pair_iterator(it) {}
        Node *operator->() { return &(pair_iterator::operator->()->second); }
        Node &operator*() { return pair_iterator::operator*().second; }
    };

    class const_value_iterator : public const_iterator
    {
    public:
        const_value_iterator() : const_iterator() {}
        const_value_iterator(const_iterator it) : const_iterator(it) {}
        const Node *operator->() const { return &(const_iterator::operator->()->second); }
        const Node &operator*() const { return const_iterator::operator*().second; }
    };

    class reverse_value_iterator : public reverse_iterator
    {
    public:
        reverse_value_iterator() : reverse_iterator() {}
        reverse_value_iterator(reverse_iterator it) : reverse_iterator(it) {}
        Node *operator->() { return &(reverse_iterator::operator->()->second); }
        Node &operator*() { return reverse_iterator::operator*().second; }
    };

    value_iterator begin();
    value_iterator end();
    const_value_iterator cbegin() const;
    const_value_iterator cend() const;
    reverse_value_iterator rbegin();
    reverse_value_iterator rend();

    NodeID createNode(std::string name, glm::ivec2 dimensions);
    bool deleteNode(NodeID nodeID);
    Node *node(NodeID nodeID);
    size_t numNodes() const;
    Bounds bounds() const;

protected:
    static NodeID lastID;
    std::map<NodeID, Node> m_nodes;

    void validateUniqueSetting(const std::string &name) const;
    void validateKeyExists(const std::string &name) const;
};