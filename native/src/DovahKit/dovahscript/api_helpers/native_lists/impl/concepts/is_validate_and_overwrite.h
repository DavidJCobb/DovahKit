#pragma once
#include "../fields/overwrite_value.h"
#include "../fields/validate_value.h"

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept is_validate_and_overwrite = requires {
      requires fields::overwrite_value::valid<Spec>;
      requires fields::validate_value::valid<Spec>;
   };
}