#pragma once
#include <array>
#include <type_traits>
#include "../../class_array.h"
#include "./typecode.h"

namespace cobb::impl::_node {
   template<typename TypeArray>
   struct data_destroyer;
   
   template<typename... Types> requires (!std::is_trivially_destructible_v<Types> || ...)
   struct data_destroyer<cobb::class_array<Types...>> {
      using all_types = cobb::class_array<Types...>;
      using typecode  = impl::_node::typecode<sizeof...(Types)>;
      //
      static constexpr const auto handlers = []() {
         std::array<void(*)(void*), sizeof...(Types)> out = {};

         size_t i = 0;
         all_types::for_each([&i, &out]<typename Data>() {
            out[i++] = [](void* n) { ((Data*)n)->~Data(); };
         });

         return out;
      }();

      static constexpr void destroy(void* data, typecode type) {
         (handlers[type])(data);
      }
   };
}