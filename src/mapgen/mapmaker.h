#pragma once
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "context.hpp"
#include "operators/invert.hpp"
#include "operators/perlin.h"
#include "operator.h"
#include "renders.h"

enum class State
{
    Idle,
    Preprocessing,
    Processing,
    Processed,
    Error,
};

/*
MapMaker owns no textures, each operator owns the textures it generates.
MapMaker instead owns a running mapping of Layer: Texture* as a RenderSet
that it passes as input to each operator. If rewinding to a previous operator,
this must be reset and re-populated by each prior operator in sequence.
*/
class MapMaker
{
protected:
    std::atomic<unsigned int> m_width;
    std::atomic<unsigned int> m_height;
    RenderSet renderSet; // Needs to be owned by thread

    // Thread variables. Lock is required for non-atomic states and the `stopped`
    // condition variable for pausing the thread.
    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;

    // State (threaded)
    std::vector<State> m_states;
    std::atomic<size_t> m_targetIdx = 0;
    std::atomic<int> m_resetIdx = -1;
    std::atomic<bool> m_resizing = false;
    std::atomic<bool> m_paused = false;
    std::atomic<bool> m_idle = false;
    std::atomic<bool> m_stopped = false;
    // This is only ever read and written to by the thread
    size_t m_currIdx = 0;

    /*
    A single processing step for operator states. Eg, can advance the current operator from

        idle -> preprocessing
        preprocessing -> processing
        processing -> processed

    Step does not hold the lock while making the calls to operator methods as these may
    take an unknown length of time.
    */
    bool operatorStep();
    /*
    Checks if any changes were made that would require an operator to be reset.
    Resetting an operator also resets all subsequent operators.
    */
    bool maybeReset();
    /*
    Checks if the width/height has changed. If so, all operators are resized and reset.
    */
    bool maybeResize();
    /*
    Threaded processing method that attempts to advance operator state
    */
    void process();
    /*
    Used by the thread to wait for work to be available, and to not be paused.
    */
    void waitToProcess();
    /*
    Whether or not the thread is active, ie, unpaused and has data to process
    */
    bool isActive();

public:
    Context context;
    // Making this public for now, should really expose an iterator of some sort
    std::vector<Operator *> operators;

    MapMaker(unsigned int width, unsigned int height);
    ~MapMaker();

    unsigned int Width() const;
    unsigned int Height() const;
    const RenderSet *const GetRenderSet() const;

    /*
    Sets what operator the thread will process up to.
    If the target is already processed, no new processing is performed.
    */
    void setTargetIndex(size_t index);
    /*
    Updates an operator's setting. This causes the operator and all subsequent
    operators to reset. It is advised to pause the thread if making consecutive
    changes.
    */
    void updateSetting(size_t index, std::string key, SettingValue value);
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
};