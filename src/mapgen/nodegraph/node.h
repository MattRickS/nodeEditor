#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "connector.h"
#include "graphelement.h"

typedef unsigned int NodeID;

class Node : public GraphElement
{
public:
    Node(NodeID id, Op::Operator *op, glm::ivec2 dimensions = glm::ivec2(1024, 1024));
    ~Node();

    friend bool operator==(const Node &a, const Node &b);
    friend bool operator!=(const Node &a, const Node &b);

    NodeID id() const;
    std::string name() const;
    State state() const;
    const RenderSet *renderSet() const;

    // A node's dimensions are taken from it's first input. These methods can be
    // used to assign default dimensions if it has no inputs.
    virtual bool definesDimensions() const;
    glm::ivec2 dimensions() const;
    size_t width() const;
    size_t height() const;
    bool setDimensions(glm::ivec2 dimensions);

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
    // Core properties
    NodeID m_id;
    std::string m_name;
    Op::Operator *m_op;
    Settings m_settings;
    glm::ivec2 m_dimensions;
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