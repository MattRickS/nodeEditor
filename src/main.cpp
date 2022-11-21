#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Not used, directly, but must be included to be added to registry
#include "nodeeditor/operators/Add.hpp"
#include "nodeeditor/operators/CheckerBoard.hpp"
#include "nodeeditor/operators/Clamp.hpp"
#include "nodeeditor/operators/Constant.hpp"
#include "nodeeditor/operators/ConvolveTexture.hpp"
#include "nodeeditor/operators/CopyLayer.hpp"
#include "nodeeditor/operators/ExtractLayer.hpp"
#include "nodeeditor/operators/Gaussian.hpp"
#include "nodeeditor/operators/Gradient.hpp"
#include "nodeeditor/operators/Invert.hpp"
#include "nodeeditor/operators/JumpFlood.hpp"
#include "nodeeditor/operators/Load.hpp"
#include "nodeeditor/operators/Merge.hpp"
#include "nodeeditor/operators/Multiply.hpp"
#include "nodeeditor/operators/Normals.hpp"
#include "nodeeditor/operators/Offset.hpp"
#include "nodeeditor/operators/Perlin.hpp"
#include "nodeeditor/operators/Pixel.hpp"
#include "nodeeditor/operators/Power.hpp"
#include "nodeeditor/operators/Save.hpp"
#include "nodeeditor/operators/Shuffle.hpp"
#include "nodeeditor/operators/Temperature.hpp"
#include "nodeeditor/operators/VectorBand.hpp"
#include "nodeeditor/operators/Voronoi.hpp"

#include "nodeeditor/Application.h"
#include "nodeeditor/interface/UI.h"
#include "nodeeditor/gl/RenderScene.h"

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    // GLFW init
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    RenderScene scene;
    if (!scene.context()->isInitialised())
        return 1;

    UI ui = UI(1280, 720, "MapMakerUI", scene.context());
    if (!ui.isInitialised())
        return 1;

    Application app = Application(&scene, &ui);
    app.exec();

    // XXX: Seeing output "Glfw Error 65537: The GLFW library is not initialized"
    // Think this fails because the window is already destroyed, non-critical
    glfwTerminate();
    return 0;
}