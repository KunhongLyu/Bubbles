#include "application.h"

namespace CGL {

    Application::Application() {
    }

    Application::~Application() {
        if (quadShader) {
            delete quadShader;
            quadShader = NULL;
        }
        if (quad) {
            delete quad;
            quad = NULL;
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


        float cubeVert[] = {
            // Front face (z = -0.5, normal = 0, 0, -1)
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  // 0
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  // 1
            0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  // 2
            0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  // 3

            // Back face (z = 0.5, normal = 0, 0, 1)
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   // 4
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   // 5
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   // 6
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   // 7

            // Left face (x = -0.5, normal = -1, 0, 0)
            0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,   // 8
            0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f,   // 9
            0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f,   // 10
            0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f,   // 11

            // Right face (x = 0.5, normal = 1, 0, 0)
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   // 12
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   // 13
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,   // 14
            0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   // 15

            // Bottom face (y = -0.5, normal = 0, -1, 0)
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  // 16
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,  // 17
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  // 18
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,  // 19

            // Top face (y = 0.5, normal = 0, 1, 0)
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   // 20
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   // 21
             0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,   // 22
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f    // 23
        };
        uint32_t cubeIndices[] = {// Front
            0, 1, 2,   0, 2, 3,
            // Back
            4, 5, 6,   4, 6, 7,
            // Left
            8, 9, 10,  8, 10, 11,
            // Right
            12, 13, 14, 12, 14, 15,
            // Bottom
            16, 17, 18, 16, 18, 19,
            // Top
            20, 21, 22, 20, 22, 23 };

        vector<ShaderInput> cubeInputFormat;
        cubeInputFormat.push_back({ Type_Float , 3 });
        cubeInputFormat.push_back({ Type_Float , 3 });

        cube = new MeshBuffer(cubeInputFormat, cubeVert, 24, cubeIndices, Type_UInt, 36, Usage_StaticDraw, Usage_StaticDraw);
    }

    void Application::render() {
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_hud();
    }

    void Application::resize(size_t w, size_t h) {
        screenW = w;
        screenH = h;
        textManager.resize(w, h);
    }

    string Application::name() {
        return "renderer";
    }

    string Application::info() {
        return "bubble";
    }


    void Application::cursor_event(float x, float y) {
        if (leftDown && !middleDown && !rightDown) {
            mouse1_dragged(x, y);
        } else if (!leftDown && !middleDown && rightDown) {
            mouse2_dragged(x, y);
        } else if (!leftDown && !middleDown && !rightDown) {
            mouse_moved(x, y);
        }

        mouseX = x;
        mouseY = y;
    }

    void Application::scroll_event(float offset_x, float offset_y) {
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
            case 'e': case 'E':
            case 'v': case 'V':
            case 's': case 'S':
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

    }

    void Application::mouse2_dragged(float x, float y) {

    }

    void Application::mouse_moved(float x, float y) {

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
        quadShader->setVec4(colorloc, 0.0, 0.0, 0.0, 0.8);
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
