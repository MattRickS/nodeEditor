#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "operators/erosion.hpp"
#include "operators/invert.hpp"
#include "operators/perlin.h"
#include "operators/voronoi.hpp"
#include "operator.h"
#include "renders.h"
#include "mapmaker.h"
#include "util.hpp"

MapMaker::MapMaker(unsigned int width, unsigned int height) : m_width(width), m_height(height), context("MapMaker")
{
    operators.push_back(OperatorRegistry::create("VoronoiNoise"));
    operators.push_back(OperatorRegistry::create("InvertOp"));
    operators.push_back(OperatorRegistry::create("PerlinNoise"));
    operators.push_back(OperatorRegistry::create("ErosionOp"));
    for (auto op : operators)
    {
        op->init(m_width, m_height);
    }
}
MapMaker::~MapMaker()
{
    // TODO: minor, as this being destroyed is also exiting the process
    // for (size_t i = operators.size() - 1; i >= 0; --i)
    // {
    //     delete operators[i];
    // }
    if (!m_stopped.load())
    {
        stopProcessing();
    }
}

unsigned int MapMaker::Width() const { return m_width.load(); }
unsigned int MapMaker::Height() const { return m_height.load(); }

const RenderSet *const MapMaker::GetRenderSet() const
{
    return &renderSet;
}

size_t MapMaker::GetCurrentIndex()
{
    // Current index is used to track processing and gets incremented until it
    // exceeds the target
    return std::min(m_currIdx, m_targetIdx.load());
}
size_t MapMaker::GetTargetIndex() { return m_targetIdx.load(); }

void MapMaker::setTargetIndex(size_t index)
{
    // TODO: this may also need to reset the state of the current operator
    //       if it's still processing. This is because an operator potentially
    //       relies on preprocess to set global state that would be lost when
    //       switching to another operator. Current design means no other
    //       operator _should_ be called, but something to be wary of.
    m_targetIdx = index;
    if (m_targetIdx >= m_currIdx)
    {
        for (size_t i = m_currIdx; i <= m_targetIdx; ++i)
        {
            if (m_states[i] != State::Processed)
            {
                // There are internal state changes to process, ensure the thread is awake.
                // If the thread is paused from the UI, it won't wake until unpaused.
                setAwake(true);
                return;
            }
        }
    }
    // If going back to an old or processed index, set the current index to
    // load the renderset for that index
    setCurrentIndex(m_targetIdx);
}

bool MapMaker::updateSetting(size_t index, std::string key, SettingValue value)
{
    operators[index]->settings.get(key)->set(value);
    // Internal state only needs processing if the operator or it's dependents
    // were processed.
    for (size_t i = index; i < operators.size(); ++i)
    {
        if (m_states[i] != State::Idle)
        {
            // This will be picked up on the next iteration of the thread loop, causing
            // all operators from this index and above to be reset.
            m_resetIdx = index;
            // TODO: resetting timer before processing is triggered?
            // There are internal state changes to process, ensure the thread is awake.
            // If the thread is paused from the UI, it won't wake until unpaused.
            setAwake(true);
        }
    }
    return true;
}

bool MapMaker::startProcessing()
{
    if (m_thread)
    {
        return false;
    }

    m_stopped = false;
    m_thread = std::make_unique<std::thread>(std::bind(&MapMaker::process, this));
    return true;
}

void MapMaker::stopProcessing()
{
    // Ensure the thread is woken up so that it can stop the loop
    m_stopped = true;
    setAwake(true);
    setPaused(false);
    std::cout << "Waiting on thread to stop" << std::endl;
    m_thread->join();
}

void MapMaker::setPaused(bool paused)
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

bool MapMaker::isPaused()
{
    return m_paused.load();
}

// =============================================================================
// Private

bool MapMaker::operatorStep()
{
    if (m_currIdx > m_targetIdx.load())
    {
        return true;
    }

    // TODO: states need some lock handling. They're only modified within the sequential
    //       methods of this process loop, but their are potential synchronization/memory
    //       barrier issues that might affect the reads from the main thread.
    bool isComplete = false;
    size_t currIdx = m_currIdx;
    bool isProcessed;
    switch (m_states[currIdx])
    {
    case State::Error:
        setAwake(false);
        break;
    case State::Idle:
        m_states[currIdx] = State::Preprocessing;
        std::cout << "Pre-processing op: " << currIdx << std::endl;
        operators[currIdx]->preprocess(&renderSet);
        break;
    case State::Preprocessing:
    case State::Processing:
        m_states[currIdx] = State::Processing;

        std::cout << "Processing op: " << currIdx << std::endl;
        // Release lock while the operator method processes
        isProcessed = operators[currIdx]->process(&renderSet);

        // Regain the lock and if state changed while processing, discard the result
        if (isProcessed && m_states[currIdx] == State::Processing && currIdx == m_currIdx)
        {
            m_states[currIdx] = State::Processed;
        }

        // If only one step was requested, reset the flag. If not paused, processing
        // may continue if moved to a new operator with multiple iterations.
        if (m_processOne.load())
        {
            m_processOne = false;
        }

        // XXX: Perhaps this should be moved to a "sync" method so that it's only called
        //      when required, eg, on frame draw (1/60s)
        // Add the completed operator's renders to the renderset, replacing layers
        // that were updated
        operators[m_currIdx]->PopulateRenderSet(&renderSet);
        // Sync the updated textures with the shared context
        glFinish();

        // If process is incomplete, release the lock and let the cycle continue
        // Otherwise fall through into Processed state behaviour.
        if (!isProcessed)
        {
            break;
        }
    case State::Processed:
        // Advance to the next index and check if finished
        isComplete = ++m_currIdx > m_targetIdx.load();
        break;
    default:
        throw ""; // TODO: handle this
    }

    return isComplete;
}

bool MapMaker::maybeReset()
{
    if (m_resetIdx == -1)
        return false;

    int resetIdx = m_resetIdx.exchange(-1);

    for (size_t i = resetIdx; i < operators.size(); ++i)
    {
        operators[i]->reset();
        // TODO: lock?
        m_states[i] = State::Idle;
    }

    if (resetIdx < m_currIdx)
    {
        m_currIdx = resetIdx;
    }

    // Ensure the thread wakes up if it had paused itself
    setAwake(true);
    return true;
}

bool MapMaker::maybeResize()
{
    if (!m_resizing.load())
    {
        return false;
    }
    m_resizing = false;

    for (size_t i = 0; i < operators.size(); ++i)
    {
        // If resizing was requested again, stop processing. This will be called
        // again so all operators resize to the new values
        if (m_resizing.load())
            break;
        operators[i]->resize(m_width, m_height);
        operators[i]->reset();
        // TODO: lock?
        m_states[i] = State::Idle;
    }

    // Ensure the thread wakes up if it had paused itself
    setAwake(true);
    return true;
}

bool MapMaker::waitToProcess()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, std::bind(&MapMaker::isActive, this));
    return true;
}

void MapMaker::process()
{
    // TODO: Tidy up this thread initialisation
    // TODO: Switch to smart pointers
    context.use();
    std::cout << "Initialising thread" << std::endl;

    makeQuad(&quadVAO);

    m_states = std::vector<State>(operators.size(), State::Idle);

    std::cout << "Starting process" << std::endl;
    // Ensure the main thread hasn't requested the thread be paused
    // m_stopped is checked last so that a request to stop the thread is
    // immediate when it wakes up
    while (waitToProcess() && !m_stopped.load())
    {
        // Ensure all state changes are processed first
        if (maybeReset() || maybeResize())
        {
            std::cout << "State change" << std::endl;
            continue;
        }

        // If everything has processed up to the target operator, mark the thread as idle
        if (operatorStep())
        {
            setAwake(false);
        }
    }
}

bool MapMaker::isActive()
{
    // Stop on step requests advancing a single operator step and should occur
    // even when paused but without clearing the paused state.
    return m_processOne.load() || (!m_paused.load() && m_awake.load());
}

void MapMaker::setAwake(bool idle)
{
    std::cout << "Idle thread, going to sleep" << std::endl;
    std::lock_guard<std::mutex> guard(m_mutex);
    m_awake = idle;
    m_condition.notify_one();
}

void MapMaker::setCurrentIndex(size_t currIdx)
{
    // TODO: This needs lock handling as it will be triggered from the main thread,
    // but modified the renderset used by the render thread
    renderSet.clear();
    for (size_t i = 0; i <= currIdx; ++i)
    {
        if (m_states[i] != State::Processed)
        {
            std::cerr << "Current index set to " << currIdx << " but index " << i << " is not processed" << std::endl;
            break;
        }
        operators[i]->PopulateRenderSet(&renderSet);
    }

    m_currIdx = currIdx;
}

bool MapMaker::processOne()
{
    if (isActive())
    {
        return false;
    }

    // Ensure there is an unprocessed target
    for (size_t i = m_targetIdx; i < operators.size(); ++i)
    {
        if (m_states[i] != State::Processed)
        {
            m_targetIdx = i;
            break;
        }
    }
    // If already fully processed, return
    if (m_states[m_targetIdx.load()] == State::Processed)
    {
        return false;
    }

    m_processOne = true;
    setAwake(true);
    return true;
}
