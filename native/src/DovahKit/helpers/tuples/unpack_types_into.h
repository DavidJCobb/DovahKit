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