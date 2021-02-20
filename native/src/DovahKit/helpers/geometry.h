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
#include <cmath>
#include <limits>
#include "math.h"
#include "rotation.h"
#include "vector3.h"

namespace cobb {
   template<typename N> class oriented_bounding_box {
      public:
         static_assert(std::is_arithmetic_v<N>, "cobb::sphere must be templated on a numeric type.");
         using number_type = N;
         using vector_type = cobb::vector3<number_type>;
         //
      public:
         vector_type min; // for a centered box, min == -max
         vector_type max;
         rotation_matrix rotation;
         //
         vector_type center() const noexcept {
            return this->halfwidths() + this->min;
         }
         vector_type halfwidths() const noexcept {
            return (this->max - this->min) / 2;
         }
         number_type length_along_axis(const vector_type& axis) const noexcept {
            vector_type halfwidths = (this->max - this->min) / 2;
            number_type axis_sq    = axis.square();
            number_type length     = number_type(0);
            //
            length += (vector_type(this->rotation.local_x_axis()) * halfwidths.x).dot(axis);
            length += (vector_type(this->rotation.local_y_axis()) * halfwidths.y).dot(axis);
            length += (vector_type(this->rotation.local_z_axis()) * halfwidths.z).dot(axis);
            //
            return length * 2;
         }
         number_type volume() const noexcept {
            vector_type diff = this->max - this->min;
            return std::abs(diff.x * diff.y * diff.z);
         }

         bool separating_axis_test(const vector_type& my_position, const vector_type& point_to_test) const noexcept {
            auto local = this->rotation.transpose();
            auto ptt_l = local * point_to_test.to_array();
            auto mp_l  = local * my_position.to_array();
            //
            for (int i = 0; i < vector_type::axis_count; ++i) {
               number_type pos = ptt_l[i];
               number_type min = this->min.components[i] + mp_l[i];
               number_type max = this->max.components[i] + mp_l[i];
               if (pos < min || pos > max)
                  return false;
            }
            return true;
         }

         // Based on <https://www.jkh.me/files/tutorials/Separating%20Axis%20Theorem%20for%20Oriented%20Bounding%20Boxes.pdf>
         bool separating_axis_test(const oriented_bounding_box& other) const noexcept {
            //
            // The very end of the section "Separating Axis Theorem and Boxes in 3D Space" (beginning at 
            // the text "Consider a box A and an axis L") explains the approach for testing whether two 
            // boxes overlap along a single separating axis. A later section, "Computing the Intersection 
            // of Two Oriented Bounding Boxes," lists the fifteen separating axes that one must test for 
            // any two oriented bounding boxes.
            //
            // In our code, (a_axes) holds Ax, Ay, and Az, while (b_axes) holds Bx, By, and Bz. Meanwhile, 
            // (half_a) holds Wa, Ha, and Da, with (half_b) holding Wb, Hb, and Db. The (center_diff) 
            // variable is T, and the axis being tested at any given moment is L.
            //
            // For each lambda, we are comparing the (max) value to the (sum) value. The PDF makes clear 
            // the meaning of this comparison in a later section titled "Optimized Computation of OBBs 
            // Intersections:" if the sum is below the maximum for any single axis, then that axis is 
            // separating the two boxes and so there is no overlap between them.
            //
            // Note also that this algorithm does not work properly for coplanar 2D planes in a 3D space. 
            // If two 2D planes in a 3D space have the same rotation and are lying coplanar with each 
            // other, then the only axes that will be checked for separation are those perpendicular to 
            // the faces, and so the true separating axis will not be found and the planes will falsely 
            // test as overlapping. A cheap hack to avoid this would be to ensure that all boxes are at 
            // least hair-thick before running the test.
            //
            vector_type a_axes[vector_type::axis_count];
            vector_type b_axes[vector_type::axis_count];
            {
               auto local_a = this->rotation.transpose();
               auto local_b = other.rotation.transpose();
               for (int i = 0; i < vector_type::axis_count; ++i) {
                  a_axes[i] = vector_type(local_a.column(i));
                  b_axes[i] = vector_type(local_b.column(i));
               }
            }
            //
            vector_type center_diff = this->center() - other.center(); // T
            vector_type half_a = this->halfwidths();
            vector_type half_b = other.halfwidths();
            auto _axis_does_separate = [&center_diff, &half_a, &half_b, &a_axes, &b_axes](const vector_type& axis) {
               number_type sum = 0.0F;
               number_type max = center_diff.dot(axis);
               for (int i = 0; i < vector_type::axis_count; ++i) {
                  sum += std::abs((a_axes[i] * half_a.components[i]).dot(axis));
                  sum += std::abs((b_axes[i] * half_b.components[i]).dot(axis));
                  if (sum >= max)
                     return false;
               }
               return true;
            };
            //
            for (int i = 0; i < vector_type::axis_count; ++i) // A axes
               if (_axis_does_separate(a_axes[i]))
                  return false;
            for (int i = 0; i < vector_type::axis_count; ++i) // B axes
               if (_axis_does_separate(b_axes[i]))
                  return false;
            for (int i = 0; i < vector_type::axis_count; ++i)
               for (int j = 0; j < vector_type::axis_count; ++j) // merged axes: A[i] * B[j]
                  if (_axis_does_separate(a_axes[i] * b_axes[j]))
                     return false;
            return true;
         }

         oriented_bounding_box& operator+=(number_type margin) noexcept {
            this->min -= margin;
            this->max += margin;
            return *this;
         }
         oriented_bounding_box& operator-=(number_type margin) noexcept {
            this->min += margin;
            this->max -= margin;
            return *this;
         }
         oriented_bounding_box& operator*=(number_type scale) noexcept {
            this->min *= scale;
            this->max *= scale;
            return *this;
         }
         oriented_bounding_box& operator/=(number_type scale) noexcept {
            this->min /= scale;
            this->max /= scale;
            return *this;
         }
   };

   template<typename N> class sphere {
      public:
         static_assert(std::is_arithmetic_v<N>, "cobb::sphere must be templated on a numeric type.");
         using number_type = N;
         using vector_type = cobb::vector3<number_type>;
         //
      public:
      vector_type position;
         number_type radius = 0.0F; // if this is negative, expect undefined behavior
         //
         sphere() {}
         sphere(number_type x, number_type y, number_type z, number_type r) : position(x, y, z), radius(r) {}
         sphere(const vector_type& p, number_type r) : position(p), radius(r) {}
         //
         inline bool contains(const vector_type& point) const noexcept {
            return this->distance_to(point) <= number_type(0.0);
         }
         number_type distance_to(const vector_type& point) const noexcept { // distance from the sphere's surface to the point; values equal to or below zero indicate that the point is inside the sphere
            return (this->position - point).length() - this->radius;
         }
         bool overlaps(const sphere& other) const noexcept {
            number_type distance = (this->position - other.position).length();
            return (distance < (this->radius + other.radius));
         }

         #pragma region Point-line intersections
         //
         // Check the number of times a line of infinite length intersects this sphere. If you wish to 
         // test whether a line segment (i.e. a line of finite length) intersects the sphere, then use 
         // (intersects_line_segment) instead.
         //
         template<int precision = 4> // number of decimal places to use for the epsilon, e.g. 4 -> 0.0001
         int intersects_line(const vector_type& line_start, const vector_type& line_direction) const noexcept {
            static_assert(precision > 0, "The precision (number of decimal places to use for the epsilon) cannot be negative.");
            constexpr number_type epsilon = cobb::pow(number_type(10), -precision);
            //
            auto u = line_direction;
            u.normalize();
            //
            auto gap   = line_start - this->position;
            auto delta = cobb::pow(u.dot(gap), 2) - (gap.length_sq() - (this->radius * this->radius));
            if (delta < -epsilon) {
               return 0;
            } else if (delta >= -epsilon && delta <= epsilon) {
               return 1;
            } else if (delta > epsilon) {
               return 2;
            }
            __assume(0); // unreachable
         }

         //
         // Check the number of times a line segment (i.e. a line of finite length) intersects the sphere. 
         // Note that this function takes the line segment's start and end points, rather than a start 
         // point and a direction vector.
         //
         template<int precision = 4> // number of decimal places to use for the epsilon, e.g. 4 -> 0.0001
         int intersects_line_segment(const vector_type& line_start, const vector_type& line_end) const noexcept {
            static_assert(precision > 0, "The precision (number of decimal places to use for the epsilon) cannot be negative.");
            constexpr number_type epsilon = cobb::pow(number_type(10), -precision);

            auto u = (line_end - line_start);
            number_type max_distance = u.length();
            u.normalize();
            //
            auto gap = line_start - this->position;
            auto delta = std::pow(u.dot(gap), 2) - (gap.length_sq() - (this->radius * this->radius));
            if (delta < -epsilon) {
               return 0;
            } else if (delta >= -epsilon && delta <= epsilon) {
               number_type distance = -(u.dot(gap)); // the distance is this stuff +/- delta, but we have no delta, so...
               if (distance > max_distance)
                  return 0;
               return 1;
            } else if (delta > epsilon) {
               number_type intermediate = -u.dot(gap);
               number_type distance_one = intermediate - delta;
               number_type distance_two = intermediate + delta;
               int result = 2;
               if (distance_one < -epsilon || distance_one > max_distance)
                  --result;
               if (distance_two < -epsilon || distance_two > max_distance)
                  --result;
               return result;
            }
            __assume(0); // unreachable
         }

         //
         // Check the number of times a line of infinite length intersects this sphere, and return both the 
         // positions of those intersections and their distances to the chosen line start point.
         //
         // If you wish to test whether a line segment (i.e. a line of finite length) intersects the sphere, 
         // then just ignore intersections whose distances are negative or exceed the length of the line.
         //
         template<int precision = 4> // number of decimal places to use for the epsilon, e.g. 4 -> 0.0001
         int get_line_intersections( // returns the number of intersections
            //
            // Inputs:
            //
            const vector_type& line_start,
            const vector_type& line_direction, // if you're starting with a line segment, you can just pass in (end - start); we'll normalize it for you
            //
            // Results:
            //
            vector_type& intersection_one, // location of the first intersection,  or a vector of NaN if there isn't one
            vector_type& intersection_two, // location of the second intersection, or a vector of NaN if there isn't one
            number_type distance_one, // distance from the line-start to the first  intersection, or NaN if there is no such intersection
            number_type distance_two  // distance from the line-start to the second intersection, or NaN if there is no such intersection
         ) const noexcept {
            static_assert(precision > 0, "The precision (number of decimal places to use for the epsilon) cannot be negative.");
            constexpr number_type epsilon = cobb::pow(number_type(10), -precision);
            constexpr number_type nan     = std::numeric_limits<number_type>::quiet_NaN();
            constexpr vector_type none    = { nan, nan, nan };

            auto u = line_direction;
            u.normalize();
            //
            auto gap   = line_start - this->position;
            auto delta = std::pow(u.dot(gap), 2) - (gap.length_sq() - (this->radius * this->radius));
            if (delta < -epsilon) {
               intersection_one = none;
               intersection_two = none;
               distance_one = nan;
               distance_two = nan;
               return 0;
            } else if (delta >= -epsilon && delta <= epsilon) {
               intersection_two = none;
               distance_two = nan;
               //
               distance_one = -(u.dot(gap)); // the distance is this stuff +/- delta, but we have no delta, so...
               intersection_one = line_start + (line_direction * distance_one);
               return 1;
            } else if (delta > epsilon) {
               number_type intermediate = -u.dot(gap);
               distance_one = intermediate - delta;
               distance_two = intermediate + delta;
               intersection_one = line_start + (line_direction * distance_one);
               intersection_two = line_start + (line_direction * distance_two);
               return 2;
            }
            __assume(0); // unreachable
         }
         #pragma endregion
   };
}
