#include <algorithm>
#include <string>
#include <vector>

#include "Connector.h"
#include "Node.h"

Connector::Connector(Node *node, Type type, size_t index, const std::string &name, int maxConnections, bool isRequired) : GraphElement({0, 0, 15, 8}), m_node(node), m_type(type), m_index(index), m_name(name), m_maxConnections(maxConnections), m_required(isRequired)
{
}

bool Connector::connect(Connector *connector)
{
    if (connector->m_node == m_node || m_type == connector->type() || isFull() || connector->isFull())
        return false;

    m_connected.push_back(connector);
    connector->m_connected.push_back(this);

    // Connections connect output->input. If connected, the output is
    // unaffected but the input has gained data and must be reevaluated
    if (m_type == Input)
    {
        m_node->setDirty();
    }
    else
    {
        connector->node()->setDirty();
    }
    return true;
}
bool Connector::disconnectConnection(std::vector<Connector *>::reverse_iterator it)
{
    if (it != m_connected.rend())
    {
        Connector *connector = *it;
        // TODO: Check if this returns a copy or if we're modifying the underlying
        //       base - the latter would break disconnectAll
        m_connected.erase(it.base() - 1);
        // Connections connect output->input. If disconnected, the output is
        // unaffected but the input has lost data and must be reevaluated
        if (m_type == Input)
        {
            m_node->setDirty();
        }
        else
        {
            connector->node()->setDirty();
        }
        return connector->disconnect(this);
    }
    return false;
}
bool Connector::disconnect(Connector *connector)
{
    auto it = std::find(m_connected.rbegin(), m_connected.rend(), connector);
    return disconnectConnection(it);
}
void Connector::disconnectAll()
{
    for (auto it = m_connected.rbegin(); it != m_connected.rend(); ++it)
    {
        disconnectConnection(it);
    }
}
Connector::Type Connector::type() const { return m_type; }
size_t Connector::index() const { return m_index; }
size_t Connector::numConnections() const { return m_connected.size(); }
int Connector::maxConnections() const { return m_maxConnections; }
Connector *Connector::connection(size_t index) const { return m_connected[index]; }
Node *Connector::node() const { return m_node; }
bool Connector::isFull() const { return m_maxConnections > 0 && m_connected.size() >= (size_t)m_maxConnections; }
bool Connector::isRequired() const { return m_required; }
const std::string &Connector::name() const { return m_name; }
Bounds Connector::bounds() const
{
    // XXX: Node ref is still pointing to the old Node when copied/movied
    // Return the bounds relative to the node
    Bounds nodeBounds = node()->bounds();
    if (type() == Connector::Input)
    {
        size_t numInputs = node()->numInputs();
        float inputSpacing = (index() + 1) * nodeBounds.size().x / float(numInputs + 1) - m_bounds.size().x * 0.5f;
        return {
            glm::vec2(nodeBounds.min().x + inputSpacing, nodeBounds.min().y - m_bounds.size().y),
            glm::vec2(nodeBounds.min().x + inputSpacing + m_bounds.size().x, nodeBounds.min().y)};
    }
    else
    {
        size_t numOutputs = node()->numOutputs();
        float outputSpacing = (index() + 1) * nodeBounds.size().x / float(numOutputs + 1) - m_bounds.size().x * 0.5f;
        return {
            glm::vec2(nodeBounds.min().x + outputSpacing, nodeBounds.max().y),
            glm::vec2(nodeBounds.min().x + outputSpacing + m_bounds.size().x, nodeBounds.max().y + m_bounds.size().y)};
    }
}
