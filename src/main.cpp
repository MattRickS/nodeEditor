#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Not used, directly, but must be included to be added to registry
#include "mapgen/operators/Add.hpp"
#include "mapgen/operators/CheckerBoard.hpp"
#include "mapgen/operators/Clamp.hpp"
#include "mapgen/operators/Constant.hpp"
#include "mapgen/operators/ConvolveTexture.hpp"
#include "mapgen/operators/CopyLayer.hpp"
#include "mapgen/operators/ExtractLayer.hpp"
#include "mapgen/operators/Gaussian.hpp"
#include "mapgen/operators/Gradient.hpp"
#include "mapgen/operators/Invert.hpp"
#include "mapgen/operators/JumpFlood.hpp"
#include "mapgen/operators/Load.hpp"
#include "mapgen/operators/Merge.hpp"
#include "mapgen/operators/Multiply.hpp"
#include "mapgen/operators/Normals.hpp"
#include "mapgen/operators/Offset.hpp"
#include "mapgen/operators/Perlin.hpp"
#include "mapgen/operators/Pixel.hpp"
#include "mapgen/operators/Power.hpp"
#include "mapgen/operators/Save.hpp"
#include "mapgen/operators/Shuffle.hpp"
#include "mapgen/operators/Temperature.hpp"
#include "mapgen/operators/Voronoi.hpp"

#include "mapgen/Application.h"
#include "mapgen/interface/UI.h"
#include "mapgen/gl/RenderScene.h"

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