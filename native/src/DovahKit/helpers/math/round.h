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
#include <cstdint>
#include <limits>
#include <type_traits>
#include "./sign.h"

namespace cobb {
   namespace impl::round {
      // constant defined here in an attempt to reduce constexpr steps and avoid hitting compiler safety limits
      template<typename T> static constexpr T near_half = T(0.5) - std::numeric_limits<T>::epsilon();
   }

   namespace ct {
      template<typename T> requires std::is_floating_point_v<T> constexpr int64_t round(T v) {
         return v + ::cobb::impl::round::near_half<T> *sign(v); // return value is int, so we truncate after this
      }
   }

   template<typename T> requires std::is_floating_point_v<T> constexpr int64_t round(T v) {
      if (!std::is_constant_evaluated()) {
         return ::round(v);
      }
      return ct::round(v);
   }
}