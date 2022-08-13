#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "nodegraph/iterators.h"
#include "nodegraph/node.h"
#include "operators/invert.hpp" // Not used, directly, but must be included to be added to registry
#include "operators/perlin.hpp"
#include "operators/voronoi.hpp"
#include "renders.h"
#include "scene.h"
#include "util.hpp"

Scene::Scene(unsigned int width, unsigned int height) : m_width(width), m_height(height), context("Scene")
{
    m_graph.createNode("PerlinNoise");
    NodeID n1 = m_graph.createNode("VoronoiNoise");
    m_graph.node(n1)->setPos(glm::vec2(200, 100));
    NodeID n2 = m_graph.createNode("InvertOp");
    m_graph.node(n2)->setPos(glm::vec2(300, 200));
    // TODO: Can't rely on pointers into the graph nodes vector as it may reallocate the memory
    //       What works around that? Using indexes? Not ideal either as nodes could be deleted.
    //       Probably best to use nodeIDs everywhere and a map as storage for quick lookup
    m_graph.node(n2)->input(0)->connect(m_graph.node(n1)->output(0));
}
Scene::~Scene()
{
    if (!m_stopped.load())
    {
        stopProcessing();
    }
}

unsigned int Scene::Width() const { return m_width.load(); }
unsigned int Scene::Height() const { return m_height.load(); }

Graph *Scene::getCurrentGraph() { return &m_graph; }
Node *Scene::getCurrentNode() { return m_currNode; }
Node *Scene::getViewNode() { return m_viewNode.load(); }

void Scene::setDirty()
{
    m_isDirty = true;
    setAwake(true);
}

void Scene::setViewNode(Node *node)
{
    if (m_viewNode.load())
    {
        m_viewNode.load()->clearSelectFlag(SelectFlag_View);
    }
    m_viewNode = node;
    node->setSelectFlag(SelectFlag_View);

    // TODO: m_currNode is not thread safe currently
    m_currNode = calculateCurrentNode();
    std::cout << "View node changed to " << (node ? node->name() : "None") << ", current calculated as: " << (m_currNode ? m_currNode->name() : "None") << std::endl;
    // TODO: Check if actually needs to wake up
    setAwake(true);
}

bool Scene::startProcessing()
{
    if (m_thread)
    {
        return false;
    }

    m_stopped = false;
    m_thread = std::make_unique<std::thread>(std::bind(&Scene::process, this));
    return true;
}

void Scene::stopProcessing()
{
    // Ensure the thread is woken up so that it can stop the loop
    m_stopped = true;
    setAwake(true);
    setPaused(false);
    std::cout << "Waiting on thread to stop" << std::endl;
    m_thread->join();
}

void Scene::setPaused(bool paused)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_paused = paused;

    // TODO: Can't use the lock guard twice, needs a refactor before doing this
    // If there were any state changes the idle flag should have been disabled,
    // but disable as a safety check - it will only perform a single iteration
    // of the process loop before idling again.
    // if (!paused)
    // {
    //     setIdle(false);
    // }

    m_condition.notify_one();
}

bool Scene::isPaused()
{
    return m_paused.load();
}

// =============================================================================
// Private

bool Scene::waitToProcess()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, std::bind(&Scene::isActive, this));
    return true;
}

void Scene::process()
{
    // TODO: Tidy up this thread initialisation
    // TODO: Switch to smart pointers
    context.use();
    std::cout << "Initialising thread" << std::endl;

    makeQuad(&quadVAO);

    std::cout << "Starting process" << std::endl;
    // Ensure the main thread hasn't requested the thread be paused
    // m_stopped is checked last so that a request to stop the thread is
    // immediate when it wakes up
    while (waitToProcess() && !m_stopped.load())
    {
        // Ensure all state changes are processed first and reevaluate state
        if (maybeCleanNodes())
        {
            std::cout << "Cleaned up state" << std::endl;
            m_currNode = calculateCurrentNode();
            continue;
        }

        // If the current node is invalid or doesn't need processing, wait for changes
        if (!m_currNode || m_currNode->state() == State::Error || (m_currNode == m_viewNode && m_currNode->state() == State::Processed))
        {
            setAwake(false);
            continue;
        }

        // If the node completed processing, advance to the next node
        if (m_currNode->processStep())
        {
            m_currNode = calculateCurrentNode();
        }
    }
}

bool Scene::isActive()
{
    // Stop on step requests advancing a single operator step and should occur
    // even when paused but without clearing the paused state.
    return m_processOne.load() || (!m_paused.load() && m_awake.load());
}

void Scene::setAwake(bool idle)
{
    std::cout << "Idle thread, going to sleep" << std::endl;
    std::lock_guard<std::mutex> guard(m_mutex);
    m_awake = idle;
    m_condition.notify_one();
}

Node *Scene::calculateCurrentNode() const
{
    // Everything is up to date, nothing to do
    if (m_viewNode.load()->state() == State::Processed)
    {
        std::cout << "Current view node is fully processed, no new current node" << std::endl;
        return nullptr;
    }

    // Depth-first search up from target and return the deepest unprocessed node
    // (ie, prev node when the iteration has to move to a sibling or shallower node)
    DepthIterator it{m_viewNode, UPSTREAM, SKIP_PROCESSED};
    DepthIterator last = it++;
    for (; it != DepthIterator(); last = it++)
    {
        if (it.depth() <= last.depth())
        {
            break;
        }
    }

    return &(*last);
}

bool Scene::maybeCleanNodes()
{
    if (!m_isDirty.load())
    {
        return false;
    }
    m_isDirty = false;
    for (auto it = m_graph.begin(); it != m_graph.end(); ++it)
    {
        if (it->isDirty())
        {
            std::cout << "Cleaning node " << it->name() << " and downstream" << std::endl;
            // All nodes after (and including) a dirty node must be reset
            for (DepthIterator it2{&(*it), DOWNSTREAM}; it2 != DepthIterator(); it2++)
            {
                it2->reset();
            }
        }
    }
    return true;
}

bool Scene::processOne()
{
    if (isActive() || !m_viewNode.load())
    {
        return false;
    }

    // If already fully processed, return
    if (m_viewNode.load()->state() == State::Processed)
    {
        return false;
    }

    m_processOne = true;
    setAwake(true);
    return true;
}
