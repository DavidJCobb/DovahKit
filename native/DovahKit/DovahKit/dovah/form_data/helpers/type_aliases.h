#pragma once
#include "../params.h"
#include "dovah/types/localized_string_type.h"
namespace dovah {
   class form_stub;
   class managed_form_use;

   template<auto UseInfoEntryFlag>
   class unique_managed_form_use;

   template<auto LStringType>
   class managed_localized_string_use;

   template<auto LStringType>
   class unmanaged_localized_string_use;
}

namespace dovah::form_data_helpers {
   namespace type_alias_groups {
      struct managed {
         using form_use = managed_form_use;

         template<auto UseInfoEntryFlag>
         using unique_form_use = unique_managed_form_use<UseInfoEntryFlag>;

         using localized_general_string_use     = managed_localized_string_use<localized_string_type::general>;
         using localized_description_string_use = managed_localized_string_use<localized_string_type::description>;
         using localized_info_string_use        = managed_localized_string_use<localized_string_type::info>;
      };
      struct unmanaged {
         using form_use = form_stub*;

         template<auto UseInfoEntryFlag>
         using unique_form_use = form_use;

         using localized_general_string_use     = unmanaged_localized_string_use<localized_string_type::general>;
         using localized_description_string_use = unmanaged_localized_string_use<localized_string_type::description>;
         using localized_info_string_use        = unmanaged_localized_string_use<localized_string_type::info>;
      };
   }
   template<form_data_params Params>
   using type_aliases = std::conditional_t<
      (Params::management_mode == use_info::management_mode::managed),
      type_alias_groups::managed,
      type_alias_groups::unmanaged
   >;
}