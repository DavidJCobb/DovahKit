#include "./fragment.h"
#include "helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/papyrus/attachment_data.h"
#include "dovah/forms/components/papyrus/fragment_data/fragment_data_base.h"
#include "dovah/forms/components/papyrus/fragment_data/package_fragment_data.h"
#include "dovah/forms/components/papyrus/fragment_data/perk_fragment_data.h"
#include "dovah/forms/components/papyrus/fragment_data/scene_fragment_data.h"
#include "dovah/forms/components/papyrus/fragment_data/topic_info_fragment_data.h"
#include "dovah/forms/Form.h"

#include "../papyrus.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::papyrus_fragment;
   using root_wrapper = wrappers::papyrus_root;

   using fragment_type  = dovah::loaded_forms::components::papyrus::fragment_type;
   using basic_fragment = dovah::loaded_forms::components::papyrus::basic_fragment;
   //
   using package_fragment_data    = dovah::loaded_forms::components::papyrus::package_fragment_data;
   using perk_fragment_data       = dovah::loaded_forms::components::papyrus::perk_fragment_data;
   using scene_fragment_data      = dovah::loaded_forms::components::papyrus::scene_fragment_data;
   using topic_info_fragment_data = dovah::loaded_forms::components::papyrus::topic_info_fragment_data;
}

template<bool Emplace>
static basic_fragment* unwrap_basic_fragment(wrapper& w, root_wrapper::wrapped_type& root) {
   auto* base = root.fragment_data;
   if (!base) {
      if constexpr (Emplace) {
         auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
         if (!form)
            return nullptr;
         switch (form->stub.form_type) {
            case dovah::form_type::package:
               base = root.fragment_data = new package_fragment_data;
               break;
            case dovah::form_type::scene:
               base = root.fragment_data = new scene_fragment_data;
               break;
            case dovah::form_type::topic_info:
               base = root.fragment_data = new topic_info_fragment_data;
               break;
            default:
               return nullptr;
         }
      } else {
         return nullptr;
      }
   }
   auto& part = w.last_part();

   std::optional<basic_fragment>* frag_opt = nullptr;
   switch (base->type) {
      case fragment_type::info:
         {
            auto* frag = (topic_info_fragment_data*)base;
            switch (part.signature) {
               case wrapper_part_types::papyrus_frag_begin:
                  frag_opt = &frag->fragments.on_begin;
                  break;
               case wrapper_part_types::papyrus_frag_end:
                  frag_opt = &frag->fragments.on_end;
                  break;
            }
         }
         break;
      case fragment_type::package:
         {
            auto* frag = (package_fragment_data*)base;
            switch (part.signature) {
               case wrapper_part_types::papyrus_frag_begin:
                  frag_opt = &frag->fragments.on_begin;
                  break;
               case wrapper_part_types::papyrus_frag_change:
                  frag_opt = &frag->fragments.on_change;
                  break;
               case wrapper_part_types::papyrus_frag_end:
                  frag_opt = &frag->fragments.on_end;
                  break;
            }
         }
         break;
      case fragment_type::scene:
         {
            auto* frag = (scene_fragment_data*)base;
            switch (part.signature) {
               case wrapper_part_types::papyrus_frag_begin:
                  frag_opt = &frag->fragments.on_begin;
                  break;
               case wrapper_part_types::papyrus_frag_end:
                  frag_opt = &frag->fragments.on_end;
                  break;
            }
         }
         break;
   }
   if (!frag_opt)
      return nullptr;
   if (frag_opt->has_value())
      return &frag_opt->value();
   if constexpr (Emplace) {
      return &frag_opt->emplace();
   }
   return nullptr;
}
static perk_fragment_data::fragment* unwrap_perk_fragment(wrapper& w, root_wrapper::wrapped_type& root) {
   auto* base = root.fragment_data;
   if (!base)
      return nullptr;
   if (base->type != fragment_type::perk)
      return nullptr;
   auto* frag = (perk_fragment_data*)base;

   auto i = w.last_part().index;
   if (i >= frag->fragments.size())
      return nullptr;
   return &frag->fragments[i];
}

namespace {
   namespace _getters {
      int function_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            return 0;

         if (auto* frag = unwrap_perk_fragment(self, *root)) {
            lua_pushstring(L, frag->function.c_str());
            return 1;
         }
         if (auto* frag = unwrap_basic_fragment<false>(self, *root)) {
            lua_pushstring(L, frag->function.c_str());
            return 1;
         }
         return 0;
      }
      int script_name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            return 0;

         if (auto* frag = unwrap_perk_fragment(self, *root)) {
            lua_pushstring(L, frag->filename.c_str());
            return 1;
         }
         if (auto* frag = unwrap_basic_fragment<false>(self, *root)) {
            lua_pushstring(L, frag->script.c_str());
            return 1;
         }
         return 0;
      }
   }
   namespace _setters {
      int function_name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            cobb::lua::error(L, "papyrus_fragment wrapper has no underlying object (deleted?)");

         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         std::string_view v = lua_tostring(L, 2);

         self.before_edit();
         if (auto* frag = unwrap_perk_fragment(self, *root)) {
            frag->function = v;
         } else if (auto* frag = unwrap_basic_fragment<true>(self, *root)) {
            frag->function = v;
         }
         self.after_edit();
         return 0;
      }
      int script_name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* root = root_wrapper::unwrap(self);
         if (!root)
            cobb::lua::error(L, "papyrus_fragment wrapper has no underlying object (deleted?)");

         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         std::string_view v = lua_tostring(L, 2);

         self.before_edit();
         if (auto* frag = unwrap_perk_fragment(self, *root)) {
            frag->filename = v;
         } else if (auto* frag = unwrap_basic_fragment<true>(self, *root)) {
            frag->script = v;
         }
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "function_name", &_getters::function_name },
      { "script_name",   &_getters::script_name },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "function_name", &_setters::function_name },
      { "script_name",   &_setters::script_name },
   };
}