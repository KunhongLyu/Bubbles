#include "application.h"

namespace CGL {

    Application::Application() {
    }

    Application::~Application() {

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

        // Lighting needs to be explicitly enabled.
        glEnable(GL_LIGHTING);

        // Enable anti-aliasing and circular points.
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

        screenW = 800; screenH = 600;
    }

    void Application::render() {

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
        const float y = use_hdpi ? 128 : 64;
        const int inc = use_hdpi ? 48 : 24;
        float y0 = y + inc - size;

        Color text_color = Color::White;
        draw_string(x0, y0, "Hello world", size, text_color);


        // -- First draw a lovely black rectangle.

        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(0, 0, screenW, screenH);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, screenW, screenH, 0, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0, 0, -1);

        // -- Black with opacity .8;

        glColor4f(0.0, 0.0, 0.0, 0.8);

        float min_x = x0 - 32;
        float min_y = y0 - 32;
        float max_x = screenW;
        float max_y = y;

        float z = 0.0;

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glBegin(GL_QUADS);

        glVertex3f(min_x, min_y, z);
        glVertex3f(min_x, max_y, z);
        glVertex3f(max_x, max_y, z);
        glVertex3f(max_x, min_y, z);
        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glPopAttrib();

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);

        textManager.render();
    }

} // namespace CGL
