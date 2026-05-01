#pragma once
#include <concepts>
#include <optional>
#include <type_traits>
#include <utility> // std::pair
#include "helpers/type_traits/is_std_vector.h"
#include "lua.h"
namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}
namespace dovahscript {
   class wrapper;
}

#include "../fields/get_collection_length.h"
#include "../fields/get_item_by_index.h"
#include "../fields/initialize_value.h"
#include "../fields/overwrite_value.h"
#include "../fields/prepare_for_insertion.h"
#include "../fields/pull_value.h"
#include "../fields/store_value.h"
#include "../fields/validate_value.h"
#include "../preparation_types.h"
#include "./is_a_spec.h"
#include "./storage_is_bifurcated.h"

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   namespace prepare_for_insertion {
      template<typename Spec>
      concept valid = requires {
         requires fields::prepare_for_insertion::present<Spec>;
         requires requires(const typename Spec::collection_wrapped_type& list, size_t count_to_insert, lua_State* L, std::optional<int> stack_pos) {
            { Spec::prepare_for_insertion(list, count_to_insert, L, stack_pos) } -> cobb::is_std_vector;
         };
         typename preparation_value_type_t<Spec>;
         requires requires(typename Spec::value_stored_type& item, preparation_value_type_t<Spec> v) {
            { Spec::apply_preparation(item, v) };
         };
      };
   }
   namespace unwrap_collection {
      template<typename Spec>
      concept present = requires {
         { Spec::unwrap_collection };
      };

      template<typename Spec>
      concept is_single = requires(wrapper & self, size_t i) {
         requires present<Spec>;
         typename Spec::collection_wrapped_type;
         { Spec::unwrap_collection(self) } -> std::same_as<typename Spec::collection_wrapped_type*>;
      };

      template<typename Spec>
      concept valid = storage_is_bifurcated<Spec> || is_single<Spec>;
   }

   template<typename Spec>
   concept is_fully_valid_spec = requires {
      requires is_a_spec<Spec>;
      requires (fields::get_collection_length::valid<Spec> || fields::get_collection_length::defaultable<Spec>);
      requires (fields::get_item_by_index::valid<Spec> || fields::get_item_by_index::defaultable<Spec>);
      requires (!fields::initialize_value::present<Spec> || fields::initialize_value::valid<Spec>);
      requires (!fields::overwrite_value::present<Spec> || fields::overwrite_value::valid<Spec>);
      requires (!fields::prepare_for_insertion::present<Spec> || prepare_for_insertion::valid<Spec>);
      requires (!fields::pull_value::present<Spec> || fields::pull_value::valid<Spec>);
      requires (!fields::store_value::present<Spec> || fields::store_value::valid<Spec>);
      requires unwrap_collection::valid<Spec>;

      // If the spec doesn't allow mutation at all, then it should not claim to allow 
      // removals.
      requires (Spec::allow_mutation || !Spec::allow_removals);
      
      // If the list type is const, then the spec must not allow removals, and must not 
      // supply member functions for modifying the list.
      requires (
         !std::is_const_v<typename Spec::collection_wrapped_type> || (
            !Spec::allow_removals &&
            !fields::overwrite_value::present<Spec> &&
            !fields::store_value::present<Spec> &&
            !fields::validate_value::valid<Spec>
         )
      );

      // Don't use both `pull_value` and `validate_value`/`overwrite_value`.
      requires (!fields::pull_value::present<Spec> || !fields::validate_value::valid<Spec>);
         
      // If you specify `validate_value`, you must also specify `overwrite_value`, 
      // and vice versa.
      requires (fields::validate_value::valid<Spec> == fields::overwrite_value::present<Spec>);
   };
}