#include "./collection_grasses.h"
#include <string_view>
#include "dovahscript/wrapper.h"

#include "dovah/forms/LandTexture.h"
#include "../land_texture.h"

namespace {
   constexpr const std::string_view collection_metatable_key = "collection<dovah.classes.land_texture.grasses>";
}

#include "dovahscript/api_helpers/native_lists/member_function_spec.h"
#include "dovahscript/api_helpers/native_lists/common/pull_collection.h"
#include "dovahscript/api_helpers/native_lists/common/pull_value_as_form_of_type.h"
#include "dovahscript/api_helpers/native_lists/all_definition_params.h"

namespace {
   using namespace dovahscript;
   using containing_form_type = dovah::loaded_forms::LandTexture;
   
   struct member_function_spec : public api_helpers::native_lists::member_function_spec {
      using collection_wrapped_type = decltype(containing_form_type::grasses);
      using value_stored_type       = dovah::form_reference_t;
      using value_working_type      = dovah::form_stub*;

      static constexpr const bool allow_insertions_past_end = true;

      static constexpr const auto pull_collection = &api_helpers::native_lists::common::pull_collection<collection_metatable_key>;

      static collection_wrapped_type* unwrap_collection(wrapper& self) {
         auto* form = self.get_loaded_form_data<containing_form_type>();
         if (!form)
            return nullptr;
         return &form->grasses;
      }

      static constexpr const auto pull_value = &api_helpers::native_lists::common::pull_value_as_form_of_type<dovah::form_type::grass>;
   };
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params land_texture_grasses = api_helpers::native_lists::all_definition_params<collection_metatable_key, member_function_spec>;
}