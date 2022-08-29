#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../constants.h"
#include "../operator.h"
#include "../renders.h"
#include "../serializer.h"
#include "../settings.h"
#include "connector.h"
#include "graphelement.h"

typedef unsigned int NodeID;

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
    const std::string &type() const;
    State state() const;
    const RenderSet *renderSet() const;

    // A node's imageSize is taken from it's first input. These methods can be
    // used to assign default imageSize if it has no inputs.
    glm::ivec2 imageSize() const;
    bool recalculateImageSize(const Settings *sceneSettings);

    // Maybe settings needs a redo so that the register methods are on the node, and the settings object it exposes is immutable
    // This ensures settings are only updated through updateSetting() so that the dirty bit can be set
    Settings const *settings() const;
    void updateSetting(const std::string &name, SettingValue value);

    void addInput(const std::string &name = "", bool required = true);
    size_t numInputs() const;
    Connector *input(size_t index);
    Connector const *input(size_t index) const;

    bool addOutput(const std::string &layerName = DEFAULT_LAYER);
    size_t numOutputs() const;
    Connector *output(size_t index);

    void setError(const std::string &errorMsg);
    bool isDirty() const;
    void setDirty(bool dirty = true);

    void reset();
    bool processStep(const Settings *const sceneSettings);
    bool preprocess(const Settings *const sceneSettings);
    bool process(const Settings *const sceneSettings);

    bool serialize(Serializer *serializer) const;
    bool deserialize(Deserializer *deserializer);

protected:
    // Core properties
    NodeID m_id;
    std::string m_name;
    std::unique_ptr<Op::Operator> m_op;
    Settings m_settings;
    glm::ivec2 m_imageSize;
    std::vector<Connector> m_inputs;
    std::vector<Connector> m_outputs;

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