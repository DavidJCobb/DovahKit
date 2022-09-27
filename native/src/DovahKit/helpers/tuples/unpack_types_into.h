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
      template<template<typename...> class UnpackInto, typename Tuple> struct _unpack_types_into;
      template<template<typename...> class UnpackInto, typename... Types> struct _unpack_types_into<UnpackInto, std::tuple<Types...>> {
         using type = UnpackInto<Types...>;
      };
   }

   //
   // Unpack a tuple's type list and pass the types as parameters into another template.
   //
   template<typename Tuple, template<typename...> class UnpackInto> using unpack_types_into = typename impl::_unpack_types_into<UnpackInto, Tuple>::type;
}