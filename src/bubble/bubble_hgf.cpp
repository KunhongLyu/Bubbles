
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
        V = MatrixXd::Zero(n, 3);
        this->volume = calculateVolume();

        sinceLastUpdate = 0.0;
    }



    void BubbleHGF::update(double dt) {
        forwardKinesmatics(dt);
        correctVolume();

        sinceLastUpdate += dt;

        if (sinceLastUpdate > 0.1) {
            regularizeMesh();
            sinceLastUpdate = 0.0;
        }
    }

    void BubbleHGF::regularizeMesh() {

        // TODO:
        // maybe use tthe isotropic remeshing algorithm?
        const int outerIterations = 5;
        const int smoothingSteps = 10;
        const double maxEdgeRatio = 4.0 / 3.0;
        const double minEdgeRatio = 4.0 / 5.0;
        double meanEdgeLength = calculateMeanEdgeLength();


        for (int i = 0; i < outerIterations; i++) {
            splitLongEdges(meanEdgeLength * maxEdgeRatio);
            //collapseShortEdges(meanEdgeLength * minEdgeRatio);
            /*flipEdgesForDegree();

            for (int j = 0; j < smoothingSteps; j++) {
                tangentialSmoothing(0.2); 
            }
            */
            meanEdgeLength = calculateMeanEdgeLength();
        }
        this->topologyUpdated = true;
   
    }



    double BubbleHGF::calculateMeanEdgeLength() const {
        double totalLength = 0.0;
        int edgeCount = 0;

        for (EdgeCIter e = edgesBegin(); e != edgesEnd(); ++e) {
            totalLength += e->length();
            edgeCount++;
        }

        return totalLength / edgeCount;
    }


    void BubbleHGF::splitLongEdges(double threshold) {
       
        std::unordered_map<Vertex*, size_t> vidx;
        size_t current_vertex_count = nVertices();
        vidx.reserve(current_vertex_count * 2);
        std::cout << "vidx size: edges " << current_vertex_count << std::endl;

        size_t idx = 0;
        for (auto v = verticesBegin(); v != verticesEnd(); ++v, ++idx) {
            vidx[&(*v)] = idx;
        }
        
    
        vector<EdgeIter> edgesToSplit;

        for (EdgeIter e = edgesBegin(); e != edgesEnd(); ++e) {
            if (e->length() > threshold) {
                edgesToSplit.push_back(e);
            }
        }
        std::cout << "Found " << edgesToSplit.size() << " edges to split" << std::endl;
        std::cout << "Before splitting: vertices.size() = " << vertices.size()
            << ", V.rows() = " << V.rows()
            << ", Minv.size() = " << Minv.size() << std::endl;

        for (EdgeIter e : edgesToSplit) {
            Vertex* raw_v0 = &(*e->halfedge()->vertex());
            Vertex* raw_v1 = &(*e->halfedge()->twin()->vertex());

            // Verify vertices exist in map
            if (vidx.count(raw_v0) == 0 || vidx.count(raw_v1) == 0) {
                throw std::runtime_error("Vertex not found in index map");
            }
            size_t i0 = vidx[raw_v0];
            size_t i1 = vidx[raw_v1];
            if (Minv.size() != vertices.size()) {
                std::cout << "Resizing Minv to match vertices\n";
                Minv.resize(vertices.size());
                Minv.setConstant(1.0); 
            }

            const double epsilon = 1e-10;
            double m0 = 1.0 / std::max(Minv(i0), epsilon);

            double m1 = 1.0 / std::max(Minv(i1), epsilon);

            VertexIter newVert = splitEdge(e);
            newVert->position = 0.5 * (raw_v0->position + raw_v1->position);
            newVert->computeNormal();

            size_t new_idx = V.rows();
            V.conservativeResize(new_idx + 1, 3);
            Minv.conservativeResize(new_idx + 1);

            // interpolate velocity
            V.row(new_idx) = (m0 * V.row(i0) + m1 * V.row(i1)) / (m0 + m1);
            Minv(new_idx) = 1.0 / (m0 + m1);

            vidx[&(*newVert)] = new_idx;
        }
        std::cout << "After splitting: vertices.size() = " << vertices.size()
            << ", V.rows() = " << V.rows()
            << ", Minv.size() = " << Minv.size() << std::endl;

    }

    void BubbleHGF::collapseShortEdges(double threshold) {
        vector<EdgeIter> edgesToCollapse;

        for (EdgeIter e = edgesBegin(); e != edgesEnd(); ++e) {
            if (e->length() < threshold && !e->isBoundary()) {
                edgesToCollapse.push_back(e);
            }
        }
        std::cout << "Found " << edgesToCollapse.size() << " edges to collapse" << std::endl;

        for (EdgeIter e : edgesToCollapse) {
            Vector3D avgPos = 0.5 * (e->halfedge()->vertex()->position +
                e->halfedge()->twin()->vertex()->position);

            VertexIter newVert = collapseEdge(e); 
              newVert->position = avgPos;
              newVert->computeNormal();

            }
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
            double tetrahedronVolume = dot(v0, cross(v1, v2));
            volume += tetrahedronVolume;
        }

        return volume / 6.0;
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
		std::cout << "vidx size: remesh" << n << std::endl;
        if (n == 0) return;

        auto start = std::chrono::high_resolution_clock::now();

        // Create vertex index map
        std::unordered_map<VertexIter, size_t, VertexIterHash, VertexIterEq> vidx;
        vidx.reserve(n);
        size_t idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            vidx[v] = idx;
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        VectorXd Minv = VectorXd::Zero(n);  // Mass matrix
        MatrixXd L = MatrixXd::Zero(n, n);  // Laplacian matrix
        MatrixXd X(n, 3);                   // Position matrix

        //  mass matrix
        for (FaceIter f = facesBegin(); f != facesEnd(); ++f) {
            // closed manifold => all faces are real triangles
            HalfedgeIter h0 = f->halfedge();
            HalfedgeIter h1 = h0->next();
            HalfedgeIter h2 = h1->next();

            const Vector3D p0 = h0->vertex()->position;
            const Vector3D p1 = h1->vertex()->position;
            const Vector3D p2 = h2->vertex()->position;

            // need to divide by 2
            const double area = cross(p1 - p0, p2 - p0).norm();

            const size_t i0 = vidx[h0->vertex()];
            const size_t i1 = vidx[h1->vertex()];
            const size_t i2 = vidx[h2->vertex()];

            // need to divide by 3
            const double share = area;
            Minv(i0) += share; Minv(i1) += share; Minv(i2) += share;
        }
        // hence multiply by 6
        for (size_t i = 0; i < n; ++i) Minv(i) = 6.0 / std::max(Minv(i), 1e-12);

        auto t2 = std::chrono::high_resolution_clock::now();


        //  Laplacian matrix

        auto cotangent = [](HalfedgeIter h) {
            // in h->face(), angle at the vertex opposite the edge h
            const Vector3D pi = h->vertex()->position;                // opposite
            const Vector3D pj = h->next()->vertex()->position;
            const Vector3D pk = h->next()->next()->vertex()->position;
            const Vector3D u = pi - pk, v = pj - pk;
            const Vector3D cx = cross(u, v);
            const double nrm = cx.norm();
            if (nrm < 1e-14) return 0.0; // degenerate guard
            return dot(u, v) / nrm;
            };

        for (EdgeIter e = edgesBegin(); e != edgesEnd(); ++e) {
            HalfedgeIter h = e->halfedge();

            if (h->isBoundary() || h->twin()->isBoundary()) {
                cout << "SKIPPED!!!!";
                continue;
            }

            // vertices on either side of the edge
            int i = vidx[h->vertex()];
            int j = vidx[h->twin()->vertex()];

            // opposite angles alpha, beta
            double cotA = cotangent(h);
            double cotB = cotangent(h->twin());
            double w = 0.5 * (cotA + cotB);

            // accumulate into L
            L(i, j) += w;
            L(j, i) += w;
            L(i, i) -= w;
            L(j, j) -= w;
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
        MatrixXd newV = V + dt * (Minv.asDiagonal() * L * X);
        MatrixXd newX = X + dt * newV;

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
        //cout << "time 1: ";
        //calcTime(t1 - start);
        //cout << "time 2: ";
        //calcTime(t2 - t1);
        //cout << "time 3: ";
        //calcTime(t3 - t2);
        //cout << "time 4: ";
        //calcTime(t4 - t3);
        //cout << "time 5: ";
        //calcTime(t5 - t4);
        cout << "total time";
        calcTime(t5 - start);
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

            v->computeNormal();
            vertices.push_back(v->normal.x);
            vertices.push_back(v->normal.y);
            vertices.push_back(v->normal.z);

            vertexMap[v] = i;
            i++;
        }

        for (const auto &f : parentHGF->faces) {
            indices.push_back(vertexMap[f.halfedge()->vertex()]);
            indices.push_back(vertexMap[f.halfedge()->next()->vertex()]);
            indices.push_back(vertexMap[f.halfedge()->next()->next()->vertex()]);
        }

        //auto addVertexNormal = [](vector<float> &verts, const Vector3D pos, const Vector3D normal) {
        //    verts.push_back(pos.x);
        //    verts.push_back(pos.y);
        //    verts.push_back(pos.z);
        //    verts.push_back(normal.x);
        //    verts.push_back(normal.y);
        //    verts.push_back(normal.z);
        //    };
        //
        //int nowN = vertices.size() / 6;
        //addVertexNormal(vertices, Vector3D(-1, -1, -1), Vector3D(0, 0, 1));
        //addVertexNormal(vertices, Vector3D(1, -1, -1), Vector3D(0, 0, 1));
        //addVertexNormal(vertices, Vector3D(1, 1, -1), Vector3D(0, 0, 1));
        //addVertexNormal(vertices, Vector3D(-1, 1, -1), Vector3D(0, 0, 1));
        //
        //addVertexNormal(vertices, Vector3D(-1, -1, -1), Vector3D(0, 1, 0));
        //addVertexNormal(vertices, Vector3D(1, -1, -1), Vector3D(0, 1, 0));
        //addVertexNormal(vertices, Vector3D(1, -1, 1), Vector3D(0, 1, 0));
        //addVertexNormal(vertices, Vector3D(-1, -1, 1), Vector3D(0, 1, 0));


        //indices.push_back(nowN);
        //indices.push_back(nowN+1);
        //indices.push_back(nowN+2);
        //indices.push_back(nowN);
        //indices.push_back(nowN+2);
        //indices.push_back(nowN+3);
        //indices.push_back(nowN+4);
        //indices.push_back(nowN+5);
        //indices.push_back(nowN+6);
        //indices.push_back(nowN+4);
        //indices.push_back(nowN+6);
        //indices.push_back(nowN+7);



        MeshCPUBuffer *meshBuffer = new MeshCPUBuffer(inputFormat,
            vertices.data(), parentHGF->vertices.size(),
            indices.data(), Type_UInt, indices.size(),
            Usage_DynamicDraw, Usage_StaticDraw);
        meshBuffer->vertices = vertices;

        *ptr = meshBuffer;
    }
    void BubbleHGF::HGFMeshCapture::update_cur_ptr(MeshBuffer **ptr) const {
        if (*ptr == nullptr) {
            create_ptr(ptr);
            return;
        }

        if (parentHGF->topologyUpdated) {
            parentHGF->topologyUpdated = false;
            parentHGF->verticesUpdated = false;
            if (*ptr != nullptr) {
                release_ptr(*ptr);
                *ptr = nullptr;
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
            MeshCPUBuffer *meshBuffer = (MeshCPUBuffer *)(ptr);
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
        BubbleBSDF *bubbleBSDF;
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
        ptrMesh->bubbleBSDF = new BubbleBSDF();

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

        //ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, -1, -1), Vector3D(1, 1, -1),
        //    Vector3D(0, 0, 1), Vector3D(0, 0, 1), Vector3D(0, 0, 1), ptrMesh->bubbleBSDF));
        //ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, 1, -1), Vector3D(-1, 1, -1),
        //    Vector3D(0, 0, 1), Vector3D(0, 0, 1), Vector3D(0, 0, 1), ptrMesh->bubbleBSDF));
        //
        //ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, -1, -1), Vector3D(1, -1, 1),
        //    Vector3D(0, 1, 0), Vector3D(0, 1, 0), Vector3D(0, 1, 0), ptrMesh->bubbleBSDF));
        //ptrMesh->bubble_faces.push_back(new Triangle(Vector3D(-1, -1, -1), Vector3D(1, -1, 1), Vector3D(-1, -1, 1),
        //    Vector3D(0, 1, 0), Vector3D(0, 1, 0), Vector3D(0, 1, 0), ptrMesh->bubbleBSDF));

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