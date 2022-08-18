#pragma once
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "context.hpp"
#include "nodegraph/graph.h"
#include "operator.h"
#include "renders.h"

/*
Scene owns no textures, each node owns the textures it generates.
*/
class Scene
{
public:
    Scene(unsigned int width, unsigned int height);
    ~Scene();

    const Context *context() const;
    unsigned int Width() const;
    unsigned int Height() const;

    // Gets the graph currently being processed
    Graph *getCurrentGraph();
    // Gets the node that is currently rendering
    Node *getCurrentNode();
    // Gets the node that the scene is trying to render
    Node *getViewNode();
    // Gets the first selected node
    Node *getSelectedNode();
    // Gets a node from the scene by ID
    Node *getNode(NodeID nodeID);

    void setDirty();
    /*
    Sets what operator the thread will process up to.
    If the target is already processed, no new processing is performed.
    */
    void setViewNode(Node *node);
    /*
    Starts the thread processing. Returns false if thread is already started.
    */
    bool startProcessing();
    /*
    Terminates the thread.
    */
    void stopProcessing();
    /*
    Sets the thread state to pause once it has completed it's current operation
    */
    void setPaused(bool paused);
    /*
    Whether or not the thread is marked as paused.
    The thread might still be processing it's last operation.
    */
    bool isPaused();
    /*
    Processes the current/next operator once.

    If already active, has no effect.
    If paused, thread is awoken for one step, then remains paused.
    If not paused but already processed to the target operator, advances to the
    next operator (if any) and fully processes it. If only one step of the next
    operator is required, pause first.

    Returns true if there is anything to process, otherwise false.
    */
    bool processOne();

protected:
    Context m_context;
    std::atomic<unsigned int> m_width;
    std::atomic<unsigned int> m_height;
    GLuint m_quadVAO;
    Graph m_graph;

    // Thread variables. Lock is required for non-atomic states and the `stopped`
    // condition variable for pausing the thread.
    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;

    // State (threaded)
    std::atomic<Node *> m_viewNode = nullptr;
    std::atomic<bool> m_resizing = false;
    std::atomic<bool> m_paused = false;
    std::atomic<bool> m_awake = true;
    std::atomic<bool> m_stopped = false;
    std::atomic<bool> m_processOne = false;
    std::atomic<bool> m_isDirty = false;
    // This is only ever read and written to by the thread
    Node *m_currNode = nullptr;

    /*
    Checks if any changes were made that would require an operator to be reset.
    Resetting an operator also resets all subsequent operators.
    */
    bool maybeReset();
    /*
    Threaded processing method that attempts to advance operator state
    */
    void process();
    /*
    Used by the thread to wait for work to be available, and to not be paused.
    */
    bool waitToProcess();
    /*
    Whether or not the thread is active, ie, unpaused and has data to process
    */
    bool isActive();
    /*
    Make the process go to sleep until state is changed
    */
    void setAwake(bool idle);

    /*
    Calculates the next node to process based on current state and view node.
    */
    Node *calculateCurrentNode() const;
    /*
    Checks scene state and resets any nodes marked as dirty (or downstream of a dirty node)
    */
    bool maybeCleanNodes();
};