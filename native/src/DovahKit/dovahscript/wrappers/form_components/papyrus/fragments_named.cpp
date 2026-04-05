#include "./fragments_named.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/papyrus/attachment_data.h"
#include "dovah/forms/components/papyrus/fragment_data/topic_info_fragment_data.h"
#include "dovah/forms/Form.h"

#include "../papyrus.h"
#include "./fragment.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::papyrus_fragments_named;

   using root_wrapper = wrappers::papyrus_root;

   using topic_info_fragment_data = dovah::loaded_forms::components::papyrus::topic_info_fragment_data;
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
static bool has_overall_script_name(dovah::form_stub& stub) {
   switch (stub.form_type) {
      case dovah::form_type::topic_info:
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
      int script_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return 0;
         if (!has_overall_script_name(form->stub)) {
            lua_pushnil(L);
            return 1;
         }
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            return 0;

         if (auto* base = root->fragment_data) {
            if (auto* frag = dynamic_cast<topic_info_fragment_data*>(base)) {
               lua_pushstring(L, frag->filename.c_str());
               return 1;
            }
         }
         lua_pushstring(L, "");
         return 1;
      }
   }
   namespace _setters {
      int script_name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            cobb::lua::error(L, "papyrus_fragment wrapper has no underlying object (deleted?)");
         if (!has_overall_script_name(form->stub))
            cobb::lua::error(L, "only `topic_info` fragment data specifies an overall script name; for all other form types with fragment data, only individual fragments specify script names");

         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         std::string_view v = lua_tostring(L, 2);

         self.before_edit();
         {
            topic_info_fragment_data* frag = nullptr;
            if (auto* base = root->fragment_data) {
               frag = dynamic_cast<topic_info_fragment_data*>(base);
               assert(!!frag);
            } else {
               root->fragment_data = frag = new topic_info_fragment_data;
            }
            frag->filename = v;
         }
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "on_begin",    &_getters::on_begin },
      { "on_change",   &_getters::on_change },
      { "on_end",      &_getters::on_end },
      { "script_name", &_getters::script_name },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "script_name", &_setters::script_name },
   };
}