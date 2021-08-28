#include "topic_info.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../pull_native_object.h"
#include "../../push_native_object.h"
#include "../../wrapper.h"

#include "../../../dovah/forms/TopicInfo.h"
#include "topic_info/collection_responses.h"
#include "topic_info/response.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::topic_info;
   using wrapped_type = dovah::loaded_forms::TopicInfo;

   namespace _getters {
      int hours_until_reset(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, (lua_Number)form->days_until_reset * 24.0);
         return 1;
      }
      int override_topic_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->override_topic_text.c_str());
         return 1;
      }
      int parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* p = self.stub->get_parent_form();
         if (p->formType != dovah::form_type::topic)
            p = nullptr;
         return push_native_object(p);
      }
      int parent_quest(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         dovah::form_stub* parent = self.stub->get_parent_form();
         dovah::form_stub* quest  = nullptr;
         if (parent->formType == dovah::form_type::topic) {
            quest = parent->get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_quest);
            if (quest->formType != dovah::form_type::quest)
               quest = nullptr;
         }
         return push_native_object(quest);
      }
      int responses(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::topic_info_response);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::topic_info_responses.registry_key);
      }
      int speaker(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->speaker);
      }
      int walk_away_topic(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->walk_away_topic);
      }
   }
   namespace _setters {
      int hours_until_reset(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->days_until_reset = lua_tonumber(L, 2) / 24.0;
         self.after_edit();
         return 0;
      }
      int override_topic_text(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->override_topic_text = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int speaker(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::actor_base);
         if (!form)
            return 0;
         self.before_edit();
         form->speaker.set(*form, value);
         self.after_edit();
         return 0;
      }
      int walk_away_topic(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::topic);
         if (!form)
            return 0;
         self.before_edit();
         form->walk_away_topic.set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "hours_until_reset",   &_getters::hours_until_reset },
      { "override_topic_text", &_getters::override_topic_text },
      { "parent",              &_getters::parent },
      { "parent_quest",        &_getters::parent_quest },
      { "parent_topic",        &_getters::parent },
      { "responses",           &_getters::responses }, // collection
      { "speaker",             &_getters::speaker },
      { "walk_away_topic",     &_getters::walk_away_topic },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "hours_until_reset",   &_setters::hours_until_reset },
      { "override_topic_text", &_setters::override_topic_text },
      { "speaker",             &_setters::speaker },
      { "walk_away_topic",     &_setters::walk_away_topic },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::topic_info_responses);
   }
}