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

Scene::Scene() : m_context("Scene")
{
    registerSettings(&m_settings);
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
Graph const *Scene::getCurrentGraph() const { return &m_graph; }
Node *Scene::getCurrentNode() { return m_currNode; }
Node *Scene::getViewNode()
{
    for (auto it = getCurrentGraph()->begin(); it != getCurrentGraph()->end(); ++it)
    {
        if (it->hasSelectFlag(SelectFlag_View))
        {
            return &(*it);
        }
    }
    return nullptr;
}
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
    setInternalPause(false);
}

void Scene::clear()
{
    m_currNode = nullptr;
    m_graph.clear();
}

void Scene::setViewNode(Node *node)
{
    Node *viewNode = getViewNode();
    if (viewNode)
    {
        viewNode->clearSelectFlag(SelectFlag_View);
    }
    if (!node)
    {
        return;
    }
    node->setSelectFlag(SelectFlag_View);

    // TODO: m_currNode is not thread safe currently
    m_currNode = calculateCurrentNode(node);
    LOG_DEBUG(
        "View node changed to '%s', current calculated as: '%s'",
        (node ? node->type().c_str() : "None"),
        (m_currNode ? m_currNode->type().c_str() : "None"));

    if (node->state() != State::Processed)
    {
        setInternalPause(false);
    }
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
    setInternalPause(false);
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
            m_currNode = calculateCurrentNode(getViewNode());
            continue;
        }

        Node *viewNode = getViewNode();
        // If the current node is invalid or doesn't need processing, wait for changes
        if (!m_currNode || !viewNode || m_currNode->state() == State::Error || (m_currNode == viewNode && m_currNode->state() == State::Processed))
        {
            setInternalPause(true);
            continue;
        }

        // If the node completed processing, advance to the next node
        if (m_currNode->processStep(&m_settings))
        {
            m_currNode = calculateCurrentNode(viewNode);
        }
    }
}

bool Scene::isActive()
{
    // Stop on step requests advancing a single operator step and should occur
    // even when paused but without clearing the paused state.
    return m_processOne.load() || !(m_paused.load() || m_internalPause.load());
}

void Scene::setInternalPause(bool paused)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    bool wasPaused = m_internalPause.exchange(paused);
    if (wasPaused == paused)
    {
        return;
    }

    m_condition.notify_one();
    if (paused)
    {
        LOG_DEBUG("Thread going to sleep");
    }
    else
    {
        LOG_DEBUG("Thread waking up");
    }
}

Node *Scene::calculateCurrentNode(Node *viewNode) const
{
    // Everything is up to date, nothing to do
    if (!viewNode || viewNode->state() == State::Processed)
    {
        LOG_DEBUG("Current view node is fully processed, no new current node");
        return nullptr;
    }

    // Depth-first search up from target and return the deepest unprocessed node
    // (ie, prev node when the iteration has to move to a sibling or shallower node)
    DepthIterator it{viewNode, GraphDirection_Upstream, IteratorFlags_SkipProcessed};
    DepthIterator last = it++;
    for (; it != DepthIterator(); last = it++)
    {
        if (it.depth() <= last.depth())
        {
            break;
        }
    }

    LOG_DEBUG("New current node calculated as '%s'", last->type().c_str());
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
            LOG_DEBUG("Cleaning node '%s' and downstream", it->type().c_str());
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
    Node *viewNode = getViewNode();
    if (isActive() || !viewNode)
    {
        return false;
    }

    // If already fully processed, return
    if (viewNode->state() == State::Processed)
    {
        return false;
    }

    m_processOne = true;
    setInternalPause(false);
    return true;
}

void Scene::registerSettings(Settings *settings) const
{
    settings->registerInt2(SCENE_SETTING_IMAGE_SIZE, {DEFAULT_WIDTH, DEFAULT_HEIGHT});
}

bool Scene::serialize(Serializer *serializer) const
{
    bool ok = serializer->startObject(KEY_SETTINGS);
    ok = ok && m_settings.serialize(serializer, false); // Write ALL settings
    ok = ok && serializer->finishObject();

    ok = ok && serializer->startObject(KEY_GRAPH);
    ok = ok && getCurrentGraph()->serialize(serializer);
    ok = ok && serializer->finishObject();
    return ok;
}

bool Scene::deserialize(Deserializer *deserializer)
{
    setInternalPause(true);

    Graph graph;
    // Must register default settings so that settings can be set. Also prevents
    // errors when deserializing old content that may be missing modern settings.
    Settings settings;
    registerSettings(&settings);

    bool ok = true;
    std::string property;
    while (ok && deserializer->readProperty(property))
    {
        if (property == KEY_SETTINGS)
        {
            ok = ok && deserializer->startReadObject();
            ok = ok && settings.deserialize(deserializer);
            ok = ok && deserializer->finishReadObject();
        }
        else if (property == KEY_GRAPH)
        {
            ok = ok && deserializer->startReadObject();
            ok = ok && graph.deserialize(deserializer);
            ok = ok && deserializer->finishReadObject();
        }
    }

    if (ok)
    {
        m_settings = std::move(settings);
        m_graph = std::move(graph);
        m_currNode = calculateCurrentNode(getViewNode());
    }

    setInternalPause(false);
    return ok;
}
