
#include "bubble_hgf.h"
#include "../pathtracer/bubble_bsdf.h"
#include "../pathtracer/triangle.h"
#include <unordered_map>
#include <chrono>


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






    void BubbleHGF::build(const vector<vector<Index> > &polygons,
        const vector<Vector3D> &vertexPositions,
        const vector<Vector2D> &texcoords) {

        HalfedgeMesh::build(polygons, vertexPositions, texcoords);

        using namespace Eigen;

        size_t n = nVertices();
        V = MatrixXd(n, 3);
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

        return volume;
    }


    struct VertexIterHash {
        size_t operator()(const VertexIter &it) const noexcept {
            return std::hash<const Vertex *>()(&*it);
        }
    };

    struct VertexIterEq {
        bool operator()(const VertexIter &a, const VertexIter &b) const noexcept {
            return &*a == &*b;
        }
    };

    void BubbleHGF::forwardKinesmatics(double dt) {

        using namespace Eigen;

        // TODO:
        // Calculate the M, L, and X matrices, then
        // update the positions of the vertices based on these
        // matrices
        size_t n = vertices.size();
        if (n == 0) return;

        auto start = std::chrono::high_resolution_clock::now();

        // Create vertex index map
        std::unordered_map<VertexIter, size_t, VertexIterHash, VertexIterEq> vidx;
        size_t idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            vidx[v] = idx;
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        VectorXd M = VectorXd::Zero(n);  // Mass matrix
        MatrixXd L = MatrixXd::Zero(n, n);  // Laplacian matrix
        MatrixXd X(n, 3);                   // Position matrix

        //  mass matrix
        for (FaceIter f = facesBegin(); f != facesEnd(); ++f) {
            if (f->isBoundary()) continue;
            HalfedgeIter h0 = f->halfedge();
            HalfedgeIter h1 = h0->next();
            HalfedgeIter h2 = h1->next();
            // positions
            Vector3D p0 = h0->vertex()->position;
            Vector3D p1 = h1->vertex()->position;
            Vector3D p2 = h2->vertex()->position;
            // triangle area
            double area = 0.5 * cross(p1 - p0, p2 - p0).norm();
            // indices
            int i0 = vidx[h0->vertex()];
            int i1 = vidx[h1->vertex()];
            int i2 = vidx[h2->vertex()];
            double a3 = area / 3.0;
            M(i0) += a3;
            M(i1) += a3;
            M(i2) += a3;
        }
        for (size_t i = 0; i < n; ++i) {
            M(i) = 1.0 / std::max(M(i), 1e-6);
        }
        auto t2 = std::chrono::high_resolution_clock::now();


        //  Laplacian matrix
        for (EdgeIter e = edgesBegin(); e != edgesEnd(); ++e) {
            HalfedgeIter h = e->halfedge();
            if (h->isBoundary() || h->twin()->isBoundary()) continue;

            // vertices on either side of the edge
            int i = vidx[h->vertex()];
            int j = vidx[h->twin()->vertex()];

            // opposite angles alpha, beta
            auto cotangent = [](HalfedgeIter h) {
                // in h->face(), angle at the vertex opposite the edge h
                Vector3D a = h->next()->vertex()->position - h->vertex()->position;
                Vector3D b = h->next()->next()->vertex()->position - h->vertex()->position;
                // cot = (a¡¤b) / |a¡Áb|
                return dot(a, b) / cross(a, b).norm();
                };
            double cotA = cotangent(h->next());
            double cotB = cotangent(h->twin()->next());
            double w = 0.5 * (cotA + cotB);

            // accumulate into L
            L(i, j) -= w;
            L(j, i) -= w;
            L(i, i) += w;
            L(j, j) += w;
        }
        auto t3 = std::chrono::high_resolution_clock::now();

        //  position matrix
        idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            const auto &p = v->position;
            X(idx, 0) = p.x;  X(idx, 1) = p.y;  X(idx, 2) = p.z;
        }

        auto t4 = std::chrono::high_resolution_clock::now();

        // update positions
        MatrixXd newV = V + dt * (M.asDiagonal() * L * X);
        MatrixXd newX = X + dt * V;

        V = newV;
        
        // apply new positions
        idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            v->position.x = newX(idx, 0);
            v->position.y = newX(idx, 1);
            v->position.z = newX(idx, 2);
        }
        auto t5 = std::chrono::high_resolution_clock::now();

        verticesUpdated = true;

        auto calcTime = [](std::chrono::nanoseconds s) { cout << std::chrono::duration<double>(s).count() << endl; };
        calcTime(t1 - start);
        calcTime(t2 - t1);
        calcTime(t3 - t2);
        calcTime(t4 - t3);
        calcTime(t5 - t4);
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


            vertices.push_back(v->normal.x);
            vertices.push_back(v->normal.y);
            vertices.push_back(v->normal.z);

            vertexMap[v] = i;
            i++;
        }

        auto addVertexNormal = [](vector<float> &verts, const Vector3D pos, const Vector3D normal) {
            verts.push_back(pos.x);
            verts.push_back(pos.y);
            verts.push_back(pos.z);
            verts.push_back(normal.x);
            verts.push_back(normal.y);
            verts.push_back(normal.z);
            };

        int nowN = vertices.size() / 6;
        addVertexNormal(vertices, Vector3D(-1, -1, -1), Vector3D(0, 0, 1));
        addVertexNormal(vertices, Vector3D(1, -1, -1), Vector3D(0, 0, 1));
        addVertexNormal(vertices, Vector3D(1, 1, -1), Vector3D(0, 0, 1));
        addVertexNormal(vertices, Vector3D(-1, 1, -1), Vector3D(0, 0, 1));

        addVertexNormal(vertices, Vector3D(-1, -1, -1), Vector3D(0, 1, 0));
        addVertexNormal(vertices, Vector3D(1, -1, -1), Vector3D(0, 1, 0));
        addVertexNormal(vertices, Vector3D(1, -1, 1), Vector3D(0, 1, 0));
        addVertexNormal(vertices, Vector3D(-1, -1, 1), Vector3D(0, 1, 0));


        for (const auto &f : parentHGF->faces) {
            indices.push_back(vertexMap[f.halfedge()->vertex()]);
            indices.push_back(vertexMap[f.halfedge()->next()->vertex()]);
            indices.push_back(vertexMap[f.halfedge()->next()->next()->vertex()]);
        }

        indices.push_back(nowN);
        indices.push_back(nowN+1);
        indices.push_back(nowN+2);
        indices.push_back(nowN);
        indices.push_back(nowN+2);
        indices.push_back(nowN+3);
        indices.push_back(nowN+4);
        indices.push_back(nowN+5);
        indices.push_back(nowN+6);
        indices.push_back(nowN+4);
        indices.push_back(nowN+6);
        indices.push_back(nowN+7);



        MeshCPUBuffer *meshBuffer = new MeshCPUBuffer(inputFormat,
            vertices.data(), parentHGF->vertices.size()+8,
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

            meshBuffer->vertices[i * 6 + 3] = v->normal.x;
            meshBuffer->vertices[i * 6 + 4] = v->normal.y;
            meshBuffer->vertices[i * 6 + 5] = v->normal.z;

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
            MeshCPUBuffer *meshBuffer = static_cast<MeshCPUBuffer *>(ptr);
            delete meshBuffer;
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
        DiffuseBSDF *bubbleBSDF;
    };


    void BubbleHGF::HGFPathtracerCapture::create_ptr(MeshablePathtracer **ptr) const {
        if (*ptr != nullptr) {
            release_ptr(*ptr);
        }

        BubblePathtracerMesh *ptrMesh = new BubblePathtracerMesh();

        // TODO fill the ptrMesh->bubble_faces vector with the triangles which
        // are the faces of the bubble from this->parentHGF->faces, then use
        // BubbleBSDF for the BSDF of each triangle.
        ptrMesh->bubble_faces.reserve(parentHGF->faces.size() + 4);

        // Create a BubbleBSDF material for all faces
        ptrMesh->bubbleBSDF = new DiffuseBSDF(Vector3D(1, 1, 1));

        for (const auto& face : parentHGF->faces) {
            // Get the three vertices of the triangle
            Vector3D v0 = face.halfedge()->vertex()->position;
            Vector3D v1 = face.halfedge()->next()->vertex()->position;
            Vector3D v2 = face.halfedge()->next()->next()->vertex()->position;

            Vector3D n1 = face.halfedge()->vertex()->normal;
            Vector3D n2 = face.halfedge()->next()->vertex()->normal;
            Vector3D n3 = face.halfedge()->next()->next()->vertex()->normal;

            // Create a triangle primitive with the BubbleBSDF
            Triangle* triangle = new Triangle(v0, v1, v2, n1, n2, n3, ptrMesh->bubbleBSDF);
            ptrMesh->bubble_faces.push_back(triangle);
        }

        ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, -1, -1), Vector3D(1, 1, -1),
            Vector3D(0, 0, 1), Vector3D(0, 0, 1), Vector3D(0, 0, 1), ptrMesh->bubbleBSDF));
        ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, 1, -1), Vector3D(-1, 1, -1),
            Vector3D(0, 0, 1), Vector3D(0, 0, 1), Vector3D(0, 0, 1), ptrMesh->bubbleBSDF));

        ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, -1, -1), Vector3D(1, -1, 1),
            Vector3D(0, 1, 0), Vector3D(0, 1, 0), Vector3D(0, 1, 0), ptrMesh->bubbleBSDF));
        ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, -1, 1), Vector3D(-1, -1, 1),
            Vector3D(0, 1, 0), Vector3D(0, 1, 0), Vector3D(0, 1, 0), ptrMesh->bubbleBSDF));

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
            delete ptrMesh->bubbleBSDF;
            delete ptrMesh;
        }
    }

} // namespace CGL