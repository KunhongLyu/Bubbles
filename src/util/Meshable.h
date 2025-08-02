
#ifndef __MESHABLE_H__
#define __MESHABLE_H__

#include <vector>
#include "../pathtracer/primitive.h"


namespace CGL {
    namespace SceneObjects {

        class Meshable {

        public:
            virtual std::vector<Primitive *>::const_iterator begin() const = 0;
            virtual std::vector<Primitive *>::const_iterator end() const = 0;
            virtual std::vector<Primitive *>::iterator begin() = 0;
            virtual std::vector<Primitive *>::iterator end() = 0;
        };

        class MeshCapture {
            MeshCapture();
            ~MeshCapture();
            Meshable *capture();
            Meshable *current();
            void release();
        protected:
            virtual Meshable *create_meshable() const = 0;
            virtual Meshable *update_cur_meshable(Meshable *) const = 0;
            virtual void release_meshable(Meshable *) const = 0;

            Meshable *cur_meshable;
        };
    }
}


#endif //__MESHABLE_H__