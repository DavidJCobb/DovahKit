#pragma once
#include "./is_validate_and_overwrite.h"

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept is_pull_and_store = requires {
      requires !is_validate_and_overwrite<Spec>;
   };
}