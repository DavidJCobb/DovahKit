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
#include <type_traits>

namespace cobb {
   template<typename T> requires (std::is_arithmetic_v<T>)
   constexpr T factorial(unsigned int n) noexcept {
      T result = T(1);
      for (int i = 2; i <= n; ++i)
         result *= i;
      return result;
   }

   // Function for incremental factorials, when running loops in which you are 
   // taking the factorial of an increasingly large number.
   // 
   // factorial(n) == factorial_to(n, s) * factorial(s)
   template<typename T> requires (std::is_arithmetic_v<T>)
   constexpr T factorial_to(unsigned int n, unsigned int s) noexcept {
      T result = T(1);
      for (int i = s + 1; i <= n; ++i)
         result *= i;
      return result;
   }
   static_assert(factorial_to<double>(4, 0) == factorial<double>(4)); // s == 0 should produce a valid result
   static_assert(factorial_to<double>(4, 1) == factorial<double>(4)); // s == 1 should produce a valid result

   template<typename T, typename exponent_t> requires (std::is_arithmetic_v<T> && std::is_integral_v<exponent_t>)
   T constexpr pow(T base, exponent_t exponent) noexcept {
      return exponent == 0 ? 1 : base * pow(base, exponent - 1);
   }

   template<typename T> auto constexpr sign(T v) noexcept {
      return (T(0) < v) - (v < T(0));
   }

   // Cosine function implemented via a Taylor series to an arbitrary number of iterations. You should 
   // never actually need this.
   template<int iterations = 5, typename T> requires (std::is_arithmetic_v<T> && iterations > 1)
   constexpr T cosine(T angle) noexcept {
      int sign   = -1;
      T   result =  1;
      //
      T numerator   = 1;
      T denominator = 1;
      for (int i = 1; i < iterations; ++i, sign = -sign) {
         numerator   *= angle * angle;                       // numerator   == cobb::pow(angle, 2 * i)
         denominator *= factorial_to<T>(2 * i, 2 * (i - 1)); // denominator == cobb::factorial(2 * i)  // Given i == 1 this should be factorial_to(2, 0) which should be valid
         result += sign * numerator / denominator;
      }
      return result;
   }

   // Sine function implemented via a Taylor series to an arbitrary number of iterations. You should 
   // never actually need this.
   template<int iterations = 5, typename T> requires (std::is_arithmetic_v<T> && iterations > 1)
   constexpr T sine(T angle) noexcept {
      int sign   = -1;
      T   result = angle;
      //
      T numerator   = angle;
      T denominator = 1;
      for (int i = 1; i < iterations; ++i, sign = -sign) {
         numerator   *= angle * angle;                               // numerator   == cobb::pow(angle, 2 * i + 1)
         denominator *= factorial_to<T>(2 * i + 1, 2 * (i - 1) + 1); // denominator == cobb::factorial(2 * i + 1)  // Given i == 1 this should be factorial_to(3, 1) which should be valid
         result += sign * numerator / denominator;
      }
      return result;
   }
}