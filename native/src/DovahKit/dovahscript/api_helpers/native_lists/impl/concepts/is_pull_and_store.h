#pragma once
#include "./is_validate_and_overwrite.h"
#include "../fields/pull_value.h"

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept is_pull_and_store = requires {
      requires !is_validate_and_overwrite<Spec>;
      requires fields::pull_value::present<Spec>;
   };
}