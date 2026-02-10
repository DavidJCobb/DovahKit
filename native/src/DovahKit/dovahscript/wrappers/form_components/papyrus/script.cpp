#include "./script.h"
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
#include "./collection_properties.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::papyrus_script;
   using wrapped_type = cls::wrapped_type;

   using root_wrapper = wrappers::papyrus_root;
}

wrapped_type* cls::unwrap(wrapper& w) {
   auto* root = wrappers::papyrus_root::unwrap(w);
   if (!root)
      return nullptr;

   size_t depth = w.parts.size();
   for (size_t i = 0; i < w.parts.size(); ++i) {
      if (w.parts[i].signature == wrapper_part_types::papyrus_script) {
         depth = i;
         break;
      }
   }
   if (depth >= w.parts.size())
      return nullptr;

   auto& list = root->scripts;
   auto  i    = w.parts[depth].index;
   if (i >= list.size())
      return nullptr;
   return &list[i];
}

namespace {
   namespace _getters {
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            return 0;
         lua_pushstring(L, item->name.c_str());
         return 1;
      }
      int properties(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_property);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::papyrus_properties.registry_key);
      }
      int status(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            return 0;
         using enumeration = decltype(wrapped_type::status);
         switch (item->status) {
            case enumeration::defined_locally:
               lua_pushstring(L, "local");
               break;
            case enumeration::overrides_base:
               lua_pushstring(L, "override");
               break;
            case enumeration::defined_on_base:
               lua_pushstring(L, "inherited");
               break;
            case enumeration::removed:
               lua_pushstring(L, "removed");
               break;
            default:
               lua_pushnil(L);
               break;
         }
         return 1;
      }
   }
   namespace _setters {
      int name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            cobb::lua::error(L, "papyrus_script wrapper has no underlying object (deleted?)");
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         std::string_view name = lua_tostring(L, 2);

         if (dovah::papyrus::helpers::name_equals(name, item->name)) {
            //
            // Skip uniqueness checks and just apply the name (in case there are letter case changes).
            //
            self.before_edit();
            item->name = name;
            self.after_edit();
            return 0;
         }

         // Uniqueness check.
         auto* root = root_wrapper::unwrap(self);
         assert(root != nullptr);
         if (root->lookup_script(name)) {
            cobb::lua::argerror(L, 2, "another script with the requested name is already attached");
         }

         self.before_edit();
         item->name = name;
         self.after_edit();
         return 0;
      }
      int status(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            cobb::lua::error(L, "papyrus_script wrapper has no underlying object (deleted?)");

         using enumeration = decltype(wrapped_type::status);
         enumeration value;
         {
            cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
            std::string_view v = lua_tostring(L, 2);
            if (v == "local") {
               value = enumeration::defined_locally;
            } else if (v == "override") {
               value = enumeration::overrides_base;
            } else if (v == "inherited") {
               value = enumeration::defined_on_base;
            } else if (v == "removed") {
               value = enumeration::removed;
            } else {
               cobb::lua::argerror(L, 2, "unrecognized value");
            }
         }

         assert(self.stub);
         if (!dovah::form_type_is_reference(self.stub->form_type)) {
            switch (value) {
               case enumeration::overrides_base:
               case enumeration::defined_on_base:
               case enumeration::removed:
                  cobb::lua::argerror(L, 2, "the only valid status for scripts attached to non-refs is \"local\"");
                  break;
            }
         }

         self.before_edit();
         item->status = value;
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "name",       &_getters::name },
      { "properties", &_getters::properties },
      { "status",     &_getters::status },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "name",   &_setters::name },
      { "status", &_setters::status },
   };
}