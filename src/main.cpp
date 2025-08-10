
#include "CGL/CGL.h"
#include "CGL/viewer.h"

#include "application.h"
#include "pathtracer/pathtracer.h"
#include "bubble/bubble_hgf.h"

#include "collada/collada.h"


using namespace CGL;

void fill_mesh(BubbleHGF *mesh, string fileName) {

    Collada::SceneInfo *sceneInfo = new Collada::SceneInfo();
    if (Collada::ColladaParser::load(fileName.c_str(), sceneInfo) < 0) {
        delete sceneInfo;
        exit(0);
    }

    vector<Collada::Node> &nodes = sceneInfo->nodes;

    bool built = false;

    int len = nodes.size();
    for (int i = 0; i < len; i++) {
        Collada::Node &node = nodes[i];
        Collada::Instance *instance = node.instance;
        const Matrix4x4 &transform = node.transform;

        switch (instance->type) {
        case Collada::Instance::CAMERA:
        case Collada::Instance::LIGHT:
        case Collada::Instance::SPHERE:
        case Collada::Instance::MATERIAL:
            break;
        case Collada::Instance::POLYMESH:
            auto polyMesh = static_cast<Collada::PolymeshInfo &>(*instance);
            // Build halfedge mesh from polygon soup
            vector< vector<size_t> > polygons;
            for (const Collada::Polygon &p : polyMesh.polygons) {
                polygons.push_back(p.vertex_indices);
            }
            //vector<Vector3D> vertices = polyMesh.vertices; // DELIBERATE COPY.

            // Read texture coordinates.
            //vector<Vector2D> texcoords = polyMesh.texcoords; // DELIBERATE COPY.

            mesh->build(polygons, polyMesh.vertices, polyMesh.texcoords);
            built = true;
            break;
        }
        if (built)
            break;
    }

    delete sceneInfo;
}

int main(int argc, char **argv) {


    BubbleHGF *bubbleDynamics = new BubbleHGF();

    string fileName = "../../../mesh/bubble2500vdeformed.dae.dae";
    //fileName = "../../../mesh/cube.dae";
    //fileName = "../../../mesh/teapot.dae";
    fill_mesh(bubbleDynamics, fileName);

    Application *app = new Application();


    app->set_bubble_dynamics(bubbleDynamics);

    // create viewer
    Viewer viewer = Viewer();

    // set renderer
    viewer.set_renderer(app);

    // init viewer
    viewer.init();

    // start viewer
    viewer.start();

    app->stop_pathtracer();


    delete app;

    delete bubbleDynamics;
}

// 14:39
// 0:35