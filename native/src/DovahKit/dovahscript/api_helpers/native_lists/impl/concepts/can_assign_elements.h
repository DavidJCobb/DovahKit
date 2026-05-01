#pragma once
#include "../fields/pull_value.h"
#include "../fields/validate_value.h"

namespace dovahscript::api_helpers::native_lists::impl::concepts {
   template<typename Spec>
   concept can_assign_elements = Spec::allow_mutation && (fields::pull_value::present<Spec> || fields::validate_value::valid<Spec>);
}