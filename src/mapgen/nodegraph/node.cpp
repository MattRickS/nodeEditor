#include <string>
#include <vector>

#include "../constants.h"
#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "../util.h"
#include "connector.h"
#include "node.h"

Node::Node(NodeID id, Op::Operator *op) : GraphElement({0, 0, 100, 25}), m_id(id), m_op(op)
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
    for (Connector &conn : m_inputs)
    {
        conn.disconnectAll();
    }
    for (Connector &conn : m_outputs)
    {
        conn.disconnectAll();
    }
    if (!m_outputTextures.empty())
    {
        for (auto tex : m_outputTextures)
        {
            delete tex;
        }
    }
}
Node::Node(Node &&node) noexcept : GraphElement(std::move(node))
{
    m_id = node.m_id;
    m_name = node.m_name;
    m_inputs = node.m_inputs;
    m_outputs = node.m_outputs;
    // The connector's Node pointer points to the old node, update to new instance
    for (Connector &conn : m_inputs)
    {
        conn.m_node = this;
    }
    for (Connector &conn : m_outputs)
    {
        conn.m_node = this;
    }
    m_state = node.m_state;
    m_dirty = node.m_dirty;
    m_error = node.m_error;

    m_op = std::move(node.m_op);
    m_settings = node.m_settings;
    m_imageSize = node.m_imageSize;
    m_renderSet = node.m_renderSet;
    m_inputTextures = node.m_inputTextures;
    m_outputTextures = node.m_outputTextures;
}
Node::Node(const Node &node) : GraphElement(node)
{
    m_id = node.m_id;
    m_name = node.m_name;
    m_inputs = node.m_inputs;
    m_outputs = node.m_outputs;
    // The connector's Node pointer points to the old node, update to new instance
    for (Connector &conn : m_inputs)
    {
        conn.m_node = this;
    }
    for (Connector &conn : m_outputs)
    {
        conn.m_node = this;
    }
    m_state = node.m_state;
    m_dirty = node.m_dirty;
    m_error = node.m_error;

    m_op.reset(Op::OperatorRegistry::create(node.m_op->name()));
    m_settings = node.m_settings;
    m_imageSize = node.m_imageSize;
    m_renderSet = node.m_renderSet;
    m_inputTextures = node.m_inputTextures;
    m_outputTextures = node.m_outputTextures;
}
Node &Node::operator=(Node &&node) noexcept
{
    GraphElement::operator=(std::move(node));

    m_id = node.m_id;
    m_name = node.m_name;
    m_inputs = node.m_inputs;
    m_outputs = node.m_outputs;
    // The connector's Node pointer points to the old node, update to new instance
    for (Connector &conn : m_inputs)
    {
        conn.m_node = this;
    }
    for (Connector &conn : m_outputs)
    {
        conn.m_node = this;
    }
    m_state = node.m_state;
    m_dirty = node.m_dirty;
    m_error = node.m_error;

    m_op = std::move(node.m_op);
    m_settings = node.m_settings;
    m_imageSize = node.m_imageSize;
    m_renderSet = node.m_renderSet;
    m_inputTextures = node.m_inputTextures;
    m_outputTextures = node.m_outputTextures;
    return *this;
}
Node &Node::operator=(const Node &node)
{
    GraphElement::operator=(node);

    m_id = node.m_id;
    m_name = node.m_name;
    m_inputs = node.m_inputs;
    m_outputs = node.m_outputs;
    // The connector's Node pointer points to the old node, update to new instance
    for (Connector &conn : m_inputs)
    {
        conn.m_node = this;
    }
    for (Connector &conn : m_outputs)
    {
        conn.m_node = this;
    }
    m_state = node.m_state;
    m_dirty = node.m_dirty;
    m_error = node.m_error;

    m_op.reset(Op::OperatorRegistry::create(node.m_op->name()));
    m_settings = node.m_settings;
    m_imageSize = node.m_imageSize;
    m_renderSet = node.m_renderSet;
    m_inputTextures = node.m_inputTextures;
    m_outputTextures = node.m_outputTextures;
    return *this;
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
const std::string &Node::type() const { return m_name; }
State Node::state() const { return m_state; }
const RenderSet *Node::renderSet() const { return &m_renderSet; }

glm::ivec2 Node::imageSize() const
{
    return m_imageSize;
}
bool Node::recalculateImageSize(const Settings *sceneSettings)
{
    if (!m_op)
    {
        return false;
    }
    glm::ivec2 imageSize = m_op->outputImageSize(m_inputTextures, sceneSettings, &m_settings);
    if (imageSize == m_imageSize)
    {
        return false;
    }
    m_imageSize = imageSize;
    return true;
}

// Maybe settings needs a redo so that the register methods are on the node, and the settings object it exposes is immutable
// This ensures settings are only updated through updateSetting() so that the dirty bit can be set
Settings const *Node::settings() const { return &m_settings; }
void Node::updateSetting(const std::string &name, SettingValue value)
{
    m_settings.get(name)->set(value);
    setDirty(true);
}

void Node::addInput(const std::string &name, bool required)
{
    m_inputs.emplace_back(this,
                          Connector::Input,
                          numInputs(),
                          name.empty() ? "input_" + std::to_string(numInputs()) : name,
                          1,
                          required);
}
size_t Node::numInputs() const { return m_inputs.size(); }
Connector *Node::input(size_t index)
{
    return index >= m_inputs.size() ? nullptr : &m_inputs[index];
}
Connector const *Node::input(size_t index) const
{
    return index >= m_inputs.size() ? nullptr : &m_inputs[index];
}

bool Node::addOutput(const std::string &layerName)
{
    for (Connector &conn : m_outputs)
    {
        if (conn.layer() == layerName)
        {
            return false;
        }
    }
    m_outputs.emplace_back(this,
                           Connector::Output,
                           numOutputs(),
                           "output_" + std::to_string(numOutputs()));
    m_outputs.back().setLayer(layerName);
    return true;
}
size_t Node::numOutputs() const { return m_outputs.size(); }
Connector *Node::output(size_t index)
{
    return index >= m_outputs.size() ? nullptr : &m_outputs[index];
}

void Node::setError(const std::string &errorMsg)
{
    m_error = errorMsg;
    m_state = State::Error;
    LOG_ERROR("Node %s has error: %s", type().c_str(), m_error.c_str());
}
bool Node::isDirty() const { return m_dirty; }
void Node::setDirty(bool dirty) { m_dirty = dirty; }

void Node::reset()
{
    LOG_DEBUG("Resetting %s", type().c_str());
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
bool Node::processStep(const Settings *const sceneSettings)
{
    bool isComplete = false;
    switch (m_state)
    {
    case State::Unprocessed:
    case State::Preprocessing:
        m_state = State::Preprocessing;
        LOG_DEBUG("Preprocessing %s", type().c_str());
        preprocess(sceneSettings);
        // In case state changed during preprocessing (shouldn't be possible), discard result
        if (m_state == State::Preprocessing)
        {
            m_state = State::Processing;
        }
        break;
    case State::Processing:
        LOG_DEBUG("Processing %s", type().c_str());
        isComplete = process(sceneSettings);
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
bool Node::preprocess(const Settings *sceneSettings)
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

    // Update the output size and ensure there are matching output textures
    recalculateImageSize(sceneSettings);
    evaluateOutputs();

    m_op->preprocess(m_inputTextures, m_outputTextures, &m_settings);
    if (m_op->hasError())
    {
        setError(m_op->error());
        return false;
    }
    return true;
}
bool Node::process([[maybe_unused]] const Settings *const sceneSettings)
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
    glFinish();
    return isComplete;
}

const RenderSet *Node::evaluateInputs()
{
    const RenderSet *inputRenderSet = nullptr;
    for (Connector &conn : m_inputs)
    {
        if (conn.numConnections() > 0)
        {
            Node *inNode = conn.connection(0)->node();
            if (inNode->state() != State::Processed)
            {
                setError("Input node '" + inNode->type() + "' has not been processed");
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
                setError("Requested input layer '" + conn.layer() + "' does not exist");
                return nullptr;
            }

            m_inputTextures.push_back(rs->at(conn.layer()));
        }
        else if (conn.isRequired())
        {
            setError("Missing required input: " + std::to_string(conn.index()));
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
    unsigned int width = m_imageSize.x, height = m_imageSize.y;

    for (size_t i = 0; i < numOutputs(); ++i)
    {
        if (m_outputTextures.size() <= i)
        {
            m_outputTextures.emplace_back(new Texture(width, height));
        }
        else if (m_outputTextures[i]->width() != width || m_outputTextures[i]->height() != height)
        {
            m_outputTextures[i]->resize(width, height);
        }
        auto it = m_renderSet.insert_or_assign(m_outputs[i].layer(), m_outputTextures[i]);
        LOG_DEBUG("%s output ID %u to layer %s", (it.second ? "Inserted" : "Assigned"), m_outputTextures[i]->id(), m_outputs[i].layer().c_str());
    }
}

bool Node::serialize(Serializer *serializer) const
{
    bool ok = serializer->writePropertyInt(KEY_NODE_ID, id());
    ok = ok && serializer->writePropertyInt2(KEY_NODE_POS, bounds().pos());
    ok = ok && serializer->writePropertyInt(KEY_NODE_FLAGS, (int)m_selectState);

    ok = ok && serializer->startObject(KEY_SETTINGS);
    ok = ok && m_settings.serialize(serializer);
    ok = ok && serializer->finishObject();
    return ok;
}

bool Node::deserialize(Deserializer *deserializer)
{
    bool ok = true;
    std::string property;
    while (ok && deserializer->readProperty(property))
    {
        if (property == KEY_NODE_ID)
        {
            int id;
            ok = ok && deserializer->readInt(id);
            if (ok)
            {
                m_id = id;
            }
        }
        else if (property == KEY_NODE_POS)
        {
            // TODO: This should be ivec2
            glm::vec2 pos;
            ok = ok && deserializer->readFloat2(pos);
            if (ok)
            {
                setPos(pos);
            }
        }
        else if (property == KEY_NODE_FLAGS)
        {
            int flags;
            ok = ok && deserializer->readInt(flags);
            if (ok)
            {
                m_selectState = SelectFlag(flags);
            }
        }
        else if (property == KEY_SETTINGS)
        {
            ok = ok && deserializer->startReadObject();
            ok = ok && m_settings.deserialize(deserializer);
            ok = ok && deserializer->finishReadObject();
        }
        else
        {
            LOG_WARNING("Unknown node property: %s", property.c_str());
        }
    }
    return true;
}
