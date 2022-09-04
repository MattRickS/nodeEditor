#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../nodegraph/Settings.h"
#include "../gl/RenderSetOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../../stb/stb_image.h"

namespace Op
{
    class Load : public RenderSetOperator
    {
    public:
        static Load *create()
        {
            return new Load();
        }

        void registerSettings(Settings *settings) const override
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

        void loadPNG(const std::string &filepath)
        {
            stbi_set_flip_vertically_on_load(true);
            int width, height, numChannels;
            unsigned char *pixels = stbi_load(filepath.c_str(), &width, &height, &numChannels, 4);

            Texture *texture = ensureOutputLayer(DEFAULT_LAYER, {width, height});
            texture->write(pixels, texture->width(), texture->height());
        }

        void loadHDR(const std::string &filepath)
        {
            stbi_set_flip_vertically_on_load(true);
            int width, height, numChannels;
            float *pixels = stbi_loadf(filepath.c_str(), &width, &height, &numChannels, 4);

            Texture *texture = ensureOutputLayer(DEFAULT_LAYER, {width, height});
            texture->write(pixels, texture->width(), texture->height());
        }

        bool process([[maybe_unused]] const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, [[maybe_unused]] Settings const *sceneSettings) override
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
                loadPNG(filepath);
                break;
            case FileType_HDR:
                loadHDR(filepath);
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
