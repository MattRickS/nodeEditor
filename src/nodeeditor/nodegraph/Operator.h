#pragma once
#include <string>
#include <vector>

#include "Settings.h"

namespace Op
{
  class OperatorRegistry;

  struct Input
  {
    std::string name = "";
    bool required = true;
  };
  struct Output
  {
    std::string name = "";
  };

  /*
  Operator represents the core logic that can be assigned to a Node.

  An operator can define custom settings which are exposed in the user interface
  through the registerSettings() method which is called once on Node initialisation.

  The process() method is called repeatedly until it returns true or an error state
  is set with setError(). It accepts 3 parameters.
  - The first is a vector of Operator pointers. These are populated based on the
    inputs() method which defines how many should be present, and whether or not
    they are required. Optional inputs may be a nullptr and should be checked within
    the process method. The framework will only provide fully processed inputs.
  - A settings pointer containing the options registered with registerSettings.
  - A settings pointer to global scene settings. See `Scene` documentation for
    available settings.

  Processing state is tracked in the Node and the framework will not call state
  out of order. Should parameters change, eg, settings/inputs, the reset() method is
  called and process() method will be called to completion again.
  */
  class Operator
  {
  public:
    virtual ~Operator() = default;

    const std::string &type() const;

    /* A display name for the Operator */
    virtual const std::string &name() const;
    /* Returns the inputs the operator expects, which may be optional. Default has no inputs. */
    virtual std::vector<Input> inputs() const;
    /* Whether or not the node produces any output */
    virtual std::vector<Output> outputs() const;
    /* Registers default settings for the operator (if any). Defaults has no settings. */
    virtual void registerSettings(Settings *const settings) const;

    /* Receives an Operator per Input defined by inputs() and performs any processing */
    virtual bool process(const std::vector<Operator const *> &inputs, Settings const *settings, Settings const *sceneSettings) = 0;
    /*
    Resets any internal state for the Operator.
    Default behaviour clears any error message, any custom implementation should make sure to
    call the base method.
    */
    virtual void reset();

    /* Sets an error message on the Operator. Cleared with reset(). */
    void setError(std::string errorMsg);
    /* Returns the set error message */
    const std::string &error() const;
    /* Whether or not the Operator has an error set */
    bool hasError() const;

  private:
    std::string m_error;
    std::string m_type = "";

    friend OperatorRegistry;

    void setType(const std::string &type);
  };
}
