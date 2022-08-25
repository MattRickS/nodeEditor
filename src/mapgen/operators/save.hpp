#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"
#include "../util.h"

namespace Op
{
    class Save : public Operator
    {
    public:
        static Save *create()
        {
            return new Save();
        }

        std::string name() const override { return "Save"; }
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        std::vector<Output> outputs() const override
        {
            return {};
        }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerString("filepath", "");
        }
        bool process([[maybe_unused]] const std::vector<Texture *> &inputs,
                     [[maybe_unused]] const std::vector<Texture *> &outputs,
                     const Settings *settings) override
        {
            std::string filepath = settings->getString("filepath");
            if (filepath.empty())
            {
                setError("No output filepath set");
                return false;
            }
            else
            {
                LOG_INFO("Saving to: %s", filepath.c_str());
            }
            return true;
        }
    };

    REGISTER_OPERATOR(Save, Save::create);
}
