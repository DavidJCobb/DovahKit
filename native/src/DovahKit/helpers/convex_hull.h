#pragma once
#include <algorithm>
#include <array>
#include <limits>
#include <vector>
#include "rotation.h"
#include "vector3.h"

//
// The contents of this document were modeled after "The Separating Axis Test between 
// Convex Polyhedra," an educational presentation by Dirk Gregorius of Valve Software.
//
// So how does it work?
//
// To test whether two spheres are intersecting, you need to check whether the distance 
// between their centerpoints is less than or equal to the sum of their radii. This is 
// logically equivalent to increasing one sphere's radius by the radius of the other, 
// and then checking whether the former contains the latter's centerpoint.
//
// We'll come back to that.
//
// So running a separating axis theorem test between two convex hulls is relatively 
// expensive. Running a separating axis theorem test between one convex hull and one 
// point in space is quite a bit cheaper. It would be ideal, then, if it were possible 
// to shrink Hull B down to just its centerpoint, and then enlarge and reshape Hull A 
// to compensate for the loss of Hull B's size and shape, before then testing whether 
// the altered Hull A contains the formerly-Hull now-Point B. Yes? Yes. And this is 
// the same operation as the one above, where we change a sphere-to-sphere check into 
// a logically equivalent sphere-to-point check. But how do we do that with arbitrary 
// convex hulls?
//
// We'll come back to that.
//
// Do you remember that one shitpost where someone "simplified the topology of a cow" 
// to make a spherical cow? Well, you can do that to any convex hull: any convex hull 
// can be simplified to a sphere, because they never curve in on themselves and they 
// have no holes. So imagine we simplify Hull A down to a sphere. Now imagine we take 
// all of the surface normals of Hull A's faces. Well, each surface normal is a unit 
// vector, and a unit vector is the vector from the centerpoint to the surface of a 
// unit sphere. So we can take all of those surface normals from the original Hull A 
// and put them *into* the topologically-simplified Sphere A, to mark points on that 
// sphere.
//
// Now imagine that for any two faces that share an edge, we draw an arc connecting 
// their surface-normal points on Sphere A. What we'll end up with is a sphere that's 
// covered in arcs. This is a "gauss map."
//
// So let's take the gauss maps of Hull A and Hull B. We'll say that Hull A's arcs 
// are colored red and Hull B's arcs are colored blue. We can overlay these gauss 
// maps on top of each other. Let's look at the points where red and blue arcs meet 
// and intersect on the combined gauss map. These points are the surface normals of 
// the faces on the Reshaped Hull A that we're trying to create. The combined gauss 
// map is the blueprint for making this new shape.
//
// So how do we make that reshaped hull? We don't!
// 
// The main reason why this method is faster than the "naive" way is because we don't 
// have to blindly test every single edge on both hulls. If we just test the separating 
// axes for that resized and reshaped hull, then we save a ton of time and effort, and 
// crucially, we don't actually need that hull.The gauss maps are enough to deduce its 
// normals and therefore the separating axes needed.Maybe we could go to the trouble of 
// trying to figure out how to actually create the resized and reshaped hull in order 
// to allow for a hull - to - point check, but that's hard and difficult and may not 
// even be worth the trouble. Essentially, the concept of the resized and reshaped hull 
// just serves to prove that the gauss maps are sufficient for getting the bare minimum 
// number of potential separating axes needed for an efficient hull-to-hull check.
//

namespace cobb {
   template<typename T> class gauss_map {
      public:
         using number_type = N;
         using vector_type = vector3<number_type>;
         //
         struct arc {
            vector_type start;
            vector_type end;
            //
            arc operator-() const noexcept {
               arc result = *this;
               result.start *= number_type(-1);
               result.end   *= number_type(-1);
               return result;
            }
         };
         //
      protected:
         static bool _arcs_intersect(const arc& u, const arc& v) noexcept {
            auto& A = u.start;
            auto& B = u.end;
            auto  C = v.start;
            auto  D = v.end;
            //
            vector_type BxA = B.cross(A);
            vector_type DxC = D.cross(C);
            number_type CBA = C.dot(BxA);
            number_type DBA = D.dot(BxA);
            number_type ADC = A.dot(DxC);
            number_type BDC = B.dot(DxC);
            return (CBA * DBA < 0) && (ADC * BDC < 0) && (CBA * BDC > 0);
         }
         //
      public:
         std::vector<arc> arcs;

         std::vector<vector_type> find_intersections(const gauss_map& other) const noexcept {
            std::vector<vector_type> intersections;
            for (auto& a : this->arcs) {
               for (auto& b : other.arcs) {
                  if (!_arcs_intersect(a, -b))
                     continue;

               }
            }
         }
   };

   template<typename N> class convex_hull {
      public:
         using number_type  = N;
         using vector_type  = vector3<number_type>;
         using face_index   = size_t;
         using vertex_index = size_t;
         using face_type    = std::array<vertex_index, 3>;
         //
         static constexpr size_t no_face = std::numeric_limits<size_t>::max();
         //
         struct edge_type {
            vertex_index indices[2];
            //
            vertex_index& operator[](int i) noexcept { return this->indices[i]; }
            const vertex_index& operator[](int i) const noexcept { return this->indices[i]; }
            bool operator==(const edge_type& o) const noexcept {
               if (this->indices[0] == o[0])
                  if (this->indices[1] == o[1])
                     return true;
               if (this->indices[0] == o[0])
                  if (this->indices[1] == o[1])
                     return true;
               return false;
            }
         };
         struct face_data { // TODO: USE ME
            std::array<vertex_index, 3> vertices;
            std::array<face_index, 3>   adjacent = { no_face, no_face, no_face };
            //
            vector_type get_edge(const convex_hull& owner, int index) const noexcept {
               auto& s = owner.vertices[this->vertices[index % 3]];
               auto& e = owner.vertices[this->vertices[(index + 1) % 3]];
               return e - s;
            }
            vector_type get_normal(const convex_hull& owner) const noexcept {
               auto& A = owner.vertices[this->vertices[0]];
               auto& B = owner.vertices[this->vertices[1]];
               auto& C = owner.vertices[this->vertices[2]];
               //
               auto n = (A - B).cross(C - A).normalize();
               if (n.dot(A - owner.get_centroid()) < 0)
                  n = -n;
               return n;
            }
         };
         //
      protected:
         vector_type _compute_centroid() const noexcept {
            vector_type c;
            for (auto& v : this->vertices)
               c += v;
            c /= this->vertices.size();
            return c;
         }
         //
      public:
         vector_type     position;
         rotation_matrix rotation;
         std::vector<vector_type> vertices;
         std::vector<face_type>   faces; // each face is an array of vertex indices
         //
         struct {
            vector_type centroid; // a.k.a. centerpoint
            std::vector<edge_type> edges;
         } cached;
         
         convex_hull() {}
         convex_hull(const std::vector<vector_type>& v, float scale = 1.0F, float epsilon = 0.0001F) : vertices(v) { // designed for bhkConvexVerticesShape
            auto& list = this->vertices;
            auto  size = list.size();
            if (size < 3)
               return;
            for (size_t i = 0; i < size - 2; ++i) {
               for (size_t j = i + 1; j < size - 1; ++j) {
                  for (size_t k = i + 2; k < size; ++k) {
                     auto& A = list[i];
                     auto& B = list[j];
                     auto& C = list[k];
                     A *= scale;
                     B *= scale; // while we're here, we may as well apply the scale as well
                     C *= scale;
                     //
                     // Okay, we've selected three vertices in the mesh. Let's see if these three vertices are 
                     // one of the mesh's faces. Imagine taking a knife and cutting the mesh. If you cut in 
                     // the middle of the mesh, then some of the mesh will be on one side of your knife and 
                     // some of the mesh will be on the other side. However, if you scrape a surface on the 
                     // mesh, then all of the mesh will be on just one side of your knife. To put it another 
                     // way: if we take three vertices and cut the mesh along the face that they form, then 
                     // we can check whether there is mesh content on only one side of that cut, and if so, 
                     // then those three vertices must form an actual face on the mesh.
                     //
                     // What we want to do, then, is iterate over every vertex in the mesh except for the 
                     // current A, B, and C vertices, and test which side of the A-B-C triangle the current 
                     // vertex is on.
                     //
                     bool good = true;
                     int  prev = 0;
                     //
                     auto normal = (A - B).cross(C - A);
                     for (size_t p = 0; p < size; ++p) {
                        auto& V = list[p];
                        if (p >= i && p <= k) // don't operate on any of A, B, or C
                           continue;
                        number_type D = (V - A).dot(normal);
                        if (D >= -epsilon && D <= epsilon) // these points are colinear (i.e. they form a straight line)
                           continue;
                        //
                        // With a dot product in hand, we can test what side of the A-B-C triangle our V 
                        // vertex is on. The sign of the dot product indicates whether the A-B-C surface 
                        // normal points toward V or away from V.
                        //
                        // The variable (prev) holds the previous sign or zero, so if we add (sign) to 
                        // it and get zero, then (sign == -prev) which would indicate that list[p] is on 
                        // a different side of the normal than list[p - 1].
                        //
                        int sign = (D > -epsilon) - (D < epsilon); // determine whether we were making a left turn or a right turn
                        if (sign + prev == 0) { // if it's different from the previous turn we made, then abort
                           good = false;
                           break;
                        }
                        prev = sign;
                     }
                     if (good)
                        this->faces.emplace_back(i, j, k);
                  }
               }
            }
            this->update_cached_data();
         }

         void update_cached_data() noexcept {
            this->cached.centroid = this->_compute_centroid();
            {
               auto& edges = this->cached.edges;
               edges.clear();
               for (auto& f : this->faces) {
                  edges.emplace_back(f[0], f[1]);
                  edges.emplace_back(f[1], f[2]);
                  edges.emplace_back(f[2], f[0]);
               }
               edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
            }
         }

         vector_type get_centroid() const noexcept {
            return this->cached.centroid;
         }

         vector_type get_edge(const edge_type& edge) const noexcept {
            return this->vertices[edge[1]] - this->vertices[edge[0]];
         }
         vector_type get_edge(vertex_index a, vertex_index b) const noexcept {
            return this->vertices[b] - this->vertices[a];
         }
         #pragma region faces
         [[nodiscard]] bool faces_share_edge(size_t face_a, size_t face_b) const noexcept {
            auto& fa = this->faces[face_a];
            auto& fb = this->faces[face_b];
            for (int i = 0; i < 3; ++i) {
               /*
               // list[fa[0]] == A | list[fb[0]] == U
               // list[fa[1]] == B | list[fb[1]] == V
               // list[fa[2]] == C | list[fb[2]] == W
               //   AB or BC or CA | UV or VW or WU
               //
               //  if any of these = any of these
               //
               // -----------------------------------
               //
               // for (auto p, q : [{a,b}, {b,c}, {c,a}]) {
               //    for (auto r, s : [{u,v}, {v,w}, [w,u}]) {
               //       if (r == p && s == q)
               //          return true;
               //       if (r == q && s == p)
               //          return true;
               //    }
               // }
               // return false;
               */
               auto& p = fa[i];
               auto& q = fa[(i + 1) % 3];
               for (int j = 0; j < 3; ++j) {
                  auto& r = fb[j];
                  auto& s = fb[(j + 1) % 3];
                  if (r == p && s == q)
                     return true;
                  if (r == q && s == p)
                     return true;
               }
            }
            return false;
         }
         [[nodiscard]] vector_type face_normal(size_t face_index) const noexcept {
            auto& face = this->faces[face_index];
            auto& A    = this->vertices[face[0]];
            auto& B    = this->vertices[face[1]];
            auto& C    = this->vertices[face[2]];
            //
            auto n = (A - B).cross(C - A).normalize();
            if (n.dot(A - this->cached.centroid) < 0)
               n = -n;
            return n;
         }
         #pragma endregion

         bool simple_separating_axis_test(const convex_hull& other) const noexcept { // "simple" does not mean "efficient"
            auto  ec_a = this->cached.edges.size();
            auto  ec_b = other.cached.edges.size();
            for (size_t i = 0; i < ec_a; ++i) {
               for (size_t j = 0; j < ec_b; ++j) {
                  auto ea   = this->get_edge(this->cached.edges[i]);
                  auto eb   = this->get_edge(other.cached.edges[j]);
                  auto axis = ea.cross(eb);
                  //
                  auto interval_a = this->project(axis); // TODO
                  auto interval_b = other.project(axis);
                  number_type separation = compare(interval_a, interval_b);
               }
            }
         }

         [[nodiscard]] gauss_map<number_type> to_gauss_map() const noexcept {
            constexpr number_type nan  = std::numeric_limits<number_type>::quiet_NaN();
            constexpr vector_type none = { nan, nan, nan };
            //
            gauss_map<number_type> gauss;
            //
            size_t fc = this->faces.size();
            for (size_t fa = 0; fa < fc; ++fa) {
               for (size_t fb = fa + 1; fb < fc; ++fb) {
                  if (!this->faces_share_edge(fa, fb))
                     continue;
                  auto& arc = gauss.arcs.emplace_back();
                  arc.start = this->face_normal(fa);
                  arc.end   = this->face_normal(fb);
               }
            }
            return gauss;
         }

         [[nodiscard]] convex_hull to_minkowski_sum(const convex_hull& other) const noexcept {

         }

      protected:
         bool _is_minkowski_face(const vector_type& A, const vector_type& B, const vector_type& C, const vector_type& D) {
            //
            // Tests for an intersection between arcs AB and CD on a unit sphere.
            //
            // All convex polyhedra can have their topologies simplified to a sphere, you see. The surface 
            // normals from that polyhedra are unit vectors, so they point from the center of the sphere 
            // to its surface. If any two faces share an edge, draw an arc to connect their normals' end-
            // points. Congratulations: you've now created a gauss map. But what the heck is that for?
            //
            // Well, if you want to test for the intersection between two convex polyhedra, you can take 
            // gauss maps of both of them and overlay them. Any points where the arcs overlap are the 
            // unit vectors for potential separating axes.
            //
            vector_type BxA = B.cross(A);
            vector_type DxC = D.cross(C);
            number_type CBA = C.dot(BxA);
            number_type DBA = D.dot(BxA);
            number_type ADC = A.dot(DxC);
            number_type BDC = B.dot(DxC);
            return (CBA * DBA < 0) && (ADC * BDC < 0) && (CBA * BDC > 0);
         };
   };
}
