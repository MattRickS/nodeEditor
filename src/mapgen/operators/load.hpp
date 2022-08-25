#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"
#include "../util.h"
#include "../../stb/stb_image.h"

namespace Op
{
    class Load : public Operator
    {
    public:
        static Load *create()
        {
            return new Load();
        }

        std::string name() const override { return "Load"; }
        std::vector<Output> outputs() const override
        {
            return {{}};
        }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerString("filepath", "");
        }
        bool process([[maybe_unused]] const std::vector<Texture *> &inputs,
                     const std::vector<Texture *> &outputs,
                     const Settings *settings) override
        {
            std::string filepath = settings->getString("filepath");
            if (filepath.empty())
            {
                setError("No input filepath set");
                return false;
            }

            // This is how the operator should define it's required output
            //   int x,y,n,ok;
            //   ok = stbi_info(filename, &x, &y, &n);

            int width, height, numChannels;
            unsigned char *pixels = stbi_load(filepath.c_str(), &width, &height, &numChannels, 4);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, outputs[0]->id());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, outputs[0]->width(), outputs[0]->height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);

            return true;
        }
    };

    REGISTER_OPERATOR(Load, Load::create);
}
