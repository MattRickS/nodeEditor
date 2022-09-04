#include <string>
#include <vector>

#include "../constants.h"
#include "../log.h"
#include "Settings.h"
#include "Connector.h"
#include "Node.h"
#include "Operator.h"
#include "OperatorRegistry.hpp"

Node::Node(NodeID id, Op::Operator *op) : GraphElement({0, 0, 100, 25}), m_id(id), m_op(op)
{
    if (op)
    {
        m_name = op->name();
        m_type = op->type();
        op->registerSettings(&m_settings);
        for (const auto &input : op->inputs())
        {
            addInput(input.name, input.required);
        }
        for (const auto &output : op->outputs())
        {
            addOutput(output.name);
        }
    }
    else
    {
        m_name = "Null";
        m_type = "Null";
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
}
Node::Node(Node &&node) noexcept : GraphElement(std::move(node))
{
    m_id = node.m_id;
    m_name = node.m_name;
    m_type = node.m_type;
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
}
Node::Node(const Node &node) : GraphElement(node)
{
    m_id = node.m_id;
    m_name = node.m_name;
    m_type = node.m_type;
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

    m_op = Op::OperatorRegistry::create(node.m_op->type());
    m_settings = node.m_settings;
}
Node &Node::operator=(Node &&node) noexcept
{
    GraphElement::operator=(std::move(node));

    m_id = node.m_id;
    m_name = node.m_name;
    m_type = node.m_type;
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
    return *this;
}
Node &Node::operator=(const Node &node)
{
    GraphElement::operator=(node);

    m_id = node.m_id;
    m_name = node.m_name;
    m_type = node.m_type;
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

    m_op = Op::OperatorRegistry::create(node.m_op->type());
    m_settings = node.m_settings;
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
const std::string &Node::name() const { return m_name; }
const std::string &Node::type() const { return m_type; }
State Node::state() const { return m_state; }
Op::Operator *Node::op() const { return m_op; }

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

bool Node::addOutput(const std::string &name)
{
    m_outputs.emplace_back(this,
                           Connector::Output,
                           numOutputs(),
                           name.empty() ? "output_" + std::to_string(numOutputs()) : name);
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
    m_error.clear();
    setDirty(false);
    m_state = State::Unprocessed;
    if (m_op)
    {
        m_op->reset();
    }
}
bool Node::processStep(Settings const *sceneSettings)
{
    bool ok = false;
    switch (m_state)
    {
    case State::Unprocessed:
    case State::Processing:
        LOG_DEBUG("Processing %s", type().c_str());
        ok = process(sceneSettings);
        if (ok && (m_state == State::Processing || m_state == State::Unprocessed))
        {
            m_state = State::Processed;
        }
        break;
    case State::Processed:
        ok = true;
        break;
    case State::Error:
        break;
    default:
        throw "Unknown state"; // TODO
    }
    return ok;
}
bool Node::process(Settings const *sceneSettings)
{
    if (!m_op)
    {
        setError("No operator");
        return false;
    }

    std::vector<Op::Operator const *> inputOps;
    if (!evaluateInputs(inputOps))
    {
        return false;
    }

    bool isComplete = m_op->process(inputOps, &m_settings, sceneSettings);
    if (m_op->hasError())
    {
        setError(m_op->error());
        return false;
    }

    return isComplete;
}

bool Node::evaluateInputs(std::vector<Op::Operator const *> &inputs)
{
    for (Connector &conn : m_inputs)
    {
        if (conn.numConnections() > 0)
        {
            Node *inNode = conn.connection(0)->node();
            if (inNode->state() != State::Processed)
            {
                setError("Input node '" + inNode->type() + "' has not been processed");
                return false;
            }

            inputs.emplace_back(inNode->op());
        }
        else if (conn.isRequired())
        {
            setError("Missing required input: " + std::to_string(conn.index()));
            return false;
        }
        else
        {
            inputs.push_back(nullptr);
        }
    }
    return true;
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
