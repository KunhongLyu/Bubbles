
#include "CGL/CGL.h"
#include "CGL/viewer.h"

#include "application.h"
#include "pathtracer/pathtracer.h"
#include "bubble/bubble_hgf.h"


using namespace CGL;


int main(int argc, char **argv) {

    Application *app = new Application();

    BubbleHGF *bubbleDynamics = new BubbleHGF();

    app->set_bubble_dynamics(bubbleDynamics);

    // create viewer
    Viewer viewer = Viewer();

    // set renderer
    viewer.set_renderer(app);

    // init viewer
    viewer.init();

    // start viewer
    viewer.start();

    delete bubbleDynamics;
}

// 14:39
// 0:35