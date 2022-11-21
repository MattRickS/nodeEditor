#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../nodegraph/Settings.h"
#include "../gl/RenderSetOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../../stb/stb_image_write.h"

namespace Op
{
    class Save : public RenderSetOperator
    {
    public:
        static Save *create()
        {
            return new Save();
        }

        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        std::vector<Output> outputs() const override
        {
            return {};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerString("filepath", "");
            settings->registerInt("format", FileType_PNG, {{"png", FileType_PNG}, {"hdr", FileType_HDR}});
        }

        int saveAsPNG(const std::string &filepath, const Texture *texture)
        {
            // Copy image data off the GPU to a buffer for writing
            // TODO: Better scope protection around the delete
            unsigned char *pixels = new unsigned char[texture->width() * texture->height() * texture->numChannels()];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id());
            glGetTexImage(GL_TEXTURE_2D, 0, texture->format(), GL_UNSIGNED_BYTE, pixels);

            stbi_flip_vertically_on_write(true);
            int result = stbi_write_png((filepath + EXTENSION_PNG).c_str(),
                                        texture->width(),
                                        texture->height(),
                                        texture->numChannels(),
                                        pixels,
                                        texture->numChannels() * texture->width());
            delete[] pixels;
            return result;
        }

        int saveAsHDR(const std::string &filepath, const Texture *texture)
        {
            // Copy image data off the GPU to a buffer for writing
            // TODO: Better scope protection around the delete
            float *pixels = new float[texture->width() * texture->height() * texture->numChannels()];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id());
            glGetTexImage(GL_TEXTURE_2D, 0, texture->format(), GL_FLOAT, pixels);

            stbi_flip_vertically_on_write(true);
            int result = stbi_write_hdr((filepath + EXTENSION_HDR).c_str(),
                                        texture->width(),
                                        texture->height(),
                                        texture->numChannels(),
                                        pixels);
            delete[] pixels;
            return result;
        }

        bool process(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, [[maybe_unused]] Settings const *sceneSettings) override
        {
            std::string filepath = settings->getString("filepath");
            if (filepath.empty())
            {
                setError("No output filepath set");
                return false;
            }

            // TODO: Implement OpenEXR
            //       PNG is unsigned char with alpha
            //       HDR is float with no alpha
            int result;
            FileType filetype = FileType(settings->getInt("format"));
            switch (filetype)
            {
            case FileType_PNG:
                result = saveAsPNG(filepath, inputs[0]->renderSet()->cbegin()->second);
                break;
            case FileType_HDR:
                result = saveAsHDR(filepath, inputs[0]->renderSet()->cbegin()->second);
                break;
            default:
                setError("Unknown format: " + std::to_string(filetype));
                return false;
            }

            if (result == 0)
            {
                setError("Failed to write output with code: " + std::to_string(result));
                return false;
            }

            return true;
        }
    };

    REGISTER_OPERATOR(Save, Save::create);
}
