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
   namespace impl::_concat {
      template<class, class> struct two;
      template<typename... A, typename... B> struct two<std::tuple<A...>, std::tuple<B...>> {
         using type = std::tuple<A..., B...>;
      };

      template<class... T> struct arbitrary {
         static constexpr auto count = sizeof...(T);
         using full_list = std::tuple<T...>;
         template<size_t n> using nth_type = typename std::tuple_element<n, full_list>::type;

         // Recursively merge pairs of tuples:
         //    a, b, c, d, e, f, g, h
         //    ab, cd, ef, gh
         //    abcd, efgh
         //    abcdefgh

         using half_sequence = std::make_index_sequence<count / 2>;

         template<size_t I> using merge_single_pair = two<nth_type<I * 2>, nth_type<I * 2 + 1>>::type;

         template<typename> struct recursive_merge_pairs;
         template<size_t... I> struct recursive_merge_pairs<std::index_sequence<I...>> {
            using type = arbitrary<
               merge_single_pair<I>...
            >::type;
         };

         // Handles the case of there being nothing left to merge, and the case of there 
         // being an odd number of tuples to merge (such that there's an odd one out and 
         // merging pairs is not sufficient).
         struct merge_all {
            using type = std::conditional_t<
               count == 1,
               nth_type<0>,
               std::conditional_t<
                  (count % 2 == 0),
                  typename recursive_merge_pairs<half_sequence>::type,
                  typename two<typename recursive_merge_pairs<half_sequence>::type, nth_type<count - 1>>::type
               >
            >;
         };

         // Final result.
         using type = merge_all::type;
      };
      template<> struct arbitrary<> {
         using type = std::tuple<>;
      };
   }

   // Concatenate an arbitrary number of tuples or tuple-like types together at compile time.
   template<class... T> using concat = impl::_concat::arbitrary<T...>::type;
}
