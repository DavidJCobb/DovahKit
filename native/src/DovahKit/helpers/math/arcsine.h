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
#include <type_traits>
#include "./factorial.h"

namespace cobb {
   namespace ct { // namespace for compile-time code
      // Cosine function implemented via a Taylor series to an arbitrary number of iterations.
      // Slower than the built-in, but constexpr-compatible.
      template<int iterations = 10, typename T> requires (std::is_arithmetic_v<T> && iterations > 1)
      constexpr T arcsine(T angle) noexcept {
         T result = angle;
         //
         T numerator_a   = angle;
         T numerator_b   = 1;
         T denominator_a = 1;
         T denominator_b = 1;
         for (int i = 1; i < iterations; ++i) {
            numerator_a   *= angle * angle; // == cobb::pow(angle, 2 * i + 1)
            numerator_b   *= factorial_to<T>(2 * i, 2 * (i - 1));
            denominator_a *= 4; // == cobb::pow(4, i)
            denominator_b *= i; // == cobb::factorial(i)
            result += (numerator_a * numerator_b) / (denominator_a * (denominator_b * denominator_b) * (2 * i + 1));
         }
         return result;
      }
   }

   template<int iterations = 10, typename T> requires (std::is_arithmetic_v<T> && iterations > 1)
   constexpr T arcsine(T angle) noexcept {
      if (std::is_constant_evaluated()) {
         return ct::arcsine(angle);
      } else {
         return std::asin(angle);
      }
   }
}