#ifndef CGL_APPLICATION_H
#define CGL_APPLICATION_H

// STL
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

// libCGL
#include "CGL/CGL.h"
#include "CGL/renderer.h"
#include "CGL/osdtext.h"

#include "util/MeshBuffer.h"
#include "util/Shader.h"


using namespace std;

namespace CGL {

    class Application : public Renderer {
    public:

        Application();

        ~Application();

        void init();
        void render();
        void resize(size_t w, size_t h);

        std::string name();
        std::string info();

        void cursor_event(float x, float y);
        void scroll_event(float offset_x, float offset_y);
        void mouse_event(int key, int event, unsigned char mods);
        void keyboard_event(int key, int event, unsigned char mods);

    private:
        OSDText textManager;

        size_t screenW;
        size_t screenH;

        MeshBuffer *quad;
        Shader *quadShader;

        MeshBuffer *cube;
        Shader *cubeShader;

        float mouseX, mouseY;
        enum e_mouse_button {
            LEFT = MOUSE_LEFT,
            RIGHT = MOUSE_RIGHT,
            MIDDLE = MOUSE_MIDDLE
        };

        bool leftDown;
        bool rightDown;
        bool middleDown;

        float rotateX, rotateY;
        float scale;

        // Event handling //

        void lmouse_pressed();   // Left Mouse pressed.
        void lmouse_released();  // Left Mouse Released.
        void rmouse_pressed();   // Right Mouse pressed.
        void rmouse_released();  // Right Mouse Released.
        void mmouse_pressed();   // Middle Mouse pressed.
        void mmouse_released();  // Middle Mouse Released.
        void mouse1_dragged(float x, float y);  // Left Mouse Dragged.
        void mouse2_dragged(float x, float y);  // Right Mouse Dragged.
        void mouse_moved(float x, float y);     // Mouse Moved.

        // HUD
        bool show_hud;
        void draw_hud();
        inline void draw_string(float x, float y,
            string str, size_t size, const Color &c);

    }; // class Application

} // namespace CGL


#endif // CGL_APPLICATION_H
