
#include "bubble_hgf.h"
#include "../pathtracer/bubble_bsdf.h"
#include "../pathtracer/triangle.h"



namespace CGL {

    // TODO see header comment
    BubbleHGF::BubbleHGF() {
        // Initialize the HGF parameters
        glMeshCapture = new HGFMeshCapture(this);
        pathtracerCapture = new HGFPathtracerCapture(this);
    }
    BubbleHGF::~BubbleHGF() {
        // Cleanup if necessary
        delete glMeshCapture;
        delete pathtracerCapture;
    }







    void BubbleHGF::update(double dt) {
        forwardKinesmatics();
        correctVolume();
    }


    double BubbleHGF::calculateVolume() const {
        // Calculate the volume of the bubble
        double volume = 0.0;

        // TODO:
        // use divergence theorem on each face to calculate the volume

        return volume;
    }

    void BubbleHGF::forwardKinesmatics() {

        // TODO:
        // Calculate the M, L, and X matrices, then
        // update the positions of the vertices based on these
        // matrices
    }
    void BubbleHGF::correctVolume() {
        double currentVolume = calculateVolume();
        double scale = pow(this->volume / currentVolume, 1.0 / 3.0);

        // TODO:
        // Correct the volume of the bubble by adjusting the vertices
        // so that the volume is equal to this->volume,
        // probably just scale the vertices by the scale factor
    }








    const ObjPtrCapture<MeshBuffer> *BubbleHGF::getMeshCapture() const {
        return glMeshCapture;
    };
    const ObjPtrCapture<MeshablePathtracer> *BubbleHGF::getMeshPathtracerCapture() const {
        return pathtracerCapture;
    }


    void HGFMeshCapture::create_ptr(MeshBuffer **ptr) const {
        if (*ptr != nullptr)
            release_ptr(*ptr);

        vector<ShaderInput> inputFormat = {
            { Type_Float, 3 }, // Position
            { Type_Float, 3 }, // Normal
        };

        vertices.clear();
        vector<uint32_t> indices;

        map<VertexIter, uint32_t> vertexMap;

        vertices.reserve(parentHGF->vertices.size() * 6);
        indices.reserve(parentHGF->faces.size() * 3);

        int i = 0;
        for (auto v = parentHGF->vertices.begin(); v != parentHGF->vertices.end(); v++) {

            vertices.push_back(v->x);
            vertices.push_back(v->y);
            vertices.push_back(v->z);

            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);

            vertexMap[v] = i;
            i++;
        }

        for (const auto &f : parentHGF->faces) {
            indices.push_back(vertexMap[f->halfedge()->vertex()]);
            indices.push_back(vertexMap[f->halfedge()->next()->vertex()]);
            indices.push_back(vertexMap[f->halfedge()->next()->next()->vertex()]);
        }

        *ptr = new MeshBuffer(inputFormat,
            vertices.data(), parentHGF->vertices.size(),
            indices.data(), Type_UInt, indices.size(),
            Usage_DynamicDraw, Usage_StaticDraw);
    }
    void HGFMeshCapture::update_cur_ptr(MeshBuffer **ptr) const {
        if (*ptr == nullptr) {
            create_ptr(ptr);
            return;
        }

        if (parentHGF->topologyUpdated) {
            parentHGF->topologyUpdated = false;
            if (*ptr != nullptr) {
                release_ptr(*ptr);
            }
            create_ptr(ptr);
            return;
        }

        if (!parentHGF->verticesUpdated)
            return;

        int i = 0;
        for (auto v = parentHGF->vertices.begin(); v != parentHGF->vertices.end(); v++) {

            vertices[i * 6 + 0] = v->x;
            vertices[i * 6 + 1] = v->y;
            vertices[i * 6 + 2] = v->z;

            i++;
        }

        if (*ptr == nullptr) {
            throw std::runtime_error("MeshBuffer pointer is null.");
        }
        if ((*ptr)->vertexCount != parentHGF->vertices.size()) {
            throw std::runtime_error("MeshBuffer vertex count mismatch.");
        }
        if ((*ptr)->indexCount != parentHGF->faces.size() * 3) {
            throw std::runtime_error("MeshBuffer index count mismatch.");
        }
        (*ptr)->updateVertex(vertices.data(), 0, parentHGF->vertices.size());
        parentHGF->verticesUpdated = false;
    }
    void HGFMeshCapture::release_ptr(MeshBuffer *ptr) const {
        if (ptr != nullptr) {
            delete ptr;
        }
    }



    class BubblePathtracerMesh : public MeshablePathtracer {
    public:
        vector<Primitive *>::const_iterator begin() const {
            return bubble_faces.begin();
        }
        vector<Primitive *>::const_iterator end() const {
            return bubble_faces.end();
        }
        vector<Primitive *>::iterator begin() {
            return bubble_faces.begin();
        }
        vector<Primitive *>::iterator end() {
            return bubble_faces.end();
        }

        vector<Primitive *> bubble_faces;
    };


    void HGFPathtracerCapture::create_ptr(MeshablePathtracer **ptr) const {
        if (*ptr != nullptr) {
            release_ptr(*ptr);
        }

        BubblePathtracerMesh *ptrMesh = new BubblePathtracerMesh();

        // TODO fill the ptrMesh->bubble_faces vector with the triangles which
        // are the faces of the bubble from this->parentHGF->faces, then use
        // BubbleBSDF for the BSDF of each triangle.
        

        *ptr = ptrMesh;
    }
    void HGFPathtracerCapture::update_cur_ptr(MeshablePathtracer **ptr) const {
        if (*ptr == nullptr) {
            create_ptr(ptr);
            return;
        }

        if (parentHGF->topologyUpdated) {
            parentHGF->topologyUpdated = false;
            if (*ptr != nullptr) {
                release_ptr(*ptr);
            }
            create_ptr(ptr);
            return;
        }

        if (!parentHGF->verticesUpdated)
            return;


        BubblePathtracerMesh *ptrMesh = *ptr;

        // TODO
        // See HGFMeshCapture::update_cur_ptr for an example of how to update the
        // MeshBuffer pointer. Here we need to update the BubblePathtracerMesh.
        // Basically here just update all the triangles in ptrMesh->bubble_faces
        // with the vertices from parentHGF->vertices.
        // can probably reuse the logic from HGFMeshCapture::create_ptr

    }
    void HGFPathtracerCapture::release_ptr(MeshablePathtracer *ptr) const {
        if (ptr != nullptr) {
            BubblePathtracerMesh *ptrMesh = dynamic_cast<BubblePathtracerMesh *>(ptr);
            delete ptrMesh;
        }
    }

} // namespace CGL