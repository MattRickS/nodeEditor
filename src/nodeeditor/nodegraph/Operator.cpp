#include <string>
#include <vector>

#include "Settings.h"
#include "Operator.h"

namespace Op
{

    const std::string &Operator::type() const { return m_type; }
    const std::string &Operator::name() const { return type(); }
    std::vector<Input> Operator::inputs() const
    {
        return {};
    }
    std::vector<Output> Operator::outputs() const
    {
        return {{}};
    }
    void Operator::registerSettings([[maybe_unused]] Settings *const settings) const
    {
    }

    void Operator::reset()
    {
        m_error.clear();
    }

    void Operator::setError(std::string errorMsg)
    {
        m_error = errorMsg;
    }
    const std::string &Operator::error() const
    {
        return m_error;
    }
    bool Operator::hasError() const
    {
        return !m_error.empty();
    }

    void Operator::setType(const std::string &type) { m_type = type; }
}
