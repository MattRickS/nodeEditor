#pragma once
#include <memory>

#include "node.h"

enum GraphDirection
{
    UPSTREAM,
    DOWNSTREAM,
};

enum IteratorFlags
{
    NO_FLAGS = 0,
    SKIP_PROCESSED = 1 << 0,
};

class DepthIterator
{
public:
    DepthIterator();
    DepthIterator(Node *node, GraphDirection direction = UPSTREAM, IteratorFlags flags = NO_FLAGS, int depth = 0);

    int depth() const;
    GraphDirection direction() const;
    IteratorFlags flags() const;

    Node *operator->();
    Node &operator*();
    DepthIterator &operator++();
    DepthIterator operator++(int);

    friend bool operator==(const DepthIterator &a, const DepthIterator &b);
    friend bool operator!=(const DepthIterator &a, const DepthIterator &b);

private:
    Node *m_node = nullptr;
    GraphDirection m_direction = UPSTREAM;
    IteratorFlags m_flags = NO_FLAGS;
    int m_depth = 0;
    // Using a shared ptr to handle the lifetime of recursive iterators when an iterator
    // in the chain is copied - such as some monster using it++ instead of ++it
    std::shared_ptr<DepthIterator> m_next = nullptr;
    int m_connectorIndex = 0;
    int m_connectionIndex = -1;
    bool m_isExhausted = false;

    const DepthIterator *deepestIterator() const;
    Node *currentNode() const;
    bool advance();
};
