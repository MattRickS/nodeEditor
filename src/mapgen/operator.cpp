#include <vector>

#include <GL/glew.h>

#include "operator.h"
#include "renders.h"

namespace Op
{
    std::vector<Input> Operator::inputs() const { return {}; }
    std::vector<Output> Operator::outputs() const { return {{}}; }
    void Operator::defaultSettings([[maybe_unused]] Settings *settings) const {}

    void Operator::preprocess([[maybe_unused]] const std::vector<Texture *> &inputs, [[maybe_unused]] const std::vector<Texture *> &outputs, [[maybe_unused]] const Settings *settings) {}
    const std::string &Operator::error() { return m_error; }

    void Operator::setError(std::string errorMsg) { m_error = errorMsg; }
    bool Operator::hasError() const { return !m_error.empty(); }
    void Operator::reset() { m_error.clear(); }
}
