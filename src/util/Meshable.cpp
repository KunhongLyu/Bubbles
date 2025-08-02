
#include "Meshable.h"

namespace CGL {
    namespace SceneObjects {

        MeshCapture::MeshCapture() : cur_meshable(NULL) {};
        MeshCapture::~MeshCapture() {
            release();
        };

        Meshable *MeshCapture::capture() {
            if (cur_meshable == nullptr) {
                cur_meshable = create_meshable();
            }
            update_cur_meshable(cur_meshable);
            return cur_meshable;
        }                                                                                                       
        Meshable *MeshCapture::current() {
            return cur_meshable;
        }
        void MeshCapture::release() {
            if (cur_meshable != nullptr) {
                release_meshable(cur_meshable);
                cur_meshable = nullptr;
            }
        };

    }
}