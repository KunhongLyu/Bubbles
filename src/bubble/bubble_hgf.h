

#ifndef __BUBBLE_HGF_H__
#define __BUBBLE_HGF_H__

#include "bubble_dynamics.h"
#include "halfEdgeMesh.h"


namespace CGL {
    /**
     * Bubble HGF is a half-edge mesh with bubble dynamics.
     * It is used to simulate the behavior of bubbles in a fluid.
     */
    class BubbleHGF : public HalfedgeMesh, public BubbleDynamics {
    public:
        // TODO we probably need to have some initial mesh for this
        BubbleHGF();
        ~BubbleHGF();
        void update(double dt);


        ObjPtrCapture<MeshBuffer> *getMeshCapture() const;
        ObjPtrCapture<MeshablePathtracer> *getMeshPathtracerCapture() const;

    private:


        class HGFMeshCapture : public ObjPtrCapture<MeshBuffer> {
        public:
            HGFMeshCapture(BubbleHGF *parentHGF) : parentHGF(parentHGF) {}
            ~HGFMeshCapture() {};

        protected:
            void create_ptr(MeshBuffer **) const;
            void update_cur_ptr(MeshBuffer **) const;
            void release_ptr(MeshBuffer *) const;
        private:
            BubbleHGF *parentHGF;
        };


        class HGFPathtracerCapture : public ObjPtrCapture<MeshablePathtracer> {
        public:
            HGFPathtracerCapture(BubbleHGF *parentHGF) : parentHGF(parentHGF) {}
            ~HGFPathtracerCapture() {};

        protected:
            void create_ptr(MeshablePathtracer **) const;
            void update_cur_ptr(MeshablePathtracer **) const;
            void release_ptr(MeshablePathtracer *) const;

        private:
            BubbleHGF *parentHGF;
        };

        HGFMeshCapture *glMeshCapture;
        HGFPathtracerCapture *pathtracerCapture;


        double calculateVolume() const;
        void forwardKinesmatics(double dt);
        void correctVolume();
        

        double volume; // volume of the bubble, kept constant


        // not thread safe flags
        bool verticesUpdated = false; ///< whether the mesh has been updated since the last capture
        bool topologyUpdated = false; ///< whether the mesh has been updated since the last capture
    };
} // namespace CGL




#endif // __BUBBLE_HGF_H__