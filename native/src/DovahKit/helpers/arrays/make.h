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
#include <type_traits>
#include "../concepts.h"

namespace cobb::arrays {

   namespace impl::_make {
      template<typename T> struct is_string_literal {
         static constexpr bool value = false;
      };
      template<size_t N> struct is_string_literal<const char(&)[N]> {
         static constexpr bool value = true;
      };

      template<typename... Types> using final_value_type = std::conditional_t<
         (is_string_literal<Types>::value && ...),
         const char*,
         std::tuple_element_t<0, std::tuple<std::decay_t<Types>...>>
      >;
   }

   //
   // Helper template for creating an array of an arbitrary length but with a specific 
   // desired type. Includes a specialization to ensure that string literals are used 
   // as `const char*` and not `const char(&)[N]`.
   //
   template<typename... Types> requires (cobb::all_same<Types...> || (impl::_make::is_string_literal<Types>::value && ...))
   constexpr std::array<impl::_make::final_value_type<Types...>, sizeof...(Types)> make(Types&&... args) {
      std::array<impl::_make::final_value_type<Types...>, sizeof...(Types)> out = { args... };
      return out;
   };
}
