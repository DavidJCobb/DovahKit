#include "./fragments_named.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/papyrus/attachment_data.h"
#include "dovah/forms/Form.h"

#include "../papyrus.h"
#include "./fragment.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::papyrus_fragments_named;

   using root_wrapper = wrappers::papyrus_root;
}

static bool has_begin_and_end_fragment(dovah::form_stub& stub) {
   switch (stub.form_type) {
      case dovah::form_type::package:
      case dovah::form_type::scene:
      case dovah::form_type::topic_info:
         return true;
   }
   return false;
}
static bool has_change_fragment(dovah::form_stub& stub) {
   switch (stub.form_type) {
      case dovah::form_type::package:
         return true;
   }
   return false;
}

namespace {
   namespace _getters {
      int on_begin(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return 0;
         if (!has_begin_and_end_fragment(form->stub)) {
            lua_pushnil(L);
            return 1;
         }
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_frag_begin);
         return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_fragment::metatable_key);
      }
      int on_change(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return 0;
         if (!has_change_fragment(form->stub)) {
            lua_pushnil(L);
            return 1;
         }
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_frag_change);
         return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_fragment::metatable_key);
      }
      int on_end(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return 0;
         if (!has_begin_and_end_fragment(form->stub)) {
            lua_pushnil(L);
            return 1;
         }
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_frag_end);
         return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_fragment::metatable_key);
      }
   }
   namespace _setters {
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "on_begin",  &_getters::on_begin },
      { "on_change", &_getters::on_change },
      { "on_end",    &_getters::on_end },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;
}