#pragma once
#include <concepts>
#include <type_traits>
namespace dovahscript::api_helpers::native_lists {
   struct member_function_spec;
}
namespace dovahscript {
   class wrapper;
}

#include "./pushes_value_as_subobject_wrapper.h"
#include "./pushes_value_directly.h"
#include "./values_are_subobjects.h"
#include "../push_value.h"

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept is_a_spec = requires {
      requires std::is_base_of_v<member_function_spec, Spec>;
      requires !std::is_same_v<typename Spec::collection_wrapped_type, void>;
      requires !std::is_same_v<typename Spec::value_stored_type,       void>;
      requires !std::is_same_v<typename Spec::value_working_type,      void>;

      requires requires(lua_State* L) {
         { Spec::pull_collection(L) } -> std::same_as<wrapper&>;
      };
      requires (
         impl::concepts::values_are_subobjects<Spec> ?
            concepts::pushes_value_as_subobject_wrapper<Spec>
         :
            (concepts::pushes_value_directly<Spec> || value_type_is_default_pushable<Spec>)
      );
   };
}