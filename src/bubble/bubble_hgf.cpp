
#include "bubble_hgf.h"
#include "../pathtracer/bubble_bsdf.h"
#include "../pathtracer/triangle.h"
#include <Eigen/Dense> 


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
        forwardKinesmatics(dt);
        correctVolume();
    }



    double BubbleHGF::calculateVolume() const {
        // Calculate the volume of the bubble
        double volume = 0.0;

        // TODO:done
        // use divergence theorem on each face to calculate the volume Vtetr = (V0 DOT (v1 x v2) )


            // Get v for each face
        for (FaceCIter f = faces.begin(); f != faces.end(); f++) {
            HalfedgeCIter h = f->halfedge();
            Vector3D v0 = h->vertex()->position;
            Vector3D v1 = h->next()->vertex()->position;
            Vector3D v2 = h->next()->next()->vertex()->position;

            // calc the signed volume of the tetrahedron formed by the triangle and the origin
            double tetrahedronVolume = dot(v0, cross(v1, v2)) / 6.0;
            volume += tetrahedronVolume;
        }
    }

    void BubbleHGF::forwardKinesmatics(double dt) {

        using namespace Eigen;

        // TODO:
        // Calculate the M, L, and X matrices, then
        // update the positions of the vertices based on these
        // matrices
        size_t n = vertices.size();
        if (n == 0) return;

        // Create vertex index map
        std::unordered_map<VertexIter, size_t> vertexIndices;
        size_t idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            vertexIndices[v] = idx;
        }

        MatrixXd M = MatrixXd::Zero(n, n);  // Mass matrix
        MatrixXd L = MatrixXd::Zero(n, n);  // Laplacian matrix
        MatrixXd X(n, 3);                   // Position matrix

        //  mass matrix
        double m = 1.0 / n;
        for (size_t i = 0; i < n; ++i) {
            M(i, i) = m;
        }

        //  position matrix
        idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            X.row(idx) << v->position.x, v->position.y, v->position.z;
        }

        //  Laplacian matrix
        for (auto e = edges.begin(); e != edges.end(); ++e) {
            size_t i = vertexIndices[e->halfedge()->vertex()];
            size_t j = vertexIndices[e->halfedge()->twin()->vertex()];
            L(i, j) = -1.0;
            L(j, i) = -1.0;
            L(i, i) += 1.0;
            L(j, j) += 1.0;
        }

        // update positions
        MatrixXd newX = X + dt * (M.inverse() * L * X);

        // apply new positions
        idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            v->position.x = newX(idx, 0);
            v->position.y = newX(idx, 1);
            v->position.z = newX(idx, 2);
        }

        verticesUpdated = true;
    }


    void BubbleHGF::correctVolume() {
        double currentVolume = calculateVolume();
        if (currentVolume <= 0.0) return;
        double scale = pow(this->volume / currentVolume, 1.0 / 3.0);

        // TODO:
        // Correct the volume of the bubble by adjusting the vertices
        // so that the volume is equal to this->volume,
        // probably just scale the vertices by the scale factor
        Vector3D center(0.0, 0.0, 0.0);

        // calc center of mass
        for (auto v = vertices.begin(); v != vertices.end(); ++v) {
            center += v->position;
        }
        center /= vertices.size();

        // Scale vertices about center
        for (auto v = vertices.begin(); v != vertices.end(); ++v) {
            v->position = center + scale * (v->position - center);
        }

        verticesUpdated = true;
    }








    ObjPtrCapture<MeshBuffer> *BubbleHGF::getMeshCapture() const {
        return glMeshCapture;
    };
    ObjPtrCapture<MeshablePathtracer> *BubbleHGF::getMeshPathtracerCapture() const {
        return pathtracerCapture;
    }

    class MeshCPUBuffer : public MeshBuffer {
    public:

        MeshCPUBuffer(const std::vector<ShaderInput> &inputFormat, void *vertexData, size_t vertexCount, void *indexData, InputType indexFormat, size_t indexCount, BufferUsage vertexUsage, BufferUsage indexUsage) :
            MeshBuffer(inputFormat, vertexData, vertexCount, indexData, indexFormat, indexCount, vertexUsage, indexUsage) {}
        ~MeshCPUBuffer() {};

        vector<float> vertices;
    };

    void BubbleHGF::HGFMeshCapture::create_ptr(MeshBuffer **ptr) const {
        if (*ptr != nullptr)
            release_ptr(*ptr);

        vector<ShaderInput> inputFormat = {
            { Type_Float, 3 }, // Position
            { Type_Float, 3 }, // Normal
        };


        vector<float> vertices;
        vector<uint32_t> indices;

        map<VertexCIter, uint32_t> vertexMap;

        vertices.reserve(parentHGF->vertices.size() * 6);
        indices.reserve(parentHGF->faces.size() * 3);

        int i = 0;
        for (auto v = parentHGF->vertices.begin(); v != parentHGF->vertices.end(); v++) {

            vertices.push_back(v->position.x);
            vertices.push_back(v->position.y);
            vertices.push_back(v->position.z);

            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);

            vertexMap[v] = i;
            i++;
        }

        for (const auto &f : parentHGF->faces) {
            indices.push_back(vertexMap[f.halfedge()->vertex()]);
            indices.push_back(vertexMap[f.halfedge()->next()->vertex()]);
            indices.push_back(vertexMap[f.halfedge()->next()->next()->vertex()]);
        }

        MeshCPUBuffer *meshBuffer = new MeshCPUBuffer(inputFormat,
            vertices.data(), parentHGF->vertices.size(),
            indices.data(), Type_UInt, indices.size(),
            Usage_DynamicDraw, Usage_StaticDraw);
        meshBuffer->vertices = std::move(vertices);

        *ptr = meshBuffer;
    }
    void BubbleHGF::HGFMeshCapture::update_cur_ptr(MeshBuffer **ptr) const {
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

        MeshCPUBuffer *meshBuffer = static_cast<MeshCPUBuffer *>(*ptr);

        int i = 0;
        for (auto v = parentHGF->vertices.begin(); v != parentHGF->vertices.end(); v++) {

            meshBuffer->vertices[i * 6 + 0] = v->position.x;
            meshBuffer->vertices[i * 6 + 1] = v->position.y;
            meshBuffer->vertices[i * 6 + 2] = v->position.z;

            i++;
        }

        if ((*ptr)->getVertexCount() != parentHGF->nVertices()) {
            throw std::runtime_error("MeshBuffer vertex count mismatch.");
        }
        if ((*ptr)->getIndexCount() != parentHGF->nFaces() * 3) {
            throw std::runtime_error("MeshBuffer index count mismatch.");
        }
        (*ptr)->updateVertex(meshBuffer->vertices.data(), 0, parentHGF->vertices.size());
        parentHGF->verticesUpdated = false;
    }
    void BubbleHGF::HGFMeshCapture::release_ptr(MeshBuffer *ptr) const {
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


    void BubbleHGF::HGFPathtracerCapture::create_ptr(MeshablePathtracer **ptr) const {
        if (*ptr != nullptr) {
            release_ptr(*ptr);
        }

        BubblePathtracerMesh *ptrMesh = new BubblePathtracerMesh();

        // TODO fill the ptrMesh->bubble_faces vector with the triangles which
        // are the faces of the bubble from this->parentHGF->faces, then use
        // BubbleBSDF for the BSDF of each triangle.
        ptrMesh->bubble_faces.reserve(parentHGF->faces.size());

        // Create a BubbleBSDF material for all faces
        BSDF* bubbleBSDF = new BubbleBSDF();

        for (const auto& face : parentHGF->faces) {
            // Get the three vertices of the triangle
            Vector3D v0 = face.halfedge()->vertex()->position;
            Vector3D v1 = face.halfedge()->next()->vertex()->position;
            Vector3D v2 = face.halfedge()->next()->next()->vertex()->position;

            Vector3D n1 = face.halfedge()->vertex()->normal;
            Vector3D n2 = face.halfedge()->vertex()->normal;
            Vector3D n3 = face.halfedge()->vertex()->normal;

            // Create a triangle primitive with the BubbleBSDF
            Triangle* triangle = new Triangle(v0, v1, v2, n1, n2, n3, bubbleBSDF);
            ptrMesh->bubble_faces.push_back(triangle);
        }

        *ptr = ptrMesh;
    }
    void BubbleHGF::HGFPathtracerCapture::update_cur_ptr(MeshablePathtracer **ptr) const {
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


        BubblePathtracerMesh *ptrMesh = static_cast<BubblePathtracerMesh *>(*ptr);

        // TODO
        // See HGFMeshCapture::update_cur_ptr for an example of how to update the
        // MeshBuffer pointer. Here we need to update the BubblePathtracerMesh.
        // Basically here just update all the triangles in ptrMesh->bubble_faces
        // with the vertices from parentHGF->vertices.
        // can probably reuse the logic from HGFMeshCapture::create_ptr

    }
    void BubbleHGF::HGFPathtracerCapture::release_ptr(MeshablePathtracer *ptr) const {
        if (ptr != nullptr) {
            BubblePathtracerMesh *ptrMesh = static_cast<BubblePathtracerMesh *>(ptr);
            delete ptrMesh;
        }
    }

} // namespace CGL