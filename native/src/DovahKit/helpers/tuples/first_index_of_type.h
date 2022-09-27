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
#include <tuple>
#include <type_traits>

namespace cobb::tuples {
   namespace impl {
      template<typename Tuple, typename Desired>
      struct _first_index_of_type;

      template<typename Desired, typename... Types>
      struct _first_index_of_type<std::tuple<Types...>, Desired> {
         static constexpr std::size_t value = [](){
            std::size_t i = 0;
            ((std::is_same_v<Types, Desired> || (++i, false)) || ...);
            if (i >= sizeof...(Types))
               return (std::size_t)-1;
            return i;
         }();
      };
   }

   template<typename Tuple, typename T>
   constexpr std::size_t first_index_of_type = impl::_first_index_of_type<Tuple, T>::value;
}