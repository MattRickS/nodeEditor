#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Not used, directly, but must be included to be added to registry
#include "mapgen/operators/add.hpp"
#include "mapgen/operators/constant.hpp"
#include "mapgen/operators/gradient.hpp"
#include "mapgen/operators/invert.hpp"
#include "mapgen/operators/load.hpp"
#include "mapgen/operators/merge.hpp"
#include "mapgen/operators/multiply.hpp"
#include "mapgen/operators/normals.hpp"
#include "mapgen/operators/offset.hpp"
#include "mapgen/operators/perlin.hpp"
#include "mapgen/operators/save.hpp"
#include "mapgen/operators/temperature.hpp"
#include "mapgen/operators/voronoi.hpp"

#include "mapgen/application.h"
#include "mapgen/interface/ui.h"
#include "mapgen/scene.h"

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

    Scene scene;
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