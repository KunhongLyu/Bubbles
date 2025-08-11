

#ifndef __BUBBLE_HGF_H__
#define __BUBBLE_HGF_H__

#include "bubble_dynamics.h"
#include "halfEdgeMesh.h"
#include <Eigen/Dense>


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

        void build(const vector<vector<Index> > &polygons,
            const vector<Vector3D> &vertexPositions,
            const vector<Vector2D> &texcoords);


        ObjPtrCapture<MeshBuffer> *getMeshCapture() const;
        ObjPtrCapture<MeshablePathtracer> *getMeshPathtracerCapture() const;

    private:


        class HGFMeshCapture : public ObjPtrCapture<MeshBuffer> {
        public:
            HGFMeshCapture(BubbleHGF *parentHGF) : parentHGF(parentHGF) {}
            ~HGFMeshCapture() { release(); };

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
            ~HGFPathtracerCapture() { release(); };

        protected:
            void create_ptr(MeshablePathtracer **) const;
            void update_cur_ptr(MeshablePathtracer **) const;
            void release_ptr(MeshablePathtracer *) const;

        private:
            BubbleHGF *parentHGF;
        };

        HGFMeshCapture *glMeshCapture;
        HGFPathtracerCapture *pathtracerCapture;

        Eigen::MatrixXd V;
        Eigen::VectorXd Minv;
		Eigen::MatrixXd M; 

        double calculateVolume() const;
        void forwardKinesmatics(double dt);
        void correctVolume();
        void regularizeMesh();
         

        //remesh helper 
        double calculateMeanEdgeLength() const;
        void splitLongEdges(double threshold); 
        void collapseShortEdges(double threshold);
        bool BubbleHGF::collectIncidentEdges(VertexIter v, EdgeIter collapsingEdge, std::vector<EdgeIter>& out);
        //void flipEdgesForRegularDegree(); 
        //bool shouldFlipEdge(EdgeIter e) const; 
        //void tangentialSmoothing(double weight); 

		//void updateMeshBuffers();
        double sinceLastUpdate;
        //void rebuildPhysicsMatrices(); 

        double volume; // volume of the bubble, kept constant


        // not thread safe flags
        bool verticesUpdated = false; ///< whether the mesh has been updated since the last capture
        bool topologyUpdated = false; ///< whether the mesh has been updated since the last capture
    };
} // namespace CGL




#endif // __BUBBLE_HGF_H__