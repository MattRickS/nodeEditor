#include <string>
#include <vector>

#include "node.h"
#include "../operator.h"

#include "graph.h"

NodeID Graph::lastID = 1;

Graph::iterator Graph::begin() { return m_nodes.begin(); }
Graph::iterator Graph::end() { return m_nodes.end(); }
Graph::const_iterator Graph::cbegin() { return m_nodes.cbegin(); }
Graph::const_iterator Graph::cend() { return m_nodes.cend(); }

NodeID Graph::createNode(std::string name)
{
    m_nodes.emplace_back(++lastID, OperatorRegistry::create(name));
    // How does the node get initialised with width/height?
    // Can width height just be a metadata property passed in to process along with the current renderSet?
    // Textures could then be generated/resized when starting to process
    return m_nodes.back().id();
}
bool Graph::deleteNode(Node *node)
{
    if (!node)
    {
        return false;
    }
    auto it = std::find(m_nodes.cbegin(), m_nodes.cend(), *node);
    if (it != m_nodes.end())
    {
        m_nodes.erase(it);
        return true;
    }
    return false;
}
Node *Graph::node(NodeID nodeID)
{
    auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&nodeID](const Node &n)
                           { return n.id() == nodeID; });
    if (it != m_nodes.end())
    {
        return &(*it);
    }
    return nullptr;
}
size_t Graph::numNodes() const { return m_nodes.size(); }
