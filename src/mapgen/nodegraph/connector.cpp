#include <string>
#include <vector>

#include "connector.h"
#include "node.h"

Connector::Connector(Node *node, Type type, int maxConnections) : m_node(node), m_type(type), m_maxConnections(maxConnections) {}

bool Connector::connect(Connector *connector)
{
    if (!m_node || !connector->node() || connector->m_node == m_node || m_type == connector->type() || isFull() || connector->isFull())
        return false;

    m_connected.push_back(connector);
    connector->m_connected.push_back(this);

    // Connections connect output->input. If connected, the output is
    // unaffected but the input has gained data and must be reevaluated
    if (m_type == INPUT)
    {
        m_node->setDirty();
    }
    else
    {
        connector->node()->setDirty();
    }
    return true;
}
bool Connector::disconnect(Connector *connector)
{
    if (!m_node || !connector->node())
        return false;

    auto it = std::find(m_connected.begin(), m_connected.end(), connector);
    if (it != m_connected.end())
    {
        m_connected.erase(it);
        // Connections connect output->input. If disconnected, the output is
        // unaffected but the input has lost data and must be reevaluated
        if (m_type == INPUT)
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
Connector::Type Connector::type() const { return m_type; }
size_t Connector::numConnections() const { return m_connected.size(); }
int Connector::maxConnections() const { return m_maxConnections; }
const Connector *Connector::connection(size_t index) const { return m_connected[index]; }
Node *Connector::node() const { return m_node; }
bool Connector::isFull() const { return m_maxConnections > 0 && m_connected.size() >= (size_t)m_maxConnections; }
const std::string &Connector::layer() const { return m_layer; }
void Connector::setLayer(const std::string &layer) { m_layer = layer; }

InputConnector::InputConnector(Node *node, std::string name, bool required) : Connector(node, INPUT, 1), m_name(name), m_required(required) {}

const std::string &InputConnector::name() const { return m_name; }
bool InputConnector::isRequired() const { return m_required; }

OutputConnector::OutputConnector(Node *node, std::string layerName) : Connector(node, OUTPUT)
{
    m_layer = layerName;
}
