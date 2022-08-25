#pragma once
#include <string>
#include <vector>

#include "../constants.h"
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
        glm::ivec2 outputImageSize([[maybe_unused]] const std::vector<Texture *> &inputs, [[maybe_unused]] const Settings *const sceneSettings, const Settings *const opSettings) override
        {
            std::string filepath = opSettings->getString("filepath");
            int width, height, numChannels, ok;
            ok = stbi_info(filepath.c_str(), &width, &height, &numChannels);
            if (!ok)
            {
                setError("Unable to read input file");
                return {};
            }
            return {width, height};
        }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerString("filepath", "");
        }

        FileType detectFileType(const std::string &filepath)
        {
            for (auto &[filetype, ext] : FILE_TYPES)
            {
                if (filepath.compare(filepath.length() - ext.length(), ext.length(), ext) == 0)
                {
                    return filetype;
                }
            }
            return FileType_None;
        }

        void loadPNG(const std::string &filepath, Texture *texture)
        {
            stbi_set_flip_vertically_on_load(true);
            int width, height, numChannels;
            unsigned char *pixels = stbi_load(filepath.c_str(), &width, &height, &numChannels, 4);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width(), texture->height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        }

        void loadHDR(const std::string &filepath, Texture *texture)
        {
            stbi_set_flip_vertically_on_load(true);
            int width, height, numChannels;
            float *pixels = stbi_loadf(filepath.c_str(), &width, &height, &numChannels, 4);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->id());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width(), texture->height(), GL_RGBA, GL_FLOAT, pixels);
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

            FileType filetype = detectFileType(filepath);
            if (filetype == FileType_None)
            {
                setError("Unrecognised or unsupported file type");
                return false;
            }

            switch (filetype)
            {
            case FileType_PNG:
                loadPNG(filepath, outputs[0]);
                break;
            case FileType_HDR:
                loadHDR(filepath, outputs[0]);
                break;

            default:
                setError("Invalid filetype");
                return false;
            }

            return true;
        }
    };

    REGISTER_OPERATOR(Load, Load::create);
}
