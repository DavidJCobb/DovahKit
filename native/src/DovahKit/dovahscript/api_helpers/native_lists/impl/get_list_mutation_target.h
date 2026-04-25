#pragma once
#include <tuple>
#include "helpers/lua/error.h"
#include "../member_function_spec.h"

namespace dovahscript::api_helpers::native_lists::impl {
   // Takes: wrapper; zero-based index within list
   // Returns: (sub-)list to modify; zero-based index within (sub-)list
   template<typename Spec>
   std::tuple<typename Spec::collection_wrapped_type*, size_t, size_t> get_list_mutation_target(lua_State* L, wrapper& self, size_t requested_index) {
      if constexpr (impl::unwrap_collection::is_single<Spec>) {
         return { Spec::unwrap_collection(self), requested_index, requested_index };
      } else {
         static_assert(impl::unwrap_collection::is_bifurcated<Spec>);
         const auto pair = Spec::unwrap_collection(self);
         if (pair.first) {
            const size_t no_no_threshold = pair.first->size();
            if (requested_index < no_no_threshold)
               cobb::lua::error(L, "cannot modify items within the locked part of the list");
            requested_index -= no_no_threshold;
         }
         return { pair.second, requested_index, requested_index };
      }
   }

   // Takes: wrapper; zero-based index within list
   // Returns: (sub-)list to modify; zero-based index within (sub-)list; zero-based sub-object index
   template<typename Spec>
   std::tuple<typename Spec::collection_wrapped_type*, size_t, size_t> get_list_mutation_target(lua_State* L, wrapper& self, std::optional<size_t> requested_index) {
      if constexpr (impl::unwrap_collection::is_single<Spec>) {
         auto*  list_ptr = Spec::unwrap_collection(self);
         size_t index;
         if (requested_index.has_value()) {
            index = requested_index.value();
         } else {
            index = list_ptr->size();
         }
         return { list_ptr, index, index };
      } else {
         static_assert(impl::unwrap_collection::is_bifurcated<Spec>);
         const auto pair = Spec::unwrap_collection(self);
         if (requested_index.has_value()) {
            size_t subobject_index = requested_index.value();
            size_t sublist_index   = subobject_index;
            if (pair.first) {
               size_t size = pair.first->size();
               if (subobject_index < sublist_index)
                  cobb::lua::error(L, "cannot modify items within the locked part of the list");
               sublist_index -= size;
            }
            return { pair.second, sublist_index, subobject_index };
         } else {
            size_t subobject_index = 0;
            if (pair.first)
               subobject_index += pair.first->size();
            if (pair.second)
               subobject_index += pair.second->size();
            return { pair.second, subobject_index, subobject_index };
         }
      }
   }
}