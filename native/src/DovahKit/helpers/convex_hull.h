#pragma once
#include <algorithm>
#include <array>
#include <cmath>
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
   template<typename N> class gauss_map {
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
      public:
         std::vector<arc> arcs;
   };

   template<typename N> class convex_hull {
      public:
         using number_type  = N;
         using vector_type  = vector3<number_type>;
         using face_index   = size_t;
         using edge_index   = size_t;
         using vertex_index = size_t;
         //
         static constexpr size_t no_edge = std::numeric_limits<size_t>::max();
         static constexpr size_t no_face = std::numeric_limits<size_t>::max();
         //
         struct half_edge_type {
            //
            // What's a half-edge? Well, consider two faces that share an edge. That's one edge, yes? 
            // This is half of that.
            //
            // ...Okay, that explanation sucks. How about this: if an edge is shared between two faces, 
            // then there will be two separate half-edges representing that one edge, each of which will 
            // be associated with one of the faces.
            //
            vertex_index start;
            vertex_index end;
            face_index   face;
            edge_index   twin = no_edge;
            //
            vector_type get_vector(const convex_hull& owner) const noexcept {
               auto& s = owner.vertices[this->start];
               auto& e = owner.vertices[this->end];
               return e - s;
            }
            vector_type get_main_face_normal(const convex_hull& owner) const noexcept {
               return owner.faces[this->face].get_normal(owner);
            }
            vector_type get_twin_face_normal(const convex_hull& owner) const noexcept {
               return owner.half_edges[this->twin].get_main_face_normal(owner);
            }
         };
         class face_type {
            protected:
               inline vector_type _get_vertex(const convex_hull& owner, size_t i) const noexcept {
                  return owner.vertices[this->vertices[i % 3]];
               }
            public:
               face_type() {}
               face_type(vertex_index u, vertex_index v, vertex_index w, edge_index a, edge_index b, edge_index c) : vertices({ u, v, w }), half_edges({ a, b, c }) {}
               //
               std::array<vertex_index, 3> vertices;
               std::array<edge_index, 3>   half_edges;
               //
               vector_type get_edge(const convex_hull& owner, int index) const noexcept {
                  auto  ei = this->half_edges[index];
                  auto& he = owner.half_edges[ei];
                  auto& s  = owner.vertices[he.start];
                  auto& e  = owner.vertices[he.end];
                  return e - s;
               }
               vector_type get_normal(const convex_hull& owner) const noexcept {
                  auto& A = this->_get_vertex(owner, 0);
                  auto& B = this->_get_vertex(owner, 1);
                  auto& C = this->_get_vertex(owner, 2);
                  //
                  auto n = (A - B).cross(C - A).normalize();
                  if (n.dot(A - owner.get_centroid()) < 0)
                     n = -n;
                  return n;
               }
               //
               bool shares_edge_with(const convex_hull& owner, const face_type& other) const noexcept {
                  for (auto i : this->half_edges)
                     for (auto j : other.half_edges)
                        if (owner.half_edges[i].twin == j)
                           return true;
                  return false;
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
         std::vector<vector_type>    vertices;
         std::vector<half_edge_type> half_edges;
         std::vector<face_type>      faces; // each face is an array of vertex indices
         //
         struct {
            vector_type centroid; // a.k.a. centerpoint
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
                     if (good) {
                        auto fc = this->faces.size();
                        auto ec = this->half_edges.size();
                        this->half_edges.emplace_back(i, j, fc, no_edge);
                        this->half_edges.emplace_back(j, k, fc, no_edge);
                        this->half_edges.emplace_back(k, i, fc, no_edge);
                        this->faces.emplace_back(i, j, k, ec, ec + 1, ec + 2);
                     }
                  }
               }
            }
            this->connect_twin_edges();
            this->update_cached_data();
         }

         void connect_twin_edges() noexcept {
            auto& list = this->half_edges;
            auto  size = list.size();
            for (edge_index i = 0; i < size; ++i) {
               for (edge_index j = i + 1; j < size; ++j) {
                  auto& a = list[i];
                  auto& b = list[j];
                  if (a.start != b.start || a.end != b.end)
                     if (a.start != b.end || a.end != b.start)
                        continue;
                  a.twin = j;
                  b.twin = i;
               }
            }
         }
         void update_cached_data() noexcept {
            this->cached.centroid = this->_compute_centroid();
         }

         vector_type get_centroid() const noexcept {
            return this->cached.centroid;
         }

         vector_type get_edge(edge_index i) const noexcept {
            return this->half_edges[i].get_vector(*this);
         }
         vector_type get_edge(vertex_index a, vertex_index b) const noexcept {
            return this->vertices[b] - this->vertices[a];
         }

         /*//
         bool simple_separating_axis_test(const convex_hull& other) const noexcept { // "simple" does not mean "efficient"
            auto ec_a = this->half_edges.size();
            auto ec_b = other.half_edges.size();
            for (size_t i = 0; i < ec_a; ++i) {
               for (size_t j = 0; j < ec_b; ++j) {
                  auto ea   = this->get_edge(i);
                  auto eb   = other.get_edge(j);
                  auto axis = ea.cross(eb);
                  //
                  auto interval_a = this->project(axis); // TODO
                  auto interval_b = other.project(axis);
                  number_type separation = compare(interval_a, interval_b);
               }
            }
         }
         //*/

      protected:
         struct edge_query {
            edge_index  a = no_edge;
            edge_index  b = no_edge;
            number_type distance = std::numeric_limits<number_type>::quiet_NaN();
         };
         static bool _arcs_intersect(const vector_type& A, const vector_type& B, const vector_type& C, const vector_type& D) noexcept { // test whether arcs AB and CD intersect
            vector_type BxA = B.cross(A);
            vector_type DxC = D.cross(C);
            number_type CBA = C.dot(BxA);
            number_type DBA = D.dot(BxA);
            number_type ADC = A.dot(DxC);
            number_type BDC = B.dot(DxC);
            return (CBA * DBA < 0) && (ADC * BDC < 0) && (CBA * BDC > 0);
         }
         static bool _build_minkowski_face(const convex_hull& ha, const half_edge_type& ea, const convex_hull& hb, const half_edge_type& eb) noexcept {
            vector_type a = ea.get_main_face_normal(ha);
            vector_type b = ea.get_twin_face_normal(ha);
            vector_type c = eb.get_main_face_normal(hb);
            vector_type d = eb.get_twin_face_normal(hb);
            return _arcs_intersect(a, b, -c, -d); // negate one of the pairs of normals to account for the Minkowski difference
         }
         edge_query _edge_sat(const convex_hull& other) const noexcept {
            //
            // TODO: This doesn't take transforms (hull positions/rotations) into account at any step in the process!
            //
            edge_query result;
            //
            edge_index  eca      = this->half_edges.size();
            edge_index  ecb      = other.half_edges.size();
            vector_type center_a = this->get_centroid();
            for (edge_index ia = 0; ia < eca; ++ia) {
               auto& ea = this->half_edges[ia];
               if (ea.twin < ia)
                  continue;
               for (edge_index ib = 0; ib < ecb; ++ib) {
                  auto& eb = other.half_edges[ib];
                  if (eb.twin < ib)
                     continue;
                  if (!_build_minkowski_face(*this, ea, other, eb))
                     continue;
                  number_type separation;
                  {
                     vector_type pa     = this->vertices[ea.start]; // edge origins
                     vector_type pb     = other.vertices[eb.start];
                     vector_type da     = this->vertices[ea.end] - pa; // edge directions
                     vector_type db     = other.vertices[eb.end] - pb;
                     vector_type normal = da.cross(db);
                     if (normal.length_sq() < 0.005F * std::sqrt(da.length_sq() * db.length_sq()))
                        //
                        // The edges are parallel or nearly parallel.
                        //
                        continue;
                     normal.normalize();
                     if (normal.dot(pa - center_a))
                        normal = -normal;
                     separation = normal.dot(pb - pa);
                  }
                  if (separation < result.distance) // when the distance is NaN, this should always be false
                     continue;
                  result.a = ia;
                  result.b = ib;
                  result.distance = separation;
               }
            }
            return result;
         }

      public:
         bool overlaps(const convex_hull& other) const noexcept {
            auto face_query = this->_query_face_directions(*this, other);
            if (face_query > 0)
               return false;
            face_query = this->_query_face_directions(other, *this);
            if (face_query > 0)
               return false;
            auto edge_query = this->_edge_sat(other);
            if (edge_query.distance > 0)
               return false;
            return true;
         }

         // not actually needed for S.A.T.
         [[nodiscard]] gauss_map<number_type> to_gauss_map() const noexcept {
            constexpr number_type nan  = std::numeric_limits<number_type>::quiet_NaN();
            constexpr vector_type none = { nan, nan, nan };
            //
            gauss_map<number_type> gauss;
            //
            size_t fc = this->faces.size();
            for (size_t fa = 0; fa < fc; ++fa) {
               for (size_t fb = fa + 1; fb < fc; ++fb) {
                  auto& a = this->faces[fa];
                  auto& b = this->faces[fb];
                  if (!a.shares_edge_with(*this, b))
                     continue;
                  auto& arc = gauss.arcs.emplace_back();
                  arc.start = a.get_normal(*this);
                  arc.end   = b.get_normal(*this);
               }
            }
            return gauss;
         }
   };
}
