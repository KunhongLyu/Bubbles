#include "application.h"

namespace CGL {


    // TODO : Implement the Application class.

    Application::Application() {
        bubbleDynamics = nullptr;
        pathtracingSkybox = nullptr;
    }

    Application::~Application() {
        if (quadShader) {
            delete quadShader;
            quadShader = nullptr;
        }
        if (quad) {
            delete quad;
            quad = nullptr;
        }
        if (pathtracingSkybox) {
            delete pathtracingSkybox;
            pathtracingSkybox = nullptr;
        }
    }

    void Application::init() {

        textManager.init(use_hdpi);

        // Setup all the basic internal state to default values,
        // as well as some basic OpenGL state (like depth testing
        // and lighting).

        // Set the integer bit vector representing which keys are down.
        leftDown = false;
        rightDown = false;
        middleDown = false;

        show_hud = true;


        screenW = 800; screenH = 600;

        quadShader = new Shader("Shaders/quad.vert", "Shaders/quad.frag");

        
        std::vector<ShaderInput> quadInputFormat;
        quadInputFormat.push_back({ Type_Float , 2 });

        float vertices[] = {
            -1,  1,
             1,  1,
             1, -1,
            -1, -1, };
        uint32_t indices[] = { 0, 2, 1, 0, 3, 2 };
        
        quad = new MeshBuffer(quadInputFormat, vertices, 4, indices, Type_UInt, 6, Usage_StaticDraw, Usage_StaticDraw);



        phongShader = new Shader("Shaders/phong.vert", "Shaders/phong.frag");

        lightDirLoc = phongShader->uniformLocation("lightDir");
        lightColorLoc = phongShader->uniformLocation("lightColor");
        viewPosLoc = phongShader->uniformLocation("viewPos");
        ambientColorLoc = phongShader->uniformLocation("ambientColor");
        diffuseColorLoc = phongShader->uniformLocation("diffuseColor");
        specularColorLoc = phongShader->uniformLocation("specularColor");
        shininessLoc = phongShader->uniformLocation("shininess");
        modelloc = phongShader->uniformLocation("model");
        viewloc = phongShader->uniformLocation("view");
        projloc = phongShader->uniformLocation("projection");

        // example scene parameters (replace with your actual values)
        Vector3D light_direction_world = Vector3D(-0.5, -1, -1).unit(); // direction toward light source
        Vector3D light_color = Vector3D(1.0, 1.0, 1.0);
        Vector3D ambient_color = Vector3D(0.1, 0.1, 0.1);
        Vector3D diffuse_color = Vector3D(1.0, 1.0, 1.0);
        Vector3D specular_color = Vector3D(1.0, 1.0, 1.0);
        float shininess = 64.0f;
        Vector3D camera_pos = Vector3D(0.0, 0.0, 2.0);

        // upload them
        phongShader->setVec3(lightDirLoc, light_direction_world);    // shader negates it internally
        phongShader->setVec3(lightColorLoc, light_color);
        phongShader->setVec3(ambientColorLoc, ambient_color);
        phongShader->setVec3(diffuseColorLoc, diffuse_color);
        phongShader->setVec3(specularColorLoc, specular_color);
        phongShader->setVec1(shininessLoc, shininess);
        phongShader->setVec3(viewPosLoc, camera_pos);



        rotateX = 0;
        rotateY = 0;
        scale = 1.0f;

        renderMode = Mode_Phong;
        pathtracingMode = Pathtracing_Phong;
        pt = nullptr;

        global_light = new DirectionalLight(
            Vector3D(1, 1, 1),  // color
            Vector3D(-1, -1, -1).unit()  // direction
        );


        double hFov = 50;
        double vFov = 35;
        double nClip = 0.01;
        double fClip = 100;
        camera.configure(hFov, vFov, nClip, fClip, screenW, screenH);

        camera.place(
            Vector3D(0, 0, 0),
            PI / 2,           
            0.0,              
            5.0,              
            0.1,              
            100.0             
        );

        renderWireframe = false;

        // Generated the face maps from
        // https://www.panoton.de/tools/cubemap-converter/
        // using panorama/equirectangular projection


        // change this for different skybox textures
        string skyboxFolder = "galaxy";

        SkyboxFaces faces = SkyboxFaces::loadFromFiles(
            "../../../textures/" + skyboxFolder + "/nx.png",
            "../../../textures/" + skyboxFolder + "/px.png",
            "../../../textures/" + skyboxFolder + "/py.png",
            "../../../textures/" + skyboxFolder + "/ny.png",
            "../../../textures/" + skyboxFolder + "/pz.png",
            "../../../textures/" + skyboxFolder + "/nz.png"
        );

        sky = new Skybox(faces);

        pathtracingSkybox = new PathtracingSkybox(faces);
    }

    void Application::render() {

        switch (renderMode) {
        case Mode_Pathtracer:
            render_pathtracer();
            break;
        case Mode_Phong:
            render_phong();
            break;
        }
    }


    void Application::render_phong() {
        glClearColor(0, 0, 0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);




        Vector3D eyePos = camera.position();
        Vector3D eyeTarget = camera.view_point();
        Vector3D up = camera.up_dir();

        Matrix4x4 viewm = Matrix4x4::lookAt(eyePos, eyeTarget, up);

        double nearClip = 0.01;
        double farClip = 1000;

        Matrix4x4 projm = Matrix4x4::perspective(
            camera.v_fov() * (PI / 180.0),
            camera.aspect_ratio(),
            camera.near_clip(),
            camera.far_clip());

        Matrix4x4 model = Matrix4x4::rotateX(rotateX) * Matrix4x4::rotateY(rotateY);
        model = Matrix4x4::scale(scale, scale, scale) * model;

        model = Matrix4x4::identity();


        sky->setViewMat(viewm);
        sky->setProjMat(projm);
        sky->render();

        phongShader->setMat4x4(viewloc, viewm);
        phongShader->setMat4x4(projloc, projm);
        phongShader->setMat4x4(modelloc, model);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        phongShader->useProgram();

        if (bubbleDynamics != nullptr) {
            auto bubbleMeshCapture = bubbleDynamics->getMeshCapture();
            auto bubbleMesh = bubbleMeshCapture->capture();

            set_phong_color();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            bubbleMesh->draw();

            if (renderWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                set_const_color();
                float s = 1.002f;
                phongShader->setMat4x4(modelloc, Matrix4x4::scale(s, s, s));
                phongShader->useProgram();
                bubbleMesh->draw();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        } else {
            cube->draw();
        }

        phongShader->removeProgram();

        draw_hud();
    }
    void Application::render_pathtracer() {
        pt->update_screen();
    }

    void Application::set_phong_color() {
        Vector3D light_direction_world = Vector3D(-0.5, -1, -1).unit(); // direction toward light source
        Vector3D light_color = Vector3D(1.0, 1.0, 1.0);
        Vector3D ambient_color = Vector3D(0.1, 0.1, 0.1);
        Vector3D diffuse_color = Vector3D(1.0, 1.0, 1.0);
        Vector3D specular_color = Vector3D(1.0, 1.0, 1.0);
        float shininess = 64.0f;
        Vector3D camera_pos = Vector3D(0.0, 0.0, 2.0);

        // upload them
        phongShader->setVec3(lightDirLoc, light_direction_world);    // shader negates it internally
        phongShader->setVec3(lightColorLoc, light_color);
        phongShader->setVec3(ambientColorLoc, ambient_color);
        phongShader->setVec3(diffuseColorLoc, diffuse_color);
        phongShader->setVec3(specularColorLoc, specular_color);
        phongShader->setVec1(shininessLoc, shininess);
        phongShader->setVec3(viewPosLoc, camera_pos);
    }

    void Application::set_const_color() {
        Vector3D color = Vector3D(1, 0, 0);
        float shininess = 64.0f;

        // upload them
        phongShader->setVec3(ambientColorLoc, color);
        phongShader->setVec3(diffuseColorLoc, color);
        phongShader->setVec3(specularColorLoc, color);
        phongShader->setVec1(shininessLoc, shininess);
    }


    void Application::start_pathtracer() {
        if (pt == nullptr)
            pt = new PathtracerRenderer();
        
        pathtracingMode == Pathtracing_Rendering;
        scene.lights.clear();
        scene.lights.push_back(global_light);

        scene.scene = bubbleDynamics->getMeshPathtracerCapture();
        pt->set_frame_size(screenW, screenH);
        pt->set_scene(&scene);
        pt->set_camera(&camera);
        pt->set_skybox(pathtracingSkybox);
        pt->start_raytracing();
    }
    void Application::stop_pathtracer() {

        pathtracingMode == Pathtracing_Phong;
        pt->stop();
    }

    void Application::forward_dynamics() {
        bubbleDynamics->update(0.01);
        bubbleDynamics->getMeshCapture()->capture();
    }


    void Application::resize(size_t w, size_t h) {
        screenW = w;
        screenH = h;
        textManager.resize(w, h);
        camera.set_screen_size(w, h);
    }

    string Application::name() {
        return "renderer";
    }

    string Application::info() {
        return "bubble";
    }


    void Application::cursor_event(float x, float y) {
        if (leftDown) {
            mouse1_dragged(x, y);
        }
        if (rightDown) {
            mouse2_dragged(x, y);
        }

        mouse_moved(x, y);

        mouseX = x;
        mouseY = y;
    }

    void Application::scroll_event(float offset_x, float offset_y) {
        scale += offset_y * 0.1f;
        if (scale < 0.1f) {
            scale = 0.1f; // Prevent scale from going too low
        }
        if (scale > 10.0f) {
            scale = 10.0f; // Prevent scale from going too high
        }

        if (renderMode == Mode_Phong) {
            camera.move_forward(-offset_y);
        }
    }

    void Application::mouse_event(int key, int event, unsigned char mods) {
        switch (event) {
        case EVENT_PRESS:
            switch (key) {
            case MOUSE_LEFT:
                lmouse_pressed();
                break;
            case MOUSE_RIGHT:
                rmouse_pressed();
                break;
            case MOUSE_MIDDLE:
                mmouse_pressed();
                break;
            }
            break;
        case EVENT_RELEASE:
            switch (key) {
            case MOUSE_LEFT:
                lmouse_released();
                break;
            case MOUSE_RIGHT:
                rmouse_released();
                break;
            case MOUSE_MIDDLE:
                mmouse_released();
                break;
            }
            break;
        }
    }

    void Application::keyboard_event(int key, int event, unsigned char mods) {
        if (event == EVENT_PRESS) {
            switch (key) {
            case 'r': case 'R':
                start_pathtracer();
                renderMode = Mode_Pathtracer;
                break;
            case 't': case 'T':
                stop_pathtracer();
                renderMode = Mode_Phong;
                break;
            case 'f': case 'F':
                if (renderMode == Mode_Phong) {
                    forward_dynamics();
                }
                break;
            case 'm': case 'M':
                renderWireframe = !renderWireframe;
                break;
            case '[': case ']':
            case '+': case '=':
            case '-': case '_':
            case '.': case '>':
            case ',': case '<':
            case 'h': case 'H':
            case 'k': case 'K':
            case 'l': case 'L':
            case ';': case '\'':
            case 'Q': case 'q':
            case 'G': case 'g':
                pt->stop();
                pt->key_press(key);
                pt->start_raytracing();
                break;
            }
        }
    }

    void Application::lmouse_pressed() {
        leftDown = true;
    }

    void Application::lmouse_released() {
        leftDown = false;
    }

    void Application::rmouse_pressed() {
        rightDown = true;
    }

    void Application::rmouse_released() {
        rightDown = false;
    }

    void Application::mmouse_pressed() {
        middleDown = true;
    }

    void Application::mmouse_released() {
        middleDown = false;
    }

    void Application::mouse1_dragged(float x, float y) {
        float dx = x - mouseX;
        float dy = y - mouseY;
        float horizontalSensitivity = 0.01f;
        float verticalSensitivity = 0.01f;
        rotateY -= dx * horizontalSensitivity;
        rotateX += dy * verticalSensitivity;

        if (renderMode == Mode_Phong) {
            camera.rotate_by(-dy * (PI / screenH), -dx * (PI / screenW));
        }
    }

    void Application::mouse2_dragged(float x, float y) {

        float dx = (x - mouseX);
        float dy = (y - mouseY);

        // don't negate y because up is down.
        if (renderMode == Mode_Phong) {
            camera.move_by(-dx, dy, 10);
        }
    }

    void Application::mouse_moved(float x, float y) {

    }

    void Application::set_bubble_dynamics(BubbleDynamics *bubbleDynamics) {
        this->bubbleDynamics = bubbleDynamics;
    }

    inline void Application::draw_string(float x, float y,
        string str, size_t size, const Color &c) {
        int line_index = textManager.add_line((x * 2 / screenW) - 1.0,
            (-y * 2 / screenH) + 1.0,
            str, size, c);
    }


    void Application::draw_hud() {
        textManager.clear();

        const size_t size = 16;
        const float x0 = use_hdpi ? screenW - 300 * 2 : screenW - 300;
        const float y0 = use_hdpi ? 128 : 64;
        const int inc = use_hdpi ? 48 : 24;
        float y = y0 + inc - size;

        Color text_color = Color::Black;
        draw_string(x0, y0, "Hello world", size, text_color);


        // -- First draw a lovely black rectangle.


        // -- Black with opacity .8;

        float min_x = x0 - 32;
        float min_y = y0 - 32;
        float max_x = screenW;
        float max_y = y;

        float z = 0.0;

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);


        GLint colorloc = quadShader->uniformLocation("color");
        quadShader->setVec4(colorloc, 1, 1, 1, 0.8);
        GLint leftloc = quadShader->uniformLocation("left");
        GLint rightloc = quadShader->uniformLocation("right");
        GLint toploc = quadShader->uniformLocation("top");
        GLint bottomloc = quadShader->uniformLocation("bottom");
        quadShader->setVec1(leftloc, 2 * min_x / screenW - 1);
        quadShader->setVec1(rightloc, 2 * max_x / screenW - 1);
        quadShader->setVec1(bottomloc, 1 - 2 * min_y / screenH);
        quadShader->setVec1(toploc, 1 - 2 * max_y / screenH);

        quadShader->useProgram();
        quad->draw();

        textManager.render();
    }

} // namespace CGL
