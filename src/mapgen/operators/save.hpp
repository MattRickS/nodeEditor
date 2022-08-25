#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"
#include "../util.h"
#include "../../stb/stb_image_write.h"

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
        bool process(const std::vector<Texture *> &inputs,
                     [[maybe_unused]] const std::vector<Texture *> &outputs,
                     const Settings *settings) override
        {
            std::string filepath = settings->getString("filepath");
            if (filepath.empty())
            {
                setError("No output filepath set");
                return false;
            }

            LOG_INFO("Saving to: %s", filepath.c_str());

            // Copy image data off the GPU to a buffer for writing
            // TODO: Better scope protection around the delete
            unsigned char *pixels = new unsigned char[inputs[0]->width * inputs[0]->height * inputs[0]->numChannels()];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, inputs[0]->ID);
            glGetTexImage(GL_TEXTURE_2D, 0, inputs[0]->format, GL_UNSIGNED_BYTE, pixels);

            stbi_flip_vertically_on_write(true);
            int result = stbi_write_png(filepath.c_str(),
                                        inputs[0]->width,
                                        inputs[0]->height,
                                        inputs[0]->numChannels(),
                                        pixels,
                                        inputs[0]->numChannels() * inputs[0]->width);
            delete[] pixels;

            if (result == 0)
            {
                setError("Failed to write output with code");
                return false;
            }

            return true;
        }
    };

    REGISTER_OPERATOR(Save, Save::create);
}
