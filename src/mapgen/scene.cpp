#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "constants.h"
#include "nodegraph/iterators.h"
#include "nodegraph/node.h"
#include "renders.h"
#include "scene.h"
#include "util.h"

Scene::Scene(unsigned int width, unsigned int height) : m_context("Scene")
{
    m_settings.registerInt2(SCENE_SETTING_IMAGE_SIZE, {width, height});
}
Scene::~Scene()
{
    if (!m_stopped.load())
    {
        stopProcessing();
    }
}

const Context *Scene::context() const { return &m_context; }
glm::ivec2 Scene::defaultImageSize() const { return m_settings.getInt2(SCENE_SETTING_IMAGE_SIZE); }
void Scene::setDefaultImageSize(glm::ivec2 imageSize)
{
    m_settings.get(SCENE_SETTING_IMAGE_SIZE)->set(imageSize);
    bool changed = false;
    for (auto it = getCurrentGraph()->begin(); it != getCurrentGraph()->end(); ++it)
    {
        if (it->recalculateImageSize(&m_settings))
        {
            it->setDirty(true);
            changed = true;
        }
    }
    if (changed)
    {
        setDirty();
    }
}

NodeID Scene::createNode(std::string nodeType)
{
    return getCurrentGraph()->createNode(nodeType);
}
Graph *Scene::getCurrentGraph() { return &m_graph; }
Node *Scene::getCurrentNode() { return m_currNode; }
Node *Scene::getViewNode() { return m_viewNode.load(); }
Node *Scene::getSelectedNode()
{
    for (auto it = m_graph.begin(); it != m_graph.end(); ++it)
    {
        if (it->hasSelectFlag(SelectFlag_Select))
        {
            return &(*it);
        }
    }
    return nullptr;
}
Node *Scene::getNode(NodeID nodeID) { return m_graph.node(nodeID); }

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
    if (!node)
    {
        return;
    }
    node->setSelectFlag(SelectFlag_View);

    // TODO: m_currNode is not thread safe currently
    m_currNode = calculateCurrentNode();
    LOG_DEBUG(
        "View node changed to '%s', current calculated as: '%s'",
        (node ? node->name().c_str() : "None"),
        (m_currNode ? m_currNode->name().c_str() : "None"));
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
    LOG_INFO("Waiting on thread to stop");
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
    LOG_INFO("Starting process");

    m_context.use();
    makeQuad(&m_quadVAO);

    // Ensure the main thread hasn't requested the thread be paused
    // m_stopped is checked last so that a request to stop the thread is
    // immediate when it wakes up
    while (waitToProcess() && !m_stopped.load())
    {
        // Ensure all state changes are processed first and reevaluate state
        if (maybeCleanNodes())
        {
            LOG_DEBUG("Cleaned up state");
            m_currNode = calculateCurrentNode();
            continue;
        }

        // If the current node is invalid or doesn't need processing, wait for changes
        if (!m_currNode || m_currNode->state() == State::Error || (m_currNode == m_viewNode.load() && m_currNode->state() == State::Processed))
        {
            setAwake(false);
            continue;
        }

        // If the node completed processing, advance to the next node
        if (m_currNode->processStep(&m_settings))
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
    LOG_DEBUG("Idle thread, going to sleep");
    std::lock_guard<std::mutex> guard(m_mutex);
    m_awake = idle;
    m_condition.notify_one();
}

Node *Scene::calculateCurrentNode() const
{
    // Everything is up to date, nothing to do
    if (!m_viewNode.load() || m_viewNode.load()->state() == State::Processed)
    {
        LOG_DEBUG("Current view node is fully processed, no new current node");
        return nullptr;
    }

    // Depth-first search up from target and return the deepest unprocessed node
    // (ie, prev node when the iteration has to move to a sibling or shallower node)
    DepthIterator it{m_viewNode.load(), GraphDirection_Upstream, IteratorFlags_SkipProcessed};
    DepthIterator last = it++;
    for (; it != DepthIterator(); last = it++)
    {
        if (it.depth() <= last.depth())
        {
            break;
        }
    }

    LOG_DEBUG("New current node calculated as '%s'", last->name().c_str());
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
            LOG_DEBUG("Cleaning node '%s' and downstream", it->name().c_str());
            // All nodes after (and including) a dirty node must be reset
            for (DepthIterator it2{&(*it), GraphDirection_Downstream}; it2 != DepthIterator(); it2++)
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
