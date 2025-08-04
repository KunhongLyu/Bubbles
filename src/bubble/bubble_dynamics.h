
#ifndef __MESH_H__
#define __MESH_H__


#include "../util/objptr_capture.h"
#include "../util/mesh_buffer.h"
#include "../pathtracer/bvh.h"

namespace CGL {

    class BubbleDynamics {
    public:

        virtual void update(double dt) = 0;

        virtual const ObjPtrCapture<MeshBuffer> *getMeshCapture() const = 0;
        virtual const ObjPtrCapture<MeshablePathtracer> *getMeshPathtracerCapture() const = 0;
    };
} // namespace CGL



#endif //__MESH_H__