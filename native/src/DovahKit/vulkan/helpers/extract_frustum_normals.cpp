#include "extract_frustum_normals.h"

namespace {
   template<size_t axis, bool sub> void _make_normal(const glm::mat4& view_proj, glm::vec4& out) {
      //
      // This is as per the math presented in the 2001 paper "Fast Extraction 
      // of Viewing Frustum Planes from the World-View-Projection Matrix," by 
      // Gil Gribb and Klaus Hartmann.
      // 
      //   <http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf>
      // 
      // Let's consider the mathematics for  applying a transformation matrix 
      // M to a position V:
      // 
      //    [a b c d      [x      [xa + yb + zc + wd
      //     e f g h   *   y   =   xe + yf + zg + wh
      //     i j k l       z       xi + yj + zk + wl
      //     m n o p]      w]      xm + yn + zo + wp]
      // 
      // This can alternately be represented as:
      // 
      //    [dot(position, matrix.row(0))
      //     dot(position, matrix.row(1))
      //     dot(position, matrix.row(2))
      //     dot(position, matrix.row(3))]
      // 
      // This transformation brings the  position into NDC (normalized device 
      // coordinates), which for Vulkan are  [-1, 1] for XY and [0, 1] for Z. 
      // Notably, this transformation doesn't apply perspective directly: the 
      // GPU will do that for us, dividing  the transformed vector by its own 
      // W-component to produce  an on-screen coordinate.  This means that in 
      // effect, after the view/projection matrix is applied to the position, 
      // the bounds for each axis are [-(M*V).w, +(M*V).w].
      // 
      // Let's give ourselves some variable names to make this clearer:
      // 
      //    glm::mat4 view_proj;
      //    glm::vec4 position; // W-component is 1
      // 
      //    glm::vec4 ndc_pos = view_proj * position;
      // 
      //    if (ndc_pos.x < -ndc_pos.w)
      //       return false; // off-screen (left)
      //    if (ndc_pos.x >  ndc_pos.w)
      //       return false; // off-screen (right)
      //    if (ndc_pos.y < -ndc_pos.w)
      //       return false; // off-screen (bottom)
      //    if (ndc_pos.y >  ndc_pos.w)
      //       return false; // off-screen (top)
      //    return true; // on-screen
      // 
      // This forms the first step of the math presented in the paper. We can 
      // go further and figure out how to  mathematically convert the frustum 
      // into a form that makes these sorts of checks as easy as possible.
      // 
      // Consider:
      // 
      //    ndc_pos.x > -ndc_pos.w // NOT off-screen to the left
      // 
      //    -ndc_pos.w < ndc_pos.x // let's reorder them for convenience
      // 
      // Recalling that a matrix-by-vector  multiplication can be represented 
      // as a series of dot products, we can rewrite this as:
      // 
      //    -dot(position, matrix.row(3)) < dot(position, matrix.row(0))
      // 
      // And therefore:
      // 
      //    0 - dot(position, matrix.row(3)) < dot(position, matrix.row(0))
      // 
      //    0 < dot(position, matrix.row(0)) + dot(position, matrix.row(3))
      // 
      //    0 < dot(position, matrix.row(0) + matrix.row(3))
      // 
      // The righthand side  of this inequality  can be rewritten in terms of 
      // the individual components (XYZW) of the position:
      // 
      //    0 < (position.x * (matrix[0][0] + matrix[0][4])) + 
      //        (position.y * (matrix[1][0] + matrix[1][4])) + 
      //        (position.z * (matrix[2][0] + matrix[2][4])) + 
      //        (position.w * (matrix[3][0] + matrix[3][4]))
      // 
      // The W-component of a position, of course, is always 1:
      // 
      //    0 < (position.x * (matrix[0][0] + matrix[0][4])) + 
      //        (position.y * (matrix[1][0] + matrix[1][4])) + 
      //        (position.z * (matrix[2][0] + matrix[2][4])) + 
      //        (matrix[3][0] + matrix[3][4])
      // 
      // This is useful to us because it follows the point-normal form of the 
      // equation of a plane. That is, it's an equation of the form
      // 
      //    0 = ax + by + cz + d
      // 
      // and those equations can define a plane.
      // 
      // Now, mathematicians have a dire phobia of structs, and tend to avoid 
      // any struct-like notation,  defining structs as the sum or product of 
      // their own members. It's unintuitive, but here, at least, there is an 
      // easily traceable reason for it. If you wanted to write a struct that 
      // defines a plane of infinite length, you might use an arbitrary point 
      // on the plane,  and the plane's surface normal.  Let's call those "P" 
      // and "N" respectively.  For any point on the plane "R", the following 
      // will be true:
      // 
      //    dot(N, R - P) = 0
      // 
      // That is: the vector from R to P will be perpendicular to N.  If you 
      // take the vector  from any point in the plane  to any other point in 
      // the plane, it will be perpendicular  to the normal and so produce a 
      // zero dot product.
      // 
      // Now, if we expand that equation, then we get:
      // 
      //    0 = (N.x * (R.x - P.x)) + 
      //        (N.y * (R.y - P.y)) + 
      //        (N.z * (R.z - P.z))
      // 
      // This is equivalent to:
      // 
      //    0 = (N.x * R.x) - (N.x * P.x) + 
      //        (N.y * R.y) - (N.y * P.y) + 
      //        (N.z * R.z) - (N.z * P.z)
      // 
      // Which is equivalent to:
      // 
      //    0 = ((N.x * R.x) + (N.y * R.y) + (N.z * R.z)) - 
      //        ((N.x * P.x) + (N.y * P.y) + (N.z * P.z))
      // 
      // That is a point-normal equation for a plane, given
      // 
      //    a = N.x
      //    b = N.y
      //    c = N.z
      //    d = -((N.x * P.x) + (N.y * P.y) + (N.z * P.z))
      // 
      // Which brings us  back to our view/projection  matrix multiplied by a 
      // position:
      // 
      //    0 < (position.x * (matrix[0][0] + matrix[0][4])) + 
      //        (position.y * (matrix[1][0] + matrix[1][4])) + 
      //        (position.z * (matrix[2][0] + matrix[2][4])) + 
      //        (matrix[3][0] + matrix[3][4])
      // 
      // Treating this as the point-normal form of the frustum's left plane:
      // 
      //    a = matrix[0][0] + matrix[0][4]
      //    b = matrix[1][0] + matrix[1][4]
      //    c = matrix[2][0] + matrix[2][4]
      //    d = matrix[3][0] + matrix[3][4]
      // 
      // i.e.
      // 
      //    a = matrix[0].x + matrix[0].w
      //    b = matrix[1].x + matrix[1].w
      //    c = matrix[2].x + matrix[2].w
      //    d = matrix[3].x + matrix[3].w
      // 
      // And of course,  given any position that lies on the plane, this will 
      // be true:
      // 
      //    0 = ax + by + cz + d
      // 
      // And the A, B, and C variables are just the components of the plane's 
      // surface normal.  We'll treat D as a W-component on the normal,  too.
      // 
      // If we want the frustum's right-side  plane, we just subtract the XYZ 
      // components from the W component, instead of adding them. For the top 
      // and bottom planes, we pull the  Y-axis instead of the X-axis for the 
      // A, B, C, and D formulae above. But how do we use these results?
      // 
      // Recall that taking the dot product of two direction vectors will let 
      // us know whether they  point in the same general  direction, based on 
      // the sign;  a zero result tells us that they're perpendicular.  For a 
      // plane surface normal and a position, the dot product tells us if the 
      // position is on the plane (zero), and if not, which side of the plane 
      // it's on (sign).  The magnitude also  tells us the  distance from the 
      // point to the plane.
      // 
      // Our four plane  surface normals are inward-facing,  so if a point is 
      // inside of the frustum, then taking  its dot product with each normal 
      // will produce four positive results. If a point is on any side of the 
      // frustum,  then we'll get more than zero but less than  four positive 
      // results; if a point is behind the frustum (i.e. the camera would see 
      // it if we turned 180 degrees), then  we'll get no positive results at 
      // all.
      // 
      // That does beg  one question,  though: if we're  relying on a surface 
      // normal, then how do  we account for the  plane's position? Well, our 
      // normal is a 4D vector, and it's not normalized. Let's look at what's 
      // called the "Hessian normal form" of a plane:
      // 
      //    dot(N, R) = -P
      // 
      //    given:
      //       vector N, a normalized surface normal
      //       vector R, any position on the plane
      //       scalar P, the distance from the origin to the plane
      // 
      // If R isn't on the plane,  then that formula  will be false,  and the 
      // amount by  which it's off will be  the distance from R to the plane: 
      // 
      //    0 != dot(N, R) + P
      // 
      //    distance = dot(N, R) + P
      // 
      // This is closely  related to the equation for a  plane given a normal 
      // and a position on the plane:
      // 
      //    dot(N, R - P) = 0
      // 
      //    given:
      //       vector N, a surface normal
      //       vector R, a position on the plane (position to test)
      //       vector P, a position on the plane (known reference position)
      // 
      // Hey, it's that one plane equation from earlier.  So in our case, our 
      // planes' surface normals aren't normalized,  but I guess the normals' 
      // W-component  is "out of step"  in just the right way  to balance out 
      // the others.
      //
      for (int i = 0; i < 4; ++i) {
         float v;
         if constexpr (axis == 0)
            v = view_proj[i].x;
         else if constexpr (axis == 1)
            v = view_proj[i].y;
         else if constexpr (axis == 2)
            v = view_proj[i].z;
         //
         if constexpr (sub)
            out[i] = view_proj[i].w - v;
         else
            out[i] = view_proj[i].w + v;
      }
   }
}

namespace vulkanDK {
   extern std::array<glm::vec4, 4> extract_frustum_normals(const glm::mat4& view_proj) {
      std::array<glm::vec4, 4> out;
      _make_normal<0, false>(view_proj, out[0]); // left   plane
      _make_normal<0, true>(view_proj, out[1]);  // right  plane
      _make_normal<1, false>(view_proj, out[2]); // top    plane
      _make_normal<1, true>(view_proj, out[3]);  // bottom plane
      for (auto& item : out)
         item /= glm::length(glm::vec3(item));
      return out;
   }
}