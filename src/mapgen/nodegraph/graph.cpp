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

void Graph::createNode(NodeID nodeID, const std::string &nodeType)
{
    m_nodes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(nodeID),
        std::forward_as_tuple(nodeID, Op::OperatorRegistry::create(nodeType)));
}
NodeID Graph::createNode(const std::string &nodeType)
{
    NodeID nodeID = ++lastID;
    createNode(nodeID, nodeType);
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
void Graph::clear()
{
    m_nodes.clear();
}

bool Graph::serialize(Serializer *serializer) const
{
    bool ok = serializer->startObject(KEY_NODES);
    for (auto it = cbegin(); it != cend(); ++it)
    {
        ok = ok && serializer->startObject(it->type());
        ok = ok && it->serialize(serializer);
        ok = ok && serializer->finishObject();
    }
    ok = ok && serializer->finishObject();

    ok = ok && serializer->startObject(KEY_INPUTS);
    for (auto it = cbegin(); it != cend(); ++it)
    {
        for (size_t i = 0; i < it->numInputs(); ++i)
        {
            if (it->input(i)->numConnections() == 0)
            {
                continue;
            }
            ok = ok && serializer->writePropertyInt(KEY_INPUT, it->id());
            ok = ok && serializer->writeInt(it->input(i)->index());
            ok = ok && serializer->writeInt(it->input(i)->connection(0)->node()->id());
            ok = ok && serializer->writeInt(it->input(i)->connection(0)->index());
        }
    }
    ok = ok && serializer->finishObject();

    return ok;
}

bool Graph::deserialize(Deserializer *deserializer)
{
    bool ok = true;
    std::string property;
    while (ok && deserializer->readProperty(property))
    {
        if (property == KEY_NODES)
        {
            ok = ok && deserializer->startReadObject();
            std::string nodeType;
            while (ok && deserializer->readProperty(nodeType))
            {
                // Create a temporary node to deserialize into
                createNode(0, nodeType);
                Node &node = m_nodes.at(NodeID(0));
                ok = ok && deserializer->startReadObject();
                ok = ok && node.deserialize(deserializer);
                ok = ok && deserializer->finishReadObject();
                // Correct the node's key or remove it
                if (ok)
                {
                    auto handler = m_nodes.extract(0);
                    handler.key() = node.id();
                    m_nodes.insert(std::move(handler));
                }
                else
                {
                    m_nodes.erase(0);
                }
            }
            ok = ok && deserializer->finishReadObject();
            if (ok)
            {
                // Map is ordered, highest ID is the last ID
                lastID = m_nodes.rbegin()->first;
            }
        }
        else if (property == KEY_INPUTS)
        {
            ok = ok && deserializer->startReadObject();
            while (ok && deserializer->readProperty(property) && property == KEY_INPUT)
            {
                int src, input, tgt, output;
                ok = ok && deserializer->readInt(src);
                ok = ok && deserializer->readInt(input);
                ok = ok && deserializer->readInt(tgt);
                ok = ok && deserializer->readInt(output);
                if (ok)
                {
                    node(NodeID(src))->input(input)->connect(node(NodeID(tgt))->output(output));
                }
            }
            ok = ok && deserializer->finishReadObject();
        }
    }
    return ok;
}
