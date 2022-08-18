#include <string>
#include <vector>

#include "node.h"
#include "../operator.h"

#include "graph.h"

NodeID Graph::lastID = 0;

Graph::value_iterator Graph::begin() { return m_nodes.begin(); }
Graph::value_iterator Graph::end() { return m_nodes.end(); }
Graph::const_value_iterator Graph::cbegin() const { return m_nodes.cbegin(); }
Graph::const_value_iterator Graph::cend() const { return m_nodes.cend(); }
Graph::reverse_value_iterator Graph::rbegin() { return m_nodes.rbegin(); }
Graph::reverse_value_iterator Graph::rend() { return m_nodes.rend(); }

NodeID Graph::createNode(std::string name)
{
    NodeID nodeID = ++lastID;
    m_nodes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(nodeID),
        std::forward_as_tuple(nodeID, Op::OperatorRegistry::create(name)));
    return nodeID;
}
bool Graph::deleteNode(NodeID nodeID)
{
    auto it = m_nodes.find(nodeID);
    if (it != m_nodes.end())
    {
        m_nodes.erase(it);
        return true;
    }
    return false;
}
Node *Graph::node(NodeID nodeID)
{
    auto it = m_nodes.find(nodeID);
    if (it != m_nodes.end())
    {
        return &it->second;
    }
    return nullptr;
}
size_t Graph::numNodes() const { return m_nodes.size(); }

Bounds Graph::bounds() const
{
    if (numNodes() == 0)
    {
        return {};
    }
    auto it = cbegin();
    Bounds bounds = it->bounds();
    for (it++; it != cend(); ++it)
    {
        bounds.expand(it->bounds());
    }
    return bounds;
}
