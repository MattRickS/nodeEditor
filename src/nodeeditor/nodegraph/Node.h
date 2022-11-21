#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "Serializer.h"
#include "Settings.h"
#include "Connector.h"
#include "GraphElement.h"
#include "Operator.h"

typedef unsigned int NodeID;

enum class State
{
    Unprocessed,
    Processing,
    Processed,
    Error,
};

class Node : public GraphElement
{
public:
    Node(NodeID id, Op::Operator *op);
    ~Node();

    Node(Node &&node) noexcept;            // Move constructor
    Node(const Node &node);                // Copy constructor
    Node &operator=(Node &&node) noexcept; // Move assignment
    Node &operator=(const Node &node);     // Copy assignment

    friend bool operator==(const Node &a, const Node &b);
    friend bool operator!=(const Node &a, const Node &b);

    NodeID id() const;
    const std::string &name() const;
    const std::string &type() const;
    State state() const;
    Op::Operator *op() const;

    // Maybe settings needs a redo so that the register methods are on the node, and the settings object it exposes is immutable
    // This ensures settings are only updated through updateSetting() so that the dirty bit can be set
    Settings const *settings() const;
    void updateSetting(const std::string &name, SettingValue value);

    void addInput(const std::string &name = "", bool required = true);
    size_t numInputs() const;
    Connector *input(size_t index);
    Connector const *input(size_t index) const;

    bool addOutput(const std::string &name = "");
    size_t numOutputs() const;
    Connector *output(size_t index);

    void setError(const std::string &errorMsg);
    bool isDirty() const;
    void setDirty(bool dirty = true);

    void reset();
    bool processStep(Settings const *sceneSettings);

    bool serialize(Serializer *serializer) const;
    bool deserialize(Deserializer *deserializer);

protected:
    // Core properties
    NodeID m_id;
    std::string m_name;
    std::string m_type;
    Op::Operator *m_op;
    Settings m_settings;
    std::vector<Connector> m_inputs;
    std::vector<Connector> m_outputs;

    // State properties
    State m_state = State::Unprocessed;
    bool m_dirty = false;
    std::string m_error;

    bool evaluateInputs(std::vector<Op::Operator const *> &inputs);
    bool process(Settings const *sceneSettings);
};