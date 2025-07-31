
#include "CGL/CGL.h"
#include "CGL/viewer.h"

#include "application/application.h"


using namespace CGL;


int main(int argc, char **argv) {

    Application *app = new Application();

    // create viewer
    Viewer viewer = Viewer();

    // set renderer
    viewer.set_renderer(app);

    // init viewer
    viewer.init();

    // start viewer
    viewer.start();
}