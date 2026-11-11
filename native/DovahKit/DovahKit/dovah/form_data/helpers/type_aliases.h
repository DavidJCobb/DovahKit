#pragma once
#include <vector>
#include "../params.h"
#include "./is_or_contains_managed_use.h"
#include "dovah/localization/category.h"
#include "dovah/types/managed_vector.h"
namespace dovah {
   class form_stub;
   class managed_form_use;

   template<auto UseInfoEntryFlag>
   class unique_managed_form_use;

   template<localization::category C>
   class managed_localized_string_use;

   template<auto LStringType>
   class unmanaged_localized_string_use;
}

namespace dovah::form_data::helpers {
   namespace type_alias_groups {
      struct managed {
         using form_use = managed_form_use;

         template<auto UseInfoEntryFlag>
         using unique_form_use = unique_managed_form_use<UseInfoEntryFlag>;

         using localized_general_string_use     = managed_localized_string_use<localization::category::general>;
         using localized_description_string_use = managed_localized_string_use<localization::category::description>;
         using localized_info_string_use        = managed_localized_string_use<localization::category::info>;

         template<typename T>
         using vector = std::conditional_t<
            is_or_contains_managed_use<T>,
            managed_vector<T>,
            std::vector<T>
         >;
      };
      struct unmanaged {
         using form_use = form_stub*;

         template<auto UseInfoEntryFlag>
         using unique_form_use = form_use;

         using localized_general_string_use     = unmanaged_localized_string_use<localization::category::general>;
         using localized_description_string_use = unmanaged_localized_string_use<localization::category::description>;
         using localized_info_string_use        = unmanaged_localized_string_use<localization::category::info>;

         template<typename T>
         using vector = std::vector<T>;
      };
   }
   template<form_data_params Params>
   using type_aliases = std::conditional_t<
      (Params::management_mode == form_use_info::management_mode::managed),
      type_alias_groups::managed,
      type_alias_groups::unmanaged
   >;
}