This file is a running dump of the various concepts and how they're being planned.

# Concept
A list of Operators, each stores a texture which it populates by processing it's input, be that texture(s) and/or settings. The UI window then draws the texture for the active operator onto a quad, and provides transformation options to allow pan/zoom.

The UI draws a UI component for each operator in the list. A checkbox/radiobutton/listbox or similar should be used to determine which operator is considered "active".

Modifying any operator causes it to re-process it's texture, and all
subsequent operators until the active operator is processed.

# Save/Load
It should be possible to save state which entails
- Serializing the settings per operator
- Saving the texture image per operator.
Loading is then repopulating the operators.

This should be doable without needing any operator implementation.

# Simulations
Operators which run simulations, eg, erosion, should have an interface for running for a set number of iterations, or until stopped via the UI. If no predefined number of iterations has been explicitly set, stopping an indefinite run will set the current iterations as an implicit value. It should be possible to continue processing and continue incrementing the iterations.

# Randomness
Operators should all share the same RNG seed. This should be part of the base Operator class and not a setting, eg, SetSeed(seed), GetSeed().

# CPU vs GPU
Operators should be able to process the image data however they want. The current design assumes textures are on the GPU and that each operator defines a framebuffer, this should be separated into an Operator subclass, eg, GPUOperator.

CPU operations may have output information that isn't a texture, eg, vector data for regions, paths, points, text, etc... Operator methods may require another argument to be able to process and populate these.

Eg, River(path, name="Amazon"), Settlement(point, name="Dublin"), MountainRange(region, name="Alps")

# Operator methods

## void init(unsigned int width, unsigned int height)
Called once at the start to initialise any requirements. This is passed the initial image width/height to be processed. The GL context is guaranteed to exist by the time this is called, as well as a global quad VAO.

## bool process(RenderSet renderset)
Process is called repeatedly until it returns true. This allows operators to perform as many iterations on the data as required. It is passed a RenderSet which contains all the textures available from previous operators.

XXX: This may need to be updated to return a State so that Error could be included. A getError() method could then retrieve the error.
```
virtual void process(RenderSet renderset) 
{
    return error("Oops, broken");
}
```

## void reset()
Operators should reset their internal state when this is called.

## void resize(unsigned int width, unsigned int height)
Operators should resize the textures they're working with when this is called. Preserving data is optional, and not expected - the operator will likely be reset after resizing.

## preprocess()
preprocess is called once before any calls to process. It is only called again if reset() is used first. Operators have a default preprocess which does nothing.

Examples where preprocess might be useful.
- Load data onto the GPU
- Read data off the GPU (eg, CPU image processing like particles/pattern recognition/etc...)
- Scan data for elements to process

# Threading

Operator methods could be arbitrarily long, eg, could be rendering a whole scene, CPU processing an image, etc...
MapMaker should be able to process operators while being responsive to requests for changes, eg, udpating settings, resizing the image, etc...

MapMaker needs to run a processing loop with a state machine. If changes are made to the state, the processing loop updates the required operators and continues processing.

Processing should be done as fast as possible, meaning the gl context should not be restricted by monitor frame rate. Separate GL contexts will allow the UI to use vsync and minimise it's processing.

UI changes should be changes to MapMaker state using locks for the state flags, but MapMaker should never hold the lock while calling an operator method as these could be any length of time and could freeze the UI while it waits on the lock to be released.

Moving a setting slider will cause a lot of updates to the MapMaker which could result in a lot of wasted processing. User should have an option to pause the MapMaker while they make multiple changes and then restart the processing. The Controller may be able to automatically rate limit user changes to minimise wasted processing, eg, it might pause MapMaker when a change is made and start a timer which refreshes on additional changes, and only restart MapMaker once the timer has elapsed.

```
State
.idle
.preprocessing
.processing
.processed
.error

MapMaker
.width, .height
.thread
.lock

// State flags
.currIdx = 0
.targetIdx = 0
.resizing = false
.resetFromIdx = -1
.stopped = false

// states can only be modified by mapmaker, but can be read from main thread
.states = [idle for _ in operators]

// =============================================================================
// Runs in thread

private:
// A single processing step for operator states. Can advance the current operator
// from idle -> preprocessing, preprocessing -> processing, or processing->processed.
// Step holds no locks while making the calls to operator methods as these may
// take an unknown length of time.
.step()
    if currIdx >= targetIdx
        return true

    // Ensure currIdx doesn't change while determining course of action
    result = false
    wait_for_lock()
    c = currIdx
    if states[c] == idle
        states[c] = preprocessing
        release_lock()
        operators[c].preprocess()
    else if states[c] == processing || states[c] == preprocessing
        states[c] = processing
        release_lock()

        // Release lock while the operator processes
        is_processed = operators[c].process()

        // Regain the lock and if state changed while processing, discard the result
        // currIdx shouldn't be able to change without states[c] changing, just cautious
        wait_for_lock()
        if is_processed && states[c] == processing && currIdx == c
            states[c] = processed
        release_lock()
    else if states[c] == processed
        result = currIdx++ >= targetIdx
        release_lock()
    else
        throw std::exception("State's fucked")

    return result

// Checks if the mapmakers size has changed. If so, all operators are resized
// and reset.
.maybeResize()
    wait_for_lock()
    if !resizing
        release_lock()
        return false

    resizing = false
    release_lock()

    for i in 0..operators.size()
        // If resizing was requested again, stop processing. This will be called
        // again so all operators resize to the new values
        if resizing
            break
        operators[i].resize(width, height)
        operators[i].reset()
        states[i] = idle

    // Resizing could take an arbitrary length of time, and may have been
    // resized again/paused in the meantime, restart the loop before
    // processing the next step
    return true

// Checks if any changes were made that would require an operator to be reset.
// Resetting an operator also resets all subsequent operators.
.maybeReset()
    wait_for_lock()
    if resetFromIdx == -1
        release_lock()
        return false

    r = resetFromIdx
    resetFromIdx = -1
    release_lock()

    for i in t..operators.size()
        operators[i].reset()
        states[i] = idle

    currIdx = min(currIdx, resetFromIdx)

    // Resetting could take an arbitrary length of time, and may have
    // been resized/paused/reset again in the meantime, restart the loop
    // before processing the next step
    return true

// Thread process
.process()
    // Stopped should be some form of semaphore / condition variable and actually pause the thread
    while (true)
        // Allows updating repeatedly updating settings/image size without having
        // to process an operator step between updates
        if stopped maybeResize() || maybeReset()
            continue

        if step()
            // Can put the thread to sleep until it receives a state change

// =============================================================================
// Called from main thread
// "index" here will most likely be a form of iterator to avoid exposing the operators

public:
// Which operator index to process up to (inclusive)
.setTargetOperator(index)
    targetIdx = index

// Reads the current state of each operator. Doesn't technically need the lock
// as this is read only.
.states()
    wait_for_lock()
    s = copy(states)
    release_lock()
    return s

// Updates the settings for an operator and ensures it and all subsequent
// operators are reset before processing continues
.updateSettings(index, settings)
    wait_for_lock()
    operators[index].settings.update(settings)
    resetFromIdx = index
    release_lock()

// Resizes and resets all the operators before any further processing is done
.resize(w, h)
    if width == w && height == h
        return
    width = w
    height = h
    wait_for_lock()
    resizing = true
    release_lock()

// Starts the mapmaker thread processing. Thread remains active even after completion
// and will continue to update images as changes are made until stopped
.startProcessing()
    if !thread
        thread(process)
    stopped = false

// TODO: May need a timeout that allows forcibly killing the thread if not complete
// Stops the running thread. Will not start again until startProcessing is called
.stopProcessing(wait=false)
    stopped = true
    if wait
        wait_for...

```
