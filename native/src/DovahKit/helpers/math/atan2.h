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
#include <numbers>
#include <type_traits>
#include "./factorial.h"

namespace cobb {
   namespace ct { // namespace for compile-time code
      // Atan2 function implemented via a Taylor series to an arbitrary number of iterations.
      // Slower than the built-in, but constexpr-compatible.
      template<int iterations = 10, typename T> requires (std::is_arithmetic_v<T> && iterations > 1)
      constexpr T atan2(T y, T x) noexcept {
         if (x < 0.00005) {
            static constexpr auto half_pi = (std::numbers::pi_v<T> / T{ 2 });
            if (y < 0)
               return -half_pi;
            return half_pi;
         }

         T angle = y / x;
         if (angle > T{1}) {
            angle -= (std::intmax_t)angle;
         } else if (angle < T{-1}) {
            angle += (std::intmax_t)(-angle);
         }

         int sign   = -1;
         T   result = angle;
         //
         T numerator = angle;
         for (int i = 1; i < iterations; ++i) {
            numerator *= angle * angle; // numerator == cobb::pow(angle, 2 * i + 1)
            result += sign * numerator / (2 * i + 1);
         }

         if (x < 0) {
            if (y >= 0)
               result += std::numbers::pi_v<T>;
            else if (y < 0)
               result -= std::numbers::pi_v<T>;
         }

         return result;
      }
   }

   template<int iterations = 10, typename T> requires (std::is_arithmetic_v<T> && iterations > 1)
   constexpr T atan2(T angle) noexcept {
      if (std::is_constant_evaluated()) {
         return ct::atan2(angle);
      } else {
         return std::atan2(angle);
      }
   }
}