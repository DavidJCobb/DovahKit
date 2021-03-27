/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "rotation.h"
#include <cmath>
#include <cstdint>

#pragma region An explanation of rotation math as it relates to Euler angles.
/*
   
   There are  multiple ways  to represent a rotation.  The one you're probably familiar 
   with is Euler angles: nice, friendly lil' things; one per axis, so three when you're 
   dealing with 3D.  But what if I  told you that they aren't as friendly as they seem?

   See,  there are  actually  different "conventions"  for Euler angles,  such that the 
   exact same numbers can have wildly different  results between conventions.  The four 
   factors that define a set of Euler conventions are:

      ORDER: What order are you applying the angles in? XYZ? ZYX? ZXY?

      HANDEDNESS: Are you rotation clockwise or counterclockwise around each axis?

      AXIS USE:  Are you doing  bizarre stuff like using ZXZ?  This kind of falls under 
      "order."

      REFERENCE FRAME:  Are  you rotating around  global axes ("extrinsic"),  or around 
      local axes  ("intrinsic")  such that one rotation changes the axes that the other 
      rotations go around?

   This  amounts  to 48 possible Euler conventions.  Half of  those are reference frame 
   variants;  half of each half are handedness variations; this leaves 12 "unique" con-
   ventions.

   So how do  we manage this?  Well, the first thing to  keep in mind is that if you're 
   only rotating around a single axis,  then you've immediately eliminated every factor 
   except for handedness.  So perhaps we can start by converting each axis individually 
   to a more stable representation,  such as a rotation matrix.  To flip the handedness 
   of a rotation matrix,  you just take its transpose.  Dealing with that at this stage 
   means  we've now eliminated  one  of those four factors that's making  everything so 
   inconsistent.

   Okay, fine. But how, then, do we convert an entire set of Euler angles to a rotation 
   matrix? Well, this is where we deal with the order and the axis use, too: we do some 
   matrix  multiplication.  If our Euler order is ZYX,  then we literally just multiply 
   the Z into the Y, and then multiply the result of that into the X.

   NOTE: I'm 90% sure that you have to transpose each axis's rotation matrices individ-
   ually, before you multiply them together.  I'm pretty sure that transposing a "full" 
   rotation  matrix,  one  that has had all of the  axes multiplied in, won't produce a 
   correct result.

   At this point, we've dealt with everything except the reference frame.  How do we do 
   that? Well, as an example, imagine that you're doing the math for extrinsic ZYX, but 
   you do  something a little strange when you're converting  the  individual axes from 
   Euler to  rotation matrices:  you  supply the  X angle to the "Z" matrix,  and the Z 
   angle to the "X" matrix.  So,  you've switched the places of the angles; they're now 
   exactly backwards.  Congratulations: you're now doing math for intrinsic XYZ angles!

*/
#pragma endregion

namespace {
   double _passive_atrig_clamp(double f) {
      constexpr double EPSILON = 0.000001F;
      //
      // The trigonometric acos and asin functions fail if their input is outside of the 
      // range [-1, 1]. Floating-point imprecision (stemming from multiple rotation math 
      // operations) can cause us to fall a hair's breadth outside of that range. This 
      // function will correct a double if and only if it is indeed a hair's breadth or 
      // less outside of that range. If it's much more inaccurate, then we don't bother 
      // catching it on the grounds that the caller is either broken or working with 
      // data that was invalid from the start.
      //
      double g = fabs(f);
      if (g > 1.0F && g - 1.0F < EPSILON) {
         return ((0.0 < f) - (f < 0.0)); // sign
      }
      return f;
   }
}
namespace cobb {
   #pragma region rotation_matrix
   double rotation_matrix::determinant() const noexcept {
      // [a, b, c,
      //  d, e, f,
      //  g, h, i]
      double a = this->data[0][0];
      double b = this->data[0][1];
      double c = this->data[0][2];
      double d = this->data[1][0];
      double e = this->data[1][1];
      double f = this->data[1][2];
      double g = this->data[2][0];
      double h = this->data[2][1];
      double i = this->data[2][2];
      return (a*e*i) + (b*f*g) + (c*d*h) - (c*e*g) - (b*d*i) - (a*f*h);
   }

   /*static*/ rotation_matrix rotation_matrix::construct_from_x(double radians, bool righthanded) {
      //
      // For lefthanded:
      // [ 1,       0,      0,
      //   0,  cos(x), sin(x),
      //   0, -sin(x), cos(x)  ]
      //
      // You flip handedness by performing a matrix transpose, but in this case, that just ends 
      // up flipping the signs on two values.
      //
      rotation_matrix out;
      auto& d = out.data;
      double c = std::cos(radians);
      double s = std::sin(radians);
      d[0][0] = 1.0F;
      d[0][1] = 0.0F;
      d[0][2] = 0.0F;
      d[1][0] = 0.0F;
      d[1][1] = c;
      d[1][2] = righthanded ? -s : s;
      d[2][0] = 0.0F;
      d[2][1] = righthanded ? s : -s;
      d[2][2] = c;
      return out;
   }
   /*static*/ rotation_matrix rotation_matrix::construct_from_y(double radians, bool righthanded) {
      //
      // For lefthanded:
      // [ cos(y), 0, -sin(y),
      //        0, 1,       0,
      //   sin(y), 0,  cos(y) ]
      //
      // You flip handedness by performing a matrix transpose, but in this case, that just ends 
      // up flipping the signs on two values.
      //
      rotation_matrix out;
      auto& d = out.data;
      double c = std::cos(radians);
      double s = std::sin(radians);
      d[0][0] = c;
      d[0][1] = 0.0F;
      d[0][2] = righthanded ? s : -s;
      d[1][0] = 0.0F;
      d[1][1] = 1.0F;
      d[1][2] = 0.0F;
      d[2][0] = righthanded ? -s : s;
      d[2][1] = 0.0F;
      d[2][2] = c;
      return out;
   }
   /*static*/ rotation_matrix rotation_matrix::construct_from_z(double radians, bool righthanded) {
      //
      // For lefthanded:
      // [ cos(z), sin(z), 0,
      //  -sin(z), cos(z), 0,
      //        0,      0, 1 ]
      //
      // You flip handedness by performing a matrix transpose, but in this case, that just ends 
      // up flipping the signs on two values.
      //
      rotation_matrix out;
      auto& d = out.data;
      double c = std::cos(radians);
      double s = std::sin(radians);
      d[0][0] = c;
      d[0][1] = righthanded ? -s : s;
      d[0][2] = 0.0F;
      d[1][0] = righthanded ? s : -s;
      d[1][1] = c;
      d[1][2] = 0.0F;
      d[2][0] = 0.0F;
      d[2][1] = 0.0F;
      d[2][2] = 1.0F;
      return out;
   }
   
   /*static*/ rotation_matrix rotation_matrix::construct_from_extrinsic_zyx(double x, double y, double z, bool righthanded) {
      rotation_matrix i = construct_from_z(z, righthanded);
      rotation_matrix j = construct_from_y(y, righthanded);
      i *= j;
      j = construct_from_x(x, righthanded);
      i *= j;
      return i;
   }
   //
   rotation_matrix::operator axis_angle() const {
      constexpr double EPSILON = 0.0001F;
      //
      axis_angle output;
      //
      output.angle = acos(_passive_atrig_clamp((this->trace() - 1.0F) / 2.0F));
      if (abs(output.angle) < EPSILON || isnan(output.angle)) { // if it's a 0-degree angle, then we can't compute it.
         //
         // Fall back to the Z-axis.
         //
         output.x = 0.0F;
         output.y = 0.0F;
         output.z = 1.0F;
         return output;
      } else if (abs(output.angle - pi) > EPSILON) { // not a 180-degree angle
         double a = 2.0F * sin(output.angle);
         output.x = (this->data[2][1] - this->data[1][2]) / a;
         output.y = (this->data[0][2] - this->data[2][0]) / a;
         output.z = (this->data[1][0] - this->data[0][1]) / a;
         return output;
      }
      //
      // A 180-degree angle tends to lead to a zero vector as our axis.
      // There seems to be a way to correct that...
      //
      // Source for the math: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToAngle/index.htm
      // Source for the math: http://sourceforge.net/p/mjbworld/discussion/122133/thread/912b44f7
      //
      if (abs(output.angle - pi) < 0.001F) {
         output.x = sqrt(this->data[0][0] + 1.0F) / 2.0F;
         output.y = sqrt(this->data[1][1] + 1.0F) / 2.0F;
         output.z = sqrt(this->data[2][2] + 1.0F) / 2.0F;
         //
         // We don't know the signs of the above terms. Per our second source, we can start 
         // to figure that out by finding the largest term, and then...
         //
         int i = output.highest_axis();
         int iSignX = data[i][0] < 0 ? -1 : 1;
         int iSignY = data[i][1] < 0 ? -1 : 1;
         int iSignZ = data[i][2] < 0 ? -1 : 1;
         output.x *= iSignX;
         output.y *= iSignY;
         output.z *= iSignZ;
      }
      //
      // Normalize the axis.
      //
      if (output.length() != 0.0F)
         output.normalize();
      else {
         //
         // Edge-case caused by zero-vector. Fall back to the Z-axis.
         //
         output.x = 0;
         output.y = 0;
         output.z = 1;
      }
      return output;
   }
   rotation_matrix::operator euler() const {
      euler output;
      //
      // So how do we even begin to convert from a rotation matrix back to Euler 
      // angles, when there are literally twenty-four possible conventions for 
      // Euler angles? (The possibilities are based on handedness, multiplication 
      // order, and what axes you're even using e.g. ZXZ if you're a weirdo. 
      // Intrinsic/extrinsic don't matter; you can go from one to the other just 
      // by reversing the order in which you apply the angles.)
      //
      // Well, handedness variations are just a matrix transpose, so really there 
      // are only twelve "unique" Euler conventions. Lefthanded extrinsic ZYX is 
      // this:
      //
      // [ cos(y)cos(z)                       ,  cos(y)sin(z)                       ,  -sin(y)      ,
      //   sin(x)sin(y)cos(z) - cos(x)sin(z)  ,  sin(x)sin(y)sin(z) + cos(x)cos(z)  ,  sin(x)cos(y) ,
      //   cos(x)sin(y)cos(z) + sin(x)sin(z)  ,  cos(x)sin(y)sin(z) - sin(x)cos(z)  ,  cos(x)cos(y) ]
      // 
      // Every single Euler convention has a single entry in its rotation matrix 
      // that's literally just the cosine or sine of one single axis, possibly 
      // negated. That's where we start, then: we undo that (co)sine operation to 
      // solve for that axis. My function assumes lefthanded extrinsic ZYX, so for 
      // us, that element in the matrix is -sin(y) at matrix position (0, 2), and 
      // so we do this:
      //
      output.y = asin(_passive_atrig_clamp(-this->data[0][2]));
      double fCosY = cos(output.y);
      //
      // That axis is our skeleton key. In our case, we cracked it from a sine, 
      // right? Well, there are a few other elements in the matrix that are 
      // just the cosines or sines of other axes multiplied by the cosine of 
      // this axis, so as long as the cosine of the axis we just cracked is 
      // non-zero, we can decode those, too.
      //
      if (fabs(fCosY) > 0.0005F) {
         //
         // Let's start by decoding X. We can see that matrix element (1, 2) 
         // is cos(y) * sin(x), while matrix element (2, 2) is cos(x) * cos(y). 
         // So which do we use? The sine of X or the cosine of X?
         //
         // We use both. We isolate them and pass them into atan2, treating 
         // the cosine of X as the "X" for atan2 and the sine of X as the "Y" 
         // for atan2.
         //
         // Source: https://web.archive.org/web/20051124013711/http://skal.planet-d.net/demo/matrixfaq.htm#Q37
         //
         double u = this->data[2][2] / fCosY;
         double v = this->data[1][2] / fCosY;
         output.x = atan2(v, u);
         //
         // And we can do the same for Z:
         //
         u = this->data[0][0] / fCosY;
         v = this->data[0][1] / fCosY;
         output.z = atan2(v, u);
         return output;
      }
      //
      // If we reach this point, then cos(y) was zero, which means that we're 
      // gimbal locked. We'll have to compromise: we'll assume that X is zero, 
      // and dump everything into Z. If X is zero, then that means that cos(X) 
      // is 1 and sin(X) is 0, and knowing that, we can take some elements in 
      // the matrix above and simplify them.
      //
      // Element (1, 1): sin(x)sin(y)sin(z) + cos(x)cos(z) -> 0 + 1cos(z) -> cos(z)
      // Element (1, 0): sin(x)sin(y)cos(z) - cos(x)sin(z) -> 0 - 1sin(z) -> -sin(z)
      //
      // And those elements can be further simplified into:
      //
      // Element (1, 1): cos(z)
      // Element (1, 0): -sin(z)
      //
      // And as above, we can derive (z) by using atan2 on both of those values.
      //
      double u = this->data[1][1];
      double v = -this->data[1][0];
      output.x = 0.0F;
      output.z = atan2(v, u);
      return output;
   }
   rotation_matrix::operator quaternion() const {
      quaternion q;
      //
      // Shoemake's 1987 algorithm that literally everyone uses. Source has apparently 
      // been lost to time.
      //
      double a = this->trace();
      if (a > 0.0F) {
         a = sqrt(a + 1.0F);
         q.w = a / 2.0F;
         a = 0.5F / a;
         q.x = (this->data[2][1] - this->data[1][2]) * a;
         q.y = (this->data[0][2] - this->data[2][0]) * a;
         q.z = (this->data[1][0] - this->data[0][1]) * a;
      } else {
         uint8_t i = 0;
         if (this->data[1][1] > this->data[0][0])
            i = 1;
         if (this->data[2][2] > this->data[i][i])
            i = 2;
         uint8_t j = (i + 1) % 3;
         uint8_t k = (j + 1) % 3;
         //
         a = sqrt(this->data[i][i] - this->data[j][j] - this->data[k][k] + 1.0F);
         q[i + 1] = a / 2.0F; // i == 0, 1, 2 -> x, y, z
         if (a)
            a = 0.5 / a;
         q.w = (this->data[k][j] - this->data[j][k]) * a;
         q[j + 1] = (this->data[j][i] + this->data[i][j]) * a;
         q[k + 1] = (this->data[k][i] + this->data[i][k]) * a;
      }
      return q;
   };
   #pragma endregion

   #pragma region axis_angle
   axis_angle::operator euler() const { return (euler)(rotation_matrix)*this; }
   axis_angle::operator rotation_matrix() const {
      rotation_matrix output;
      //
      double c = cos(this->angle);
      double s = sin(this->angle);
      double inv_c = 1.0F - c;
      //
      double length = this->length();
      double x = this->x / length;
      double y = this->y / length;
      double z = this->z / length;
      //
      output.data[0][0] = c + pow(x, 2) * inv_c; // top row
      output.data[0][1] = x * y * inv_c - z * s;
      output.data[0][2] = x * z * inv_c - y * s;
      output.data[1][0] = y * x * inv_c + z * s; // middle row
      output.data[1][1] = c + pow(y, 2) * inv_c;
      output.data[1][2] = y * z * inv_c - x * s;
      output.data[2][0] = z * x * inv_c - y * s; // bottom row
      output.data[2][1] = z * y * inv_c + x * s;
      output.data[2][2] = c + pow(z, 2) * inv_c;
      //
      return output;
   }
   axis_angle::operator quaternion() const {
      quaternion output;
      double half_angle = this->angle / 2;
      double hf_sin     = sin(half_angle);
      double length     = this->length();
      output.w = cos(half_angle);
      output.x = hf_sin * (x / length);
      output.y = hf_sin * (y / length);
      output.z = hf_sin * (z / length);
      return output;
   }
   #pragma endregion

   #pragma region euler
   euler euler::operator*(double v) const noexcept {
      euler copy = *this;
      copy *= v;
      return copy;
   }
   euler& euler::operator*=(double v) noexcept {
      this->x *= v;
      this->y *= v;
      this->z *= v;
      return *this;
   }
   euler euler::operator/(double v) const noexcept {
      euler copy = *this;
      copy /= v;
      return copy;
   }
   euler& euler::operator/=(double v) noexcept {
      this->x /= v;
      this->y /= v;
      this->z /= v;
      return *this;
   }
   //
   euler::operator axis_angle() const { return (axis_angle)(rotation_matrix)*this; }
   euler::operator rotation_matrix() const {
      rotation_matrix output;
      //
      double sx = sin(this->x);
      double sy = sin(this->y);
      double sz = sin(this->z);
      double cx = cos(this->x);
      double cy = cos(this->y);
      double cz = cos(this->z);
      //
      // Build the matrix. The matrix for lefthanded extrinsic XYZ is:
      //
      // [ cos(y)cos(z)                       ,  cos(y)sin(z)                       ,  -sin(y)      ,
      //   sin(x)sin(y)cos(z) - cos(x)sin(z)  ,  sin(x)sin(y)sin(z) + cos(x)cos(z)  ,  sin(x)cos(y) ,
      //   cos(x)sin(y)cos(z) + sin(x)sin(z)  ,  cos(x)sin(y)sin(z) - sin(x)cos(z)  ,  cos(x)cos(y) ]
      //
      output.data[0][0] = cy*cz; // top row
      output.data[0][1] = cy*sz;
      output.data[0][2] = -sy;
      output.data[1][0] = sx*sy*cz - cx*sz; // middle row
      output.data[1][1] = sx*sy*sz + cx*cz;
      output.data[1][2] = sx*cy;
      output.data[2][0] = cx*sy*cz + sx*sz; // bottom row
      output.data[2][1] = cx*sy*sz - sx*cz;
      output.data[2][2] = cx*cy;
      //
      return output;
   }
   euler::operator quaternion() const { return (quaternion)(rotation_matrix)*this; }
   #pragma endregion

   #pragma region quaternion
   #pragma region operators
   quaternion quaternion::operator+(const quaternion& other) const noexcept {
      quaternion result;
      result.w = this->w + other.w;
      result.x = this->x + other.x;
      result.y = this->y + other.y;
      result.z = this->z + other.z;
      return result;
   }
   quaternion quaternion::operator*(const quaternion& other) const noexcept {
      quaternion result;
      result.w = (this->w * other.w) - (this->x * other.x) - (this->y * other.y) - (this->z * other.z);
      result.x = (this->w * other.x) + (this->x * other.w) + (this->y * other.z) - (this->z * other.y);
      result.y = (this->w * other.y) - (this->x * other.z) + (this->y * other.w) + (this->z * other.x);
      result.z = (this->w * other.z) + (this->x * other.y) - (this->y * other.x) + (this->z * other.w);
      return result;
   }
   quaternion quaternion::operator*(double other) const noexcept {
      quaternion result;
      result.w = this->w * other;
      result.x = this->x * other;
      result.y = this->y * other;
      result.z = this->z * other;
      return result;
   }
   quaternion quaternion::operator/(double other) const noexcept {
      quaternion result;
      result.w = this->w / other;
      result.x = this->x / other;
      result.y = this->y / other;
      result.z = this->z / other;
      return result;
   }
   quaternion quaternion::operator+=(const quaternion& other) noexcept {
      this->w += other.w;
      this->x += other.x;
      this->y += other.y;
      this->z += other.z;
      return *this;
   }
   quaternion quaternion::operator*=(const quaternion& other) noexcept {
      *this = *this * other;
      return *this;
   }
   quaternion quaternion::operator*=(double other) noexcept {
      this->w *= other;
      this->x *= other;
      this->y *= other;
      this->z *= other;
      return *this;
   }
   quaternion quaternion::operator/=(double other) noexcept {
      this->w /= other;
      this->x /= other;
      this->y /= other;
      this->z /= other;
      return *this;
   }
   #pragma endregion
   quaternion::operator axis_angle() const { return (axis_angle)(rotation_matrix)*this; }
   quaternion::operator euler() const { return (euler)(rotation_matrix)*this; }
   quaternion::operator rotation_matrix() const {
      quaternion source = *this;
      //source.normalize();
      //
      rotation_matrix output;
      output.data[0][0] = 1.0F - 2.0F * pow(source.y, 2) - 2.0F * pow(source.z, 2);
      output.data[0][1] = (2.0F * source.x * source.y) - (2.0F * source.z * source.w);
      output.data[0][2] = (2.0F * source.x * source.z) + (2.0F * source.y * source.w);
      output.data[1][0] = (2.0F * source.x * source.y) + (2.0F * source.z * source.w);
      output.data[1][1] = 1.0F - 2.0F * pow(source.x, 2) - 2.0F * pow(source.z, 2);
      output.data[1][2] = (2.0F * source.y * source.z) - (2.0F * source.x * source.w);
      output.data[2][0] = (2.0F * source.x * source.z) - (2.0F * source.y * source.w);
      output.data[2][1] = (2.0F * source.y * source.z) + (2.0F * source.x * source.w);
      output.data[2][2] = 1.0F - 2.0F * pow(source.x, 2) - 2.0F * pow(source.y, 2);
      return output;
   };
   #pragma endregion

   #pragma region coordinates
   coordinates coordinates::apply(const coordinates& offset) const {
      coordinates out;
      {  // Construct position.
         out.position = this->position;
         if (offset.position[0] || offset.position[1] || offset.position[2]) {
            auto distance = (rotation_matrix)this->rotation * offset.position; // TODO: can quaternion * vector * quaternion.inverse() do the same thing?
            out.position[0] += distance[0];
            out.position[1] += distance[1];
            out.position[2] += distance[2];
         }
      }
      {  // Construct rotation.
         quaternion qParent = (quaternion)this->rotation;
         quaternion qOffset = (quaternion)offset.rotation;
         out.rotation = (euler)(qParent * qOffset);
      }
      return out;
   }
   coordinates coordinates::convert_to_absolute(const coordinates& basis) const {
      return basis.apply(*this);
   }
   coordinates coordinates::convert_to_relative(const coordinates& basis) const {
      coordinates out;
      {
         quaternion parent = (quaternion)basis.rotation;
         quaternion offset = (quaternion)this->rotation;
         out.rotation = (euler)(parent.inverse() * offset);
      }
      {
         rotation_matrix world = (rotation_matrix)basis.rotation;
         world.transpose_in_place();
         out.position = world * this->position;
         //
         // Convert the parent's world-relative position to parent-relative coordinates, and then 
         // subtract it from the output position.
         //
         auto local = world * basis.position;
         out.position[0] -= local[0];
         out.position[1] -= local[1];
         out.position[2] -= local[2];
      }
      return out;
   }
   #pragma endregion
}