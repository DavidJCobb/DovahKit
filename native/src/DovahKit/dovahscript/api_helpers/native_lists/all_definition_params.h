#pragma once
#include <string_view>
#include "dovahscript/core/collections.h"
#include "dovahscript/wrapper.h"
#include "./member_function_spec.h"
#include "./get_collection_length.h"
#include "./get_item_by_index.h"
#include "./insert.h"
#include "./remove_item_at_index.h"
#include "./set_item_at_index.h"

namespace dovahscript::api_helpers::native_lists {
   template<const std::string_view& CollectionMetatableKey, typename Spec>
      requires impl::is_fully_valid_spec<Spec>
   constexpr const collection_definition_params all_definition_params = []() {
      auto out = collection_definition_params{
         .registry_key       = CollectionMetatableKey.data(),
         .garbage_collection = &wrapper::__gc,
      };
      if constexpr (impl::get_collection_length::valid<Spec>) {
         out.get_collection_length = &Spec::get_collection_length;
      } else {
         static_assert(impl::get_collection_length::defaultable<Spec>);
         out.get_collection_length = &get_collection_length<Spec>;
      }
      if constexpr (impl::get_item_by_index::valid<Spec>) {
         out.lookup_item_by_index = &Spec::get_item_by_index;
      } else {
         static_assert(impl::get_item_by_index::defaultable<Spec>);
         out.lookup_item_by_index = &get_item_by_index<Spec>;
      }
      if constexpr (impl::pull_value::valid<Spec>) {
         out.member_function_insert = &insert<Spec>;
         out.set_item               = &set_item_at_index<Spec>;
      }
      if constexpr (Spec::allow_removals) {
         out.member_function_remove = &api_helpers::native_lists::remove_item_at_index<Spec>;
      }
      return out;
   }();
}