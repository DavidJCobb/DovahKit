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

namespace cobb::tuples {
   namespace impl {
      template<template<typename> typename Transform, typename Tuple> struct _map_types;
      template<template<typename> typename Transform, typename... Types> struct _map_types<Transform, std::tuple<Types...>> {
         using type = std::tuple<typename Transform<Types>::type...>;
      };
   }

   // Given a std::tuple<A, B> and a transform struct, transform it into some std::tuple<C, D> where C and D are the result 
   // of running A and B through the transform. The transform should be defined as:
   //
   // template<typename T> struct my_transform { using type = some_permutation_of<T>; };
   //
   template<template<typename T> typename Transform, typename Tuple> using map_types = impl::_map_types<Transform, Tuple>::type;
}