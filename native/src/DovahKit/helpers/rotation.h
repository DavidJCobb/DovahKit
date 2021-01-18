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
#pragma once
#include <array>
#include "matrix.h"

namespace cobb {
   class axis_angle;
   class euler;
   class quaternion;

   extern constexpr double pi = 3.14159265358979323846;

   inline double degrees_to_radians(double degrees) {
      return degrees * pi / 180.0;
   }
   inline double radians_to_degrees(double radians) {
      return radians * 180.0 / pi;
   }

   //
   // An explanation of Euler angles is provided in the CPP file. Euler conventions are 
   // explained at the top of the file. The cast operators for rotation matrices explain 
   // in detail how to manage converting to and from any Euler convention.
   //
   // I built this for working with Skyrim, which uses extrinsic lefthanded XYZ, so that 
   // is what most Euler conversion functions will use by default.
   //

   class rotation_matrix : public matrix<3, 3> {
      public:
         using super_t = matrix<3, 3>;
         //
         explicit operator axis_angle() const;
         explicit operator euler() const; // assumes lefthanded XYZ extrinsic, like in Skyrim
         explicit operator quaternion() const;

         rotation_matrix() {}
         rotation_matrix(const super_t& m) { memcpy(&this->data, &m.data, sizeof(data)); } // needed for matrix methods that return matrices
         rotation_matrix(super_t&& m) { memcpy(&this->data, &m.data, sizeof(data)); } // needed for matrix methods that return matrices

         double determinant() const noexcept;
         inline double trace() const noexcept {
            return this->data[0][0] + this->data[1][1] + this->data[2][2];
         }

         static rotation_matrix construct_from_x(double radians, bool righthanded); // Skyrim is lefthanded
         static rotation_matrix construct_from_y(double radians, bool righthanded);
         static rotation_matrix construct_from_z(double radians, bool righthanded);

         static rotation_matrix construct_from_extrinsic_zyx(double x, double y, double z, bool righthanded);

         std::array<double, 3> operator*(const std::array<double, 3>& vec) const noexcept { // loop-free multiply by column (equivalent to applying (this) to (vec) as a reference frame)
            std::array<double, 3> vResult;
            vResult[0] = (this->data[0][0] * vec[0]) + (this->data[0][1] * vec[1]) + (this->data[0][2] * vec[2]);
            vResult[1] = (this->data[1][0] * vec[0]) + (this->data[1][1] * vec[1]) + (this->data[1][2] * vec[2]);
            vResult[2] = (this->data[2][0] * vec[0]) + (this->data[2][1] * vec[1]) + (this->data[2][2] * vec[2]);
            return vResult;
         }

         using super_t::operator*=;
         rotation_matrix& operator*=(const rotation_matrix& other) {
            super_t::operator*=(other);
            return *this;
         }
         rotation_matrix transpose() const noexcept {
            rotation_matrix result = *this;
            result.transpose_in_place();
            return result;
         }
   };

   class axis_angle {
      public:
         double x = 0.0F;
         double y = 0.0F;
         double z = 0.0F;
         double angle = 0.0F; // radians

         inline double length() const noexcept {
            return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
         }
         void normalize() noexcept {
            double l = this->length();
            x /= l;
            y /= l;
            z /= l;
         }

         inline int highest_axis() const noexcept {
            if (this->x > this->y)
               if (this->x > this->z)
                  return 0;
            if (this->y > this->z)
               return 1;
            return 2;
         }

         explicit operator euler() const;
         explicit operator rotation_matrix() const;
         explicit operator quaternion() const;
   };

   class euler {
      public:
         double x = 0.0F; // radians
         double y = 0.0F; // radians
         double z = 0.0F; // radians

         euler operator*(double) const noexcept;
         euler& operator*=(double) noexcept;
         euler operator/(double) const noexcept;
         euler& operator/=(double) noexcept;

         explicit operator axis_angle() const;
         explicit operator rotation_matrix() const; // assumes lefthanded XYZ extrinsic, like in Skyrim
         explicit operator quaternion() const;
   };

   class quaternion { // note: only unit quaternions can represent rotations
      public:
         double w = 0.0F;
         double x = 0.0F;
         double y = 0.0F;
         double z = 0.0F;

         inline quaternion conjugate() const noexcept {
            return quaternion{ w, -x, -y, -z };
         }
         quaternion inverse() const noexcept {
            auto result = this->conjugate();
            result = result / pow(this->norm(), 2);
            return result;
         }
         double norm() const noexcept {
            return sqrt(this->w * this->w + this->x * this->x + this->y * this->y + this->z * this->z);
         }
         void normalize() noexcept {
            *this /= this->norm();
         }

         inline double& operator[](int i) {
            switch (i) {
               case 0: return this->w;
               case 1: return this->x;
               case 2: return this->y;
               case 3: return this->z;
            }
            __assume(0); // tell MSVC this is unreachable
         }
         inline const double& operator[](int i) const noexcept {
            switch (i) {
               case 0: return this->w;
               case 1: return this->x;
               case 2: return this->y;
               case 3: return this->z;
            }
            __assume(0); // tell MSVC this is unreachable
         }

         quaternion operator+(const quaternion& other) const noexcept;
         quaternion operator*(const quaternion& other) const noexcept; // "Hamilton product"
         quaternion operator*(double other) const noexcept;
         quaternion operator/(double other) const noexcept;
         quaternion operator+=(const quaternion& other) noexcept;
         quaternion operator*=(const quaternion& other) noexcept; // "Hamilton product"
         quaternion operator*=(double other) noexcept;
         quaternion operator/=(double other) noexcept;

         explicit operator axis_angle() const;
         explicit operator euler() const;
         explicit operator rotation_matrix() const;
   };

   struct coordinates {
      std::array<double, 3> position;
      euler rotation;
      //
      coordinates apply(const coordinates& offset) const;
      coordinates convert_to_absolute(const coordinates& basis) const; // (this) is relative; apply it to (basis)
      coordinates convert_to_relative(const coordinates& basis) const; // (this) is absolute; get the relative offset to (basis)
      //
      inline bool is_NaN() const noexcept {
         return isnan(this->position[0] + this->position[1] + this->position[2] + this->rotation.x + this->rotation.y + this->rotation.z); // branchless
      }
   };
}