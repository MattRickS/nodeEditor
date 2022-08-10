#pragma once
#include <string>
#include <vector>

#include "connector.h"
#include "../constants.h"
#include "../operator.h"
#include "../renders.h"
#include "../settings.h"

typedef unsigned int NodeID;

enum class State
{
    Unprocessed,
    Preprocessing,
    Processing,
    Processed,
    Error,
};

class Node
{
public:
    Node(NodeID id, Operator *op);
    ~Node();

    friend bool operator==(const Node &a, const Node &b);
    friend bool operator!=(const Node &a, const Node &b);

    NodeID id() const;
    std::string name() const;
    State state() const;
    const RenderSet *renderSet() const;

    const glm::vec2 pos() const;
    void setPos(glm::vec2 pos);
    const glm::vec2 size() const;
    void setSize(glm::vec2 size);

    // Maybe settings needs a redo so that the register methods are on the node, and the settings object it exposes is immutable
    // This ensures settings are only updated through updateSetting() so that the dirty bit can be set
    Settings *settings();
    void updateSetting(std::string name, SettingValue value);

    void addInput(std::string name = "", bool required = true);
    size_t numInputs() const;
    InputConnector *input(size_t index);

    bool addOutput(std::string layerName = DEFAULT_LAYER);
    size_t numOutputs() const;
    OutputConnector *output(size_t index);

    void setError(std::string errorMsg);
    bool isDirty() const;
    void setDirty(bool dirty = true);

    void reset();
    bool processStep();
    bool preprocess();
    bool process();

protected:
    // Drawing properties
    glm::vec2 m_pos = glm::vec2(0);
    glm::vec2 m_size = glm::vec2(1);

    // Core properties
    NodeID m_id;
    std::string m_name;
    Operator *m_op;
    Settings m_settings;
    std::vector<InputConnector> m_inputs;
    std::vector<OutputConnector> m_outputs;

    // State properties
    State m_state = State::Unprocessed;
    bool m_dirty = false;
    std::string m_error;

    // Current state of renders, may reference textures from prior nodes
    // Calculated during preprocess to match current state
    RenderSet m_renderSet;
    std::vector<Texture *> m_inputTextures;
    std::vector<Texture *> m_outputTextures;

    const RenderSet *evaluateInputs();
    void evaluateOutputs();
};