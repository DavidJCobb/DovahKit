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
#include <bit>
#include <limits>
#include <type_traits>

namespace cobb {
   template<typename T, typename ExponentType> requires (std::is_arithmetic_v<T> && std::is_integral_v<ExponentType>)
   constexpr T pow(T base, ExponentType exponent) noexcept {
      #if __cpp_lib_int_pow2 >= 202002L
         if constexpr (std::is_integral_v<T>) {
            if (std::has_single_bit(exponent)) {
               return base << exponent;
            }
            auto two = std::bit_floor(exponent);
            if (two)
               return (base << two) * pow(base, exponent - two);
         }
      #endif
      return exponent == 0 ? 1 : base * pow(base, exponent - 1);
   }
}