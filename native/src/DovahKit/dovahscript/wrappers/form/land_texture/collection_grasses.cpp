#include "./collection_grasses.h"
#include "dovah/forms/LandTexture.h"
#include "dovahscript/api_helpers/native_lists/member_function_spec.h"
#include "dovahscript/api_helpers/native_lists/common/pull_collection.h"
#include "dovahscript/api_helpers/native_lists/common/pull_value_as_form_of_type.h"
#include "dovahscript/api_helpers/native_lists/define_metatable.h"
#include "dovahscript/wrapper.h"

namespace {
   using namespace dovahscript;
   using containing_form_type = dovah::loaded_forms::LandTexture;
   using wrapper_spec         = dovahscript::wrapper_likes::native_lists::land_texture_grasses;
   
   struct member_function_spec : public api_helpers::native_lists::member_function_spec {
      using collection_wrapped_type = decltype(containing_form_type::grasses);
      using value_stored_type       = dovah::form_reference_t;
      using value_working_type      = dovah::form_stub*;

      static constexpr const bool allow_insertions_past_end = true;

      static constexpr const auto pull_collection = &api_helpers::native_lists::common::pull_collection<wrapper_spec::metatable_key>;

      static collection_wrapped_type* unwrap_collection(wrapper& self) {
         auto* form = self.get_loaded_form_data<containing_form_type>();
         if (!form)
            return nullptr;
         return &form->grasses;
      }

      static constexpr const auto pull_value = &api_helpers::native_lists::common::pull_value_as_form_of_type<dovah::form_type::grass>;
   };
}

namespace dovahscript::wrapper_likes::native_lists {
   /*static*/ void wrapper_spec::define_metatable(lua_State* L) {
      api_helpers::native_lists::define_metatable<metatable_key, class_name, member_function_spec>(L);
   }
   /*static*/ int wrapper_spec::push(lua_State* L, const wrapper& parent) {
      wrapper out = parent;
      out.append_part(signature);
      out.is_collection = true;
      return core::subsystems::userdata::get().push(L, out, metatable_key.data());
   }
}