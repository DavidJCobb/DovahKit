#pragma once
#include "helpers/function_traits.h"
#include "../utils/get_child_list.h"

namespace IdleAnimationFormsModel_impl::concepts {
   template<typename ParentType, typename ChildType>
   constexpr const bool can_hold_nodes_of_type = std::is_lvalue_reference_v<cobb::return_type_of<&utils::get_child_list<ChildType, ParentType>>>;
}