
#include "bubble_hgf.h"
#include "../pathtracer/bubble_bsdf.h"
#include "../pathtracer/triangle.h"
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <queue>

#include <unordered_set>


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

    Eigen::VectorXd Minv;




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

        if (sinceLastUpdate > 0.03) {
            regularizeMesh(true);
            sinceLastUpdate = 0.0;
        }
    }

    void BubbleHGF::regularizeMesh(bool full) {

        // TODO:
        // maybe use tthe isotropic remeshing algorithm?
        const int outerIterations = 5;
        const int smoothingSteps = 5;
        const double maxEdgeRatio = 4.0 / 3.0;
        const double minEdgeRatio = 4.0 / 5.0;
        double meanEdgeLength = calculateMeanEdgeLength();
        
        /*int targetCount = 0;
        for (auto v = verticesBegin(); v != verticesEnd(); ++v) {
            targetCount++; 
        }*/

        if (full) {
            for (int i = 0; i < outerIterations; i++) {
                collapseShortEdges(meanEdgeLength * minEdgeRatio);

                splitLongEdges(meanEdgeLength * maxEdgeRatio);

                //if (nVertices() < targetCount * 0.99) {
                //    splitLongEdges(meanEdgeLength * 1.5); // More splits
                //}
               

                for (int j = 0; j < smoothingSteps; j++) {
                    //flipEdgesForRegularDegree();
                    tangentialSmoothing(0.8, 0.1);
                }

                cout << "next round of outerIteration" << endl;
                meanEdgeLength = calculateMeanEdgeLength();
            }
        }
        else {
            collapseShortEdges(meanEdgeLength * minEdgeRatio);
        }
        this->topologyUpdated = true;
   
    }


    void BubbleHGF::flipEdgesForRegularDegree() {
        std::unordered_map<Vertex*, size_t> vidx;
        size_t idx = 0;
        int flippedCount = 0; 
        for (auto v = verticesBegin(); v != verticesEnd(); ++v, ++idx) {
            vidx[&(*v)] = idx;
        }

        double blend_factor = 0.5;

        std::vector<EdgeIter> edgesToProcess;
        for (EdgeIter e = edgesBegin(); e != edgesEnd(); ++e) {
            edgesToProcess.push_back(e);
        }

        for (EdgeIter e : edgesToProcess) {
           

            HalfedgeIter h = e->halfedge();
            HalfedgeIter h_twin = h->twin();

         
            Vertex* a1 = &(*h->vertex());       
            Vertex* a2 = &(*h_twin->vertex());  
            Vertex* b1 = &(*h->next()->next()->vertex());       
            Vertex* b2 = &(*h_twin->next()->next()->vertex());  

            int a1_degree = a1->degree();
            int a2_degree = a2->degree();
            int b1_degree = b1->degree();
            int b2_degree = b2->degree();

            int current_deviation = abs(a1_degree - 6) + abs(a2_degree - 6) +
                abs(b1_degree - 6) + abs(b2_degree - 6);

            int flipped_deviation = abs(a1_degree - 1 - 6) + 
                abs(a2_degree - 1 - 6) +  
                abs(b1_degree + 1 - 6) + 
                abs(b2_degree + 1 - 6);   

            if (flipped_deviation < current_deviation) {
                flippedCount++;
                size_t a1_idx = vidx[a1];
                size_t a2_idx = vidx[a2];
                size_t b1_idx = vidx[b1];
                size_t b2_idx = vidx[b2];

                EdgeIter newEdge = flipEdge(e);

                V.row(b1_idx) = blend_factor * V.row(b1_idx) + (1 - blend_factor) * (V.row(a1_idx) + V.row(a2_idx)) / 2.0;;
                V.row(b2_idx) = blend_factor * V.row(b2_idx) + (1 - blend_factor) * (V.row(a1_idx) + V.row(a2_idx)) / 2.0;;
            }
                           
            
        } 
        std::cout << "Flipped " << flippedCount << " edges for regular degree optimization" << std::endl;
    }

    void BubbleHGF::computeCentroids() {
        for (VertexIter v = verticesBegin(); v != verticesEnd(); v++) {
            v->computeCentroid();
            v->newPosition = v->centroid;
            
        }
    }

    void BubbleHGF::tangentialSmoothing(double weight, double dt = 1.0 / 30.0) {

        computeCentroids();
        std::unordered_map<Vertex*, size_t> vidx;
        size_t idx = 0;
        for (auto v = verticesBegin(); v != verticesEnd(); ++v, ++idx) {
            vidx[&(*v)] = idx;
        }

        for (VertexIter v = verticesBegin(); v != verticesEnd(); v++) {
            /*if (v->isBoundary()) {
                continue; 
            }*/

            double adaptiveWeight = weight * (6.0 / v->degree());
            //adaptiveWeight = clamp(adaptiveWeight, 0.01, 0.5);
            Vector3D oldPos = v->position;

            Vector3D update = v->newPosition - oldPos;
            v->computeNormal();
            Vector3D normal = v->normal; 
            double normalComponent = dot(update, normal);
            Vector3D tangentialUpdate = update - normal * normalComponent;

            v->position += tangentialUpdate * adaptiveWeight;

            Vector3D positionChange = v->position - oldPos;

            size_t vertex_idx = vidx[&(*v)];
           

            V.row(vertex_idx) = V.row(vertex_idx) * 0.7 +  Eigen::RowVector3d(positionChange.x, positionChange.y, positionChange.z)/dt;
			//V.row(vertex_idx) *= 0.95; // dampen the velocity a bit
        }
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
        const size_t current_vertex_count = nVertices();
        std::unordered_map<Vertex*, size_t> vidx;
        vidx.reserve(current_vertex_count * 2);

        size_t idx = 0;
        for (auto v = verticesBegin(); v != verticesEnd(); ++v, ++idx) {
            vidx[&(*v)] = idx;
        }
       
        //store edges to split 
        vector<EdgeIter> edgesToSplit;
        for (EdgeIter e = edgesBegin(); e != edgesEnd(); ++e) {
            if (e->length() > threshold) {
                edgesToSplit.push_back(e);
            }
        }

        std::cout << "Found " << edgesToSplit.size() << " edges to split" << std::endl;
        const size_t initial_vertex_count = nVertices();
        V.conservativeResize(initial_vertex_count + edgesToSplit.size(), 3);
        Minv.conservativeResize(initial_vertex_count + edgesToSplit.size());
        
        int i = 0;
        for (EdgeIter e : edgesToSplit) {
            Vertex* v0 = &(*e->halfedge()->vertex());
            Vertex* v1 = &(*e->halfedge()->twin()->vertex());
         
            size_t i0 = vidx[v0];
            size_t i1 = vidx[v1];
         
            double m0 = 1.0 / Minv(i0);
            double m1 = 1.0 / Minv(i1);

            VertexIter newVert = splitEdge(e);
            const size_t new_idx = initial_vertex_count + i;

            newVert->position = 0.5 * (v0->position + v1->position);
            newVert->computeNormal();

            // interpolate velocity
            V.row(new_idx) = (m0 * V.row(i0) + m1 * V.row(i1)) / (m0 + m1);
            Minv(new_idx) = 1.0 / (m0 + m1);

            vidx[&(*newVert)] = new_idx;
            i++; 
        }
        std::cout << "After splitting: v.size() = " << vertices.size()
            << ", V.rows() = " << V.rows()
            << ", Minv.size() = " << Minv.size() << std::endl;

    }



    struct EdgeLen {
        EdgeIter e;
        double len;

        bool operator<(const EdgeLen& other) const {
            if (&(*e) == &(*other.e))
                return false;
            if (len != other.len)
                return len < other.len;
            return &(*e) < &(*other.e);
        }
    };
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

//#define __DEBUG_OUTPUT__

    void BubbleHGF::collapseShortEdges(double threshold) {
        Minv.resize(nVertices());
        Minv.setConstant(1.0);

        std::unordered_map<VertexIter, Vector3D, VertexIterHash, VertexIterEq> vertexVelocity;

#ifdef _DEBUG
        double v0mass;
        double v1mass;
        Vector3D velocity;
#endif
        int idx = 0;
        for (auto v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            vertexVelocity[v] = Vector3D(V(idx, 0), V(idx, 1), V(idx, 2));
        }


        std::set<EdgeLen> edgeSet;
        for (EdgeIter e = edgesBegin(); e != edgesEnd(); ++e) {
            double length = e->length();
            if (length >= threshold) continue;
            edgeSet.insert({ e, length });
        }

        size_t collapsesThisFrame = 0;
        size_t maxCollapsesPerFrame = 300;

        while (!edgeSet.empty() && collapsesThisFrame < maxCollapsesPerFrame) {
            EdgeLen shortest = *edgeSet.begin();
            edgeSet.erase(edgeSet.begin());

            EdgeIter e = shortest.e;

            HalfedgeIter h = e->halfedge();
            HalfedgeIter hTwin = h->twin();

            VertexIter v0 = h->vertex();
            VertexIter v1 = hTwin->vertex();
#ifdef __DEBUG_OUTPUT__
            cout << "before for loop:" << "\n";
#endif

            std::vector<EdgeIter> affectedEdges;
            std::vector<EdgeIter> tempEdgesv0;
            std::vector<EdgeIter> tempEdgesv1;

            bool v0insert = collectIncidentEdges(v0, e, tempEdgesv0);
            bool v1insert = collectIncidentEdges(v1, e, tempEdgesv1);
#ifdef __DEBUG_OUTPUT__
            std::cout << std::boolalpha << "v0insert" << v0insert << "v1insert" << v1insert << "\n";
#endif
            if (v0insert && v1insert) {
                affectedEdges.insert(affectedEdges.end(), tempEdgesv0.begin(), tempEdgesv0.end());
                affectedEdges.insert(affectedEdges.end(), tempEdgesv1.begin(), tempEdgesv1.end());

                for (auto &ae : affectedEdges) {
#ifdef __DEBUG_OUTPUT__
                    cout << "erase edges:" << "\n";
#endif
                    edgeSet.erase({ ae, ae->length() });
                }
            }

            Vector3D newPos = 0.5 * (v0->position + v1->position);
            Vector3D v0Vel = vertexVelocity[v0];
            Vector3D v1Vel = vertexVelocity[v1];

            double v0mass = vertexWeight(v0);
            double v1mass = vertexWeight(v1);


            //vertexVelocity.erase(v0);
            //vertexVelocity.erase(v1);
            // Collapse edge
#ifdef __DEBUG_OUTPUT__
            cout << "Collapsing edge:" << "\n";
#endif
            VertexIter newVert = collapseEdge(e);

            // Update position & normal
            newVert->position = newPos;
            // Interpolate velocity
            Vector3D newVel = (v0mass * v0Vel + v1mass * v1Vel) / (v0mass + v1mass);
            vertexVelocity[newVert] = newVel;
#ifdef _DEBUG
            static volatile double debugMassVelHolder[8] = {
            v0mass, v1mass,
            v0Vel.x, v0Vel.y, v0Vel.z,
            v1Vel.x, v1Vel.y, v1Vel.z
            };
#endif

#ifdef __DEBUG_OUTPUT__
            cout << "new vertex position: " << newVert->position << "\n";
#endif
            newVert->computeNormal();

            HalfedgeIter start = newVert->halfedge();
            HalfedgeIter hv = start;

            do {
                EdgeIter ae = hv->edge();
                double len = ae->length();
#ifdef __DEBUG_OUTPUT__
                std::cout << "in do while inserting length:" << len << "\n";
#endif
                if (len < threshold) {
                    edgeSet.insert({ ae, len });
                }
                hv = hv->twin()->next();
            } while (hv != start);
#ifdef __DEBUG_OUTPUT__
            std::cerr << "Looped through vertices." << std::endl;
#endif

            collapsesThisFrame++;


        }

        std::cout << "Collapses this frame: " << collapsesThisFrame << "\n";
        std::cout << "New vertex count: " << nVertices() << "\n";

        V = Eigen::MatrixXd::Zero(nVertices(), 3);
        idx = 0;
        for (auto &v = vertices.begin(); v != vertices.end(); ++v, ++idx) {
            Vector3D vel = vertexVelocity[v];
            V(idx, 0) = vel.x;
            V(idx, 1) = vel.y;
            V(idx, 2) = vel.z;
            // Update Minv
        }
    }

    bool BubbleHGF::collectIncidentEdges(VertexIter v, EdgeIter collapsingEdge,
        std::vector<EdgeIter>& out) {
        std::vector<EdgeIter> temp;
        bool success = true; // Track overall success

        if (!v->isValid()) return false;
        HalfedgeIter start = v->halfedge();
        //if (start == halfedgesEnd() || !start->isValid()) return false;

        HalfedgeIter hv = start;
        size_t maxSteps = 500;
        std::unordered_set<void*> seen;

        do {
            // Early exit if any step fails
            //if (hv == halfedgesEnd() || !hv->isValid()) {
            //    success = false;
            //    break;
            //}

            //void* key = reinterpret_cast<void*>(&(*hv));
            //if (seen.count(key)) {
            //    std::cerr << "Cycle detected in vertex neighborhood\n";
            //    success = false;
            //    break;
            //}
            //seen.insert(key);

            EdgeIter ae = hv->edge();
            //if (ae != edgesEnd() && ae->isValid() && &(*ae) != &(*collapsingEdge)) {
                temp.push_back(ae);
            //}

            //if (hv->twin() == halfedgesEnd() || !hv->twin()->isValid()) {
            //    success = false;
            //    break;
            //}
            hv = hv->twin()->next();
        } while (hv != start && --maxSteps > 0);

        // Final checks
        if (maxSteps == 0) {
            std::cerr << "collectIncidentEdges: hit maxSteps for vertex\n";
            success = false;
        }

        // Only commit if everything succeeded
        if (success) {
            out.insert(out.end(), temp.begin(), temp.end());
        }
        return success;
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