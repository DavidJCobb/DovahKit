#include "topic_info.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../pull_native_object.h"
#include "../../push_native_object.h"
#include "../../wrapper.h"

#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/use_info/entry_flags/topic.h"
#include "dovah/forms/TopicInfo.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "./topic_info/collection_link_to.h"
#include "./topic_info/collection_responses.h"
#include "./topic_info/response.h"
#include "../form_components/collection_conditions.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::topic_info;
   using wrapped_type = dovah::loaded_forms::TopicInfo;

   namespace _getters {
      template<wrapped_type::info_flag::type Flag>
      int _info_flag(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, !!(form->info_flags & Flag));
         return 1;
      }
      
      int conditions(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::condition_list);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::condition_list.registry_key);
      }
      int favor_level(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         switch (form->favor_level) {
            case wrapped_type::favor_level_t::none:
               lua_pushstring(L, "none");
               break;
            case wrapped_type::favor_level_t::small:
               lua_pushstring(L, "small");
               break;
            case wrapped_type::favor_level_t::medium:
               lua_pushstring(L, "medium");
               break;
            case wrapped_type::favor_level_t::large:
               lua_pushstring(L, "large");
               break;
            default:
               lua_pushnumber(L, (lua_Number)form->favor_level);
               break;
         }
         return 1;
      }
      int has_lip_file(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->info_flags & wrapped_type::info_flag::no_lip_file) == 0);
         return 1;
      }
      int hours_until_reset(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, (lua_Number)form->get_hours_until_reset());
         return 1;
      }
      int link_to(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::topic_info_link_to);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::topic_info_link_to.registry_key);
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
         if (p->form_type != dovah::form_type::topic)
            p = nullptr;
         return push_native_object(p);
      }
      int parent_quest(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         dovah::form_stub* parent = self.stub->get_parent_form();
         dovah::form_stub* quest  = nullptr;
         if (parent && parent->form_type == dovah::form_type::topic) {
            quest = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::topic::parent_quest>(*parent);
            if (quest->form_type != dovah::form_type::quest)
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
      int use_shared_info(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return push_native_object(form->use_shared_info);
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
      template<wrapped_type::info_flag::type Flag>
      int _info_flag(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "expected boolean");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->info_flags, Flag, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }

      int favor_level(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2) || lua_isinteger(L, 2), 2, "expected string or integer");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         if (lua_isinteger(L, 2)) {
            auto i = lua_tointeger(L, 2);
            luaL_argcheck(L, i >= 0 && i <= 255, 2, "raw integer values must be in the range [0, 255]");
            self.before_edit();
            form->favor_level = (wrapped_type::favor_level_t)i;
            self.after_edit();
            return 0;
         }
         std::string_view value = lua_tostring(L, 2);
         wrapped_type::favor_level_t level;
         if (value == "none") {
            level = wrapped_type::favor_level_t::none;
         } else if (value == "small") {
            level = wrapped_type::favor_level_t::small;
         } else if (value == "medium") {
            level = wrapped_type::favor_level_t::medium;
         } else if (value == "large") {
            level = wrapped_type::favor_level_t::large;
         } else {
            luaL_argcheck(L, false, 2, "unrecognized value");
         }
         self.before_edit();
         form->favor_level = level;
         self.after_edit();
         return 0;
      }
      int has_lip_file(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "expected boolean");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->info_flags, wrapped_type::info_flag::no_lip_file, lua_toboolean(L, 2) == 0);
         self.after_edit();
         return 0;
      }
      int hours_until_reset(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto  value = lua_tonumber(L, 2);
         luaL_argcheck(L, value >=  0, 2, "you cannot set a negative number of hours");
         luaL_argcheck(L, value <= 24, 2, "the maximum hours until reset is 24");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->set_hours_until_reset(lua_tonumber(L, 2));
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
      int use_shared_info(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::topic_info);
         if (!form)
            return 0;
         if (value) {
            bool is_shared_info = [value]() {
               auto* topic = value->get_parent_form();
               if (!topic || topic->form_type != dovah::form_type::topic)
                  return false;
               auto& fic = dovahkit::subsystems::form_info_cache::core::get();
               return fic.topic_is_sharedinfo_topic(*topic);
            }();
            luaL_argcheck(L, is_shared_info, 2, "the provided info is not a SharedInfo (i.e. it is not in a SharedInfo-subtype topic)");
         }
         self.before_edit();
         form->use_shared_info.set(*form, value);
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
      #pragma region Info flags
         { "can_move_while_greeting",      &_getters::_info_flag<wrapped_type::info_flag::can_move_while_greeting> },
         { "enable_audio_output_override", &_getters::_info_flag<wrapped_type::info_flag::audio_output_override> },
         { "enable_walk_away_topic",       &_getters::_info_flag<wrapped_type::info_flag::walk_away> },
         { "force_subtitle",               &_getters::_info_flag<wrapped_type::info_flag::force_subtitle> },
         { "hide_walk_away_topic",         &_getters::_info_flag<wrapped_type::info_flag::walk_away_invisible_in_menu> },
         { "invisible_continue",           &_getters::_info_flag<wrapped_type::info_flag::invisible_continue> },
         { "is_goodbye",                   &_getters::_info_flag<wrapped_type::info_flag::goodbye> },
         { "is_random",                    &_getters::_info_flag<wrapped_type::info_flag::random> },
         { "is_random_end",                &_getters::_info_flag<wrapped_type::info_flag::random_end> },
         { "requires_post_processing",     &_getters::_info_flag<wrapped_type::info_flag::requires_post_processing> },
         { "say_once",                     &_getters::_info_flag<wrapped_type::info_flag::say_once> },
         { "spends_favor_points",          &_getters::_info_flag<wrapped_type::info_flag::spends_favor_points> },
      #pragma endregion
      { "conditions",          &_getters::conditions },
      { "favor_level",         &_getters::favor_level },
      { "has_lip_file",        &_getters::has_lip_file },
      { "hours_until_reset",   &_getters::hours_until_reset },
      { "link_to",             &_getters::link_to }, // collection
      { "override_topic_text", &_getters::override_topic_text },
      { "parent",              &_getters::parent },
      { "parent_quest",        &_getters::parent_quest },
      { "parent_topic",        &_getters::parent },
      { "responses",           &_getters::responses }, // collection
      { "speaker",             &_getters::speaker },
      { "use_shared_info",     &_getters::use_shared_info },
      { "walk_away_topic",     &_getters::walk_away_topic },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      #pragma region Info flags
         { "can_move_while_greeting",      &_setters::_info_flag<wrapped_type::info_flag::can_move_while_greeting> },
         { "enable_audio_output_override", &_setters::_info_flag<wrapped_type::info_flag::audio_output_override> },
         { "enable_walk_away_topic",       &_setters::_info_flag<wrapped_type::info_flag::walk_away> },
         { "force_subtitle",               &_setters::_info_flag<wrapped_type::info_flag::force_subtitle> },
         { "hide_walk_away_topic",         &_setters::_info_flag<wrapped_type::info_flag::walk_away_invisible_in_menu> },
         { "invisible_continue",           &_setters::_info_flag<wrapped_type::info_flag::invisible_continue> },
         { "is_goodbye",                   &_setters::_info_flag<wrapped_type::info_flag::goodbye> },
         { "is_random",                    &_setters::_info_flag<wrapped_type::info_flag::random> },
         { "is_random_end",                &_setters::_info_flag<wrapped_type::info_flag::random_end> },
         { "requires_post_processing",     &_setters::_info_flag<wrapped_type::info_flag::requires_post_processing> },
         { "say_once",                     &_setters::_info_flag<wrapped_type::info_flag::say_once> },
         { "spends_favor_points",          &_setters::_info_flag<wrapped_type::info_flag::spends_favor_points> },
      #pragma endregion
      { "favor_level",         &_setters::favor_level },
      { "has_lip_file",        &_setters::has_lip_file },
      { "hours_until_reset",   &_setters::hours_until_reset },
      { "override_topic_text", &_setters::override_topic_text },
      { "speaker",             &_setters::speaker },
      { "use_shared_info",     &_setters::use_shared_info },
      { "walk_away_topic",     &_setters::walk_away_topic },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::topic_info_link_to);
      define_collection_metatable(L, collections::topic_info_responses);
   }
}