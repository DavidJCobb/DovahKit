#include "./papyrus.h"
#include "helpers/lua/error.h"
#include "helpers/string/strieq_ascii.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrap_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/papyrus/attachment_data.h"
#include "dovah/forms/_component_access.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/Quest.h"
#include "./papyrus/collection_fragments_indexed.h"
#include "./papyrus/collection_scripts.h"
#include "./papyrus/fragments_named.h"
#include "../form/quest/alias.h"
#include "../form/quest.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::papyrus_root;
   using wrapped_type = cls::wrapped_type;
}

int cls::wrap_and_push(lua_State* L, dovah::form_stub& stub) {
   wrapper out = wrap_native_object(stub);
   out.append_part(wrapper_part_types::papyrus_root);
   return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_root::metatable_key);
}
int cls::wrap_and_push(lua_State* L, dovah::loaded_forms::Alias& alias) {
   wrapper out = wrap_native_object(alias.owner.stub);
   out.append_part(dovahscript::wrapper_part_types::quest_alias_by_id);
   out.is_collection = true;
   out.into_collection(alias.id);
   out.last_part().noncontiguous = true;
   out.append_part(wrapper_part_types::papyrus_root);
   return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_root::metatable_key);
}
wrapped_type* cls::unwrap(wrapper& w) {
   if (!w.stub)
      return nullptr;
   auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
   if (!form)
      return nullptr;

   if (w.parts[0].signature == wrapper_part_types::quest_alias || w.parts[0].signature == wrapper_part_types::quest_alias_by_id) {
      auto* quest = w.get_loaded_form_data<dovah::loaded_forms::Quest>();
      if (!quest)
         return nullptr;
      dovah::loaded_forms::Alias* alias = nullptr;

      auto i = w.parts[0].index;
      if (w.parts[0].signature == wrapper_part_types::quest_alias) {
         if (i < quest->aliases.size())
            alias = quest->aliases[i];
      } else {
         alias = quest->lookup_alias_by_id(i);
      }
      if (!alias)
         return nullptr;
      return &alias->script_data;
   }

   return dovah::loaded_forms::component_access::get_papyrus_data(form);
}

namespace {
   namespace _getters {
      int fragments(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* form    = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return 0;
         auto* wrapped = cls::unwrap(self);
         if (!wrapped)
            return 0;

         switch (form->stub.form_type) {
            case dovah::form_type::package:
            case dovah::form_type::scene:
            case dovah::form_type::topic_info:
               {
                  wrapper out = self;
                  out.append_part(wrapper_part_types::papyrus_frags_named);
                  return core::subsystems::userdata::get().push(L, out, wrappers::papyrus_fragments_named::metatable_key);
               }
               break;
            case dovah::form_type::perk:
               {
                  wrapper out = self;
                  out.append_part(wrapper_part_types::papyrus_frag_indexed);
                  out.is_collection = true;
                  return core::subsystems::userdata::get().push(L, out, wrappers::collections::papyrus_fragments_indexed.registry_key);
               }
               break;
         }

         return 0;
      }
      int scripts(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_script);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::papyrus_scripts.registry_key);
      }
      int version(lua_State* L) {
         auto& self    = get_wrapper_for_thiscall<cls>(L);
         auto* wrapped = cls::unwrap(self);
         if (wrapped == nullptr)
            cobb::lua::error(L, "papyrus root wrapper has no underlying object (deleted?)");
         lua_pushinteger(L, wrapped->header.version);
         return 1;
      }
   }
   namespace _setters {
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "fragments", &_getters::fragments },
      { "scripts",   &_getters::scripts },
      { "version",   &_getters::version },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;
}