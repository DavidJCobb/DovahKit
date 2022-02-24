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
#include <utility>

namespace cobb {
   namespace impl {
      template<size_t Count, typename = std::make_index_sequence<Count>> struct array_of_n_values;
      template<size_t Count, size_t... Indices> struct array_of_n_values<Count, std::index_sequence<Indices...>> {
         template<typename T> static consteval std::array<T, Count> value(T v) {
            return { (Indices, v)... };
         }
      };
   }
   template<size_t Count, typename T> consteval std::array<T, Count> array_of_n_values(T v) {
      return impl::array_of_n_values<Count>::value(v);
   }
}