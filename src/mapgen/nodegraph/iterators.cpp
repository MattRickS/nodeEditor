#include <memory>

#include "node.h"
#include "iterators.h"

DepthIterator::DepthIterator() {}
DepthIterator::DepthIterator(Node *node, GraphDirection direction, IteratorFlags flags, int depth) : m_node(node), m_direction(direction), m_flags(flags), m_depth(depth) {}

int DepthIterator::depth() const
{
    auto it = deepestIterator();
    if (!it)
    {
        return -1;
    }
    return it->m_depth;
}
GraphDirection DepthIterator::direction() const { return m_direction; }
IteratorFlags DepthIterator::flags() const { return m_flags; }

Node *DepthIterator::operator->()
{
    return currentNode();
}
Node &DepthIterator::operator*()
{
    return *(deepestIterator()->m_node);
}
DepthIterator &DepthIterator::operator++()
{
    advance();
    return *this;
}
DepthIterator DepthIterator::operator++(int)
{
    DepthIterator it = *this;
    advance();
    return it;
}
bool operator==(const DepthIterator &a, const DepthIterator &b) { return a.currentNode() == b.currentNode(); }
bool operator!=(const DepthIterator &a, const DepthIterator &b) { return a.currentNode() != b.currentNode(); }

const DepthIterator *DepthIterator::deepestIterator() const
{
    if (m_isExhausted || !m_node)
    {
        return nullptr;
    }

    const DepthIterator *curr = this;
    while (curr->m_next)
    {
        curr = curr->m_next.get();
    }
    return curr;
}
Node *DepthIterator::currentNode() const
{
    auto it = deepestIterator();
    if (!it)
    {
        return nullptr;
    }
    return it->m_node;
}
bool DepthIterator::advance()
{
    if (m_isExhausted)
    {
        return false;
    }
    if (m_next && m_next->advance())
    {
        return true;
    }

    // Increment through connections / connectors until a node is found
    size_t numConnectors = m_direction == UPSTREAM ? m_node->numInputs() : m_node->numOutputs();
    while ((size_t)m_connectorIndex < numConnectors)
    {
        Connector *conn;
        if (m_direction == UPSTREAM)
        {
            conn = m_node->input(m_connectorIndex);
        }
        else
        {
            conn = m_node->output(m_connectorIndex);
        }
        while (size_t(++m_connectionIndex) < conn->numConnections())
        {
            // Found a connection, wrap in an iterator and report success
            Node *node = conn->connection(m_connectionIndex)->node();
            if (m_flags & SKIP_PROCESSED && node->state() == State::Processed)
            {
                continue;
            }
            m_next = std::make_shared<DepthIterator>(node, m_direction, m_flags, m_depth + 1);
            return true;
        }
        // Ran out of connections, move to next connector and reset connection index
        ++m_connectorIndex;
        m_connectionIndex = -1;
    }
    // Ran out of connectors, node is exhausted
    m_isExhausted = true;
    return false;
}
