#include <iostream>
#include <string>
#include <vector>

#include "connector.h"
#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "node.h"

Node::Node(NodeID id, Operator *op) : m_id(id), m_op(op)
{
    if (op)
    {
        m_name = op->name();
        op->defaultSettings(&m_settings);
        for (const auto &input : op->inputs())
        {
            addInput(input.name, input.required);
        }
        for (const auto &output : op->outputs())
        {
            addOutput(output.layer.empty() ? DEFAULT_LAYER : output.layer);
        }
    }
    else
    {
        m_name = "Null";
    }
}
Node::~Node()
{
    if (!m_outputTextures.empty())
    {
        for (auto tex : m_outputTextures)
        {
            delete tex;
        }
    }
    if (m_op)
    {
        // TODO: Deleting this is causing a segfault, not sure why...
        // Virtual and non-virtual destructor doesn't seem to matter
        delete m_op;
    }
}

bool operator==(const Node &a, const Node &b)
{
    return a.id() == b.id();
}
bool operator!=(const Node &a, const Node &b)
{
    return a.id() != b.id();
}

NodeID Node::id() const { return m_id; }
std::string Node::name() const { return m_name; }
State Node::state() const { return m_state; }
const RenderSet *Node::renderSet() const { return &m_renderSet; }

const glm::vec2 Node::pos() const { return m_pos; }
void Node::setPos(glm::vec2 pos) { m_pos = pos; }
const glm::vec2 Node::size() const { return m_size; }
void Node::setSize(glm::vec2 size) { m_size = size; }

// Maybe settings needs a redo so that the register methods are on the node, and the settings object it exposes is immutable
// This ensures settings are only updated through updateSetting() so that the dirty bit can be set
Settings *Node::settings() { return &m_settings; }
void Node::updateSetting(std::string name, SettingValue value)
{
    m_settings.get(name)->set(value);
    setDirty();
}

void Node::addInput(std::string name, bool required)
{
    if (name.empty())
    {
        name = "input_" + std::to_string(numInputs());
    }
    m_inputs.emplace_back(this, name, required);
}
size_t Node::numInputs() const { return m_inputs.size(); }
InputConnector *Node::input(size_t index)
{
    return index >= m_inputs.size() ? nullptr : &m_inputs[index];
}

bool Node::addOutput(std::string layerName)
{
    for (const OutputConnector &conn : m_outputs)
    {
        if (conn.layer() == layerName)
        {
            return false;
        }
    }
    m_outputs.emplace_back(this, layerName);
    return true;
}
size_t Node::numOutputs() const { return m_outputs.size(); }
OutputConnector *Node::output(size_t index)
{
    return index >= m_outputs.size() ? nullptr : &m_outputs[index];
}

void Node::setError(std::string errorMsg)
{
    m_error = errorMsg;
    m_state = State::Error;
}
bool Node::isDirty() const { return m_dirty; }
void Node::setDirty(bool dirty) { m_dirty = dirty; }

void Node::reset()
{
    m_renderSet.clear();
    m_error.clear();
    setDirty(false);
    m_state = State::Unprocessed;
    // input and output textures are intentionally untouched, if there is a
    // method still processing them (shouldn't be possible) then better to
    // simply let the preprocess reevaluate their contents when needed.
    if (m_op)
    {
        m_op->reset();
    }
}
bool Node::processStep()
{
    bool isComplete = false;
    switch (m_state)
    {
    case State::Unprocessed:
    case State::Preprocessing:
        m_state = State::Preprocessing;
        std::cout << "Preprocessing " << name() << std::endl;
        preprocess();
        // In case state changed during preprocessing (shouldn't be possible), discard result
        if (m_state == State::Preprocessing)
        {
            m_state = State::Processing;
        }
        break;
    case State::Processing:
        std::cout << "Processing " << name() << std::endl;
        isComplete = process();
        if (m_state == State::Processing)
        {
            m_state = State::Processed;
        }
        break;
    case State::Processed:
        isComplete = true;
        break;
    case State::Error:
        break;
    default:
        throw "Unknown state"; // TODO
    }
    return isComplete;
}
bool Node::preprocess()
{
    if (!m_op)
    {
        setError("No operator");
        return false;
    }

    m_inputTextures.clear();
    const RenderSet *const inputRenderSet = evaluateInputs();
    if (m_state == State::Error)
    {
        return false;
    }

    // Copy references to the first inputs renders as this node's initial output
    if (inputRenderSet)
    {
        m_renderSet = *inputRenderSet;
    }
    else
    {
        m_renderSet.clear();
    }

    evaluateOutputs();

    m_op->preprocess(m_inputTextures, m_outputTextures, &m_settings);
    if (m_op->hasError())
    {
        setError(m_op->error());
        return false;
    }
    return true;
}
bool Node::process()
{
    if (!m_op)
    {
        setError("No operator");
        return false;
    }

    bool isComplete = m_op->process(m_inputTextures, m_outputTextures, &m_settings);
    if (m_op->hasError())
    {
        setError(m_op->error());
        return false;
    }
    return isComplete;
}

const RenderSet *Node::evaluateInputs()
{
    const RenderSet *inputRenderSet = nullptr;
    for (const InputConnector &conn : m_inputs)
    {
        if (conn.numConnections() > 0)
        {
            Node *inNode = conn.connection(0)->node();
            if (inNode->state() != State::Processed)
            {
                setError("Input has not been processed");
                return nullptr;
            }

            const RenderSet *rs = inNode->renderSet();
            // The first valid input's renders are what will continue on
            // through this node's output (plus node changes)
            if (!inputRenderSet)
            {
                inputRenderSet = rs;
            }

            if (rs->find(conn.layer()) == rs->end())
            {
                setError("Requested input layer does not exist");
                return nullptr;
            }

            m_inputTextures.push_back(rs->at(conn.layer()));
        }
        else if (conn.isRequired())
        {
            setError("Missing required input");
            return nullptr;
        }
        else
        {
            m_inputTextures.push_back(nullptr);
        }
    }
    return inputRenderSet;
}

void Node::evaluateOutputs()
{
    // TODO: These will eventually need to be picked up from input metadata
    //       or via some alternate mechanism if the node has no inputs (eg,
    //       it only creates content)
    unsigned int width = 1024, height = 1024;
    for (size_t i = 0; i < numOutputs(); ++i)
    {
        if (m_outputTextures.size() <= i)
        {
            m_outputTextures.emplace_back(new Texture(width, height));
        }
        else if (m_outputTextures[i]->width != width || m_outputTextures[i]->height != height)
        {
            m_outputTextures[i]->resize(width, height);
        }
        m_renderSet[m_outputs[i].layer()] = m_outputTextures[i];
    }
}
