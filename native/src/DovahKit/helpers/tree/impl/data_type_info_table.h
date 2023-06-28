#pragma once
#include <array>
#include "../../class_array.h"
#include "./strip_node_data_options.h"

namespace cobb::impl::_node {
   template<typename... Parameters>
   struct data_type_info_table {
      data_type_info_table() = delete;

      using all_types = cobb::class_array<typename strip_node_data_options<Parameters>::type...>;

      using destroy_handler_type = void(*)(void*);
      
      static constexpr const auto destroy_handlers = []() {
         std::array<void(*)(void*), count> out = {};

         size_t i = 0;
         all_types::for_each([&i, &out]<typename Data>() {
            out[i++] = [](void* n) { ((Data*)n)->~Data(); };
         });

         return out;
      }();
   };
}