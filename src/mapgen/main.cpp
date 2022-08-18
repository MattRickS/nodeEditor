#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Not used, directly, but must be included to be added to registry
#include "operators/add.hpp"
#include "operators/constant.hpp"
#include "operators/invert.hpp"
#include "operators/multiply.hpp"
#include "operators/normals.hpp"
#include "operators/offset.hpp"
#include "operators/perlin.hpp"
#include "operators/temperature.hpp"
#include "operators/voronoi.hpp"

#include "application.h"
#include "interface/ui.h"
#include "scene.h"

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

    // TODO: The quad transform/vertices need to be scaled in the x-axis to match the image ratio
    Scene scene(1024, 1024);
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