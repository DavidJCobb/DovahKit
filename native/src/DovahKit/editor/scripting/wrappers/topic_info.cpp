#include "topic_info.h"
#include "../classes.h"
#include "../util.h"
#include "../editor_script_core.h"
#include "../wrapper_util.h"
#include "../collections.h"

#include "topic_info/response.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::topic_info;
   using _loaded_form_t = dovah::loaded_forms::TopicInfo;
}

#pragma region Collection: "responses"
namespace {
   using namespace editor_script;

   namespace _collections::responses {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, _wrapper_t::response_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", _wrapper_t::response_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         lua_pushnumber(L, 3);
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         auto  i    = lua_tointeger(L, 2);
         auto& list = form->responses;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         wrapper out = self;
         assert(out.is_collection);
         assert(out.parts[0].signature == editor_script::wrapper_part_types::topic_info_response);
         out.into_collection(i);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::topic_info_response::metatable_key);
      }
   }
}
#pragma endregion

#pragma region form
namespace {
   namespace _getters {
      luastackchange_t hours_until_reset(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushnumber(L, (lua_Number)form->days_until_reset * 24.0);
         return 1;
      }
      luastackchange_t override_topic_text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->override_topic_text.c_str());
         return 1;
      }
      luastackchange_t parent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         if (!self.stub)
            return 0;
         auto* p = self.stub->get_parent_form();
         if (p->formType != dovah::form_type::topic)
            p = nullptr;
         wrapper out;
         auto* mt = wrap_form(out, p);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t parent_quest(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         if (!self.stub)
            return 0;
         dovah::form_stub* parent = self.stub->get_parent_form();
         dovah::form_stub* quest  = nullptr;
         if (parent->formType == dovah::form_type::topic) {
            quest = parent->get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_quest);
            if (quest->formType != dovah::form_type::quest)
               quest = nullptr;
         }
         wrapper out;
         auto* mt = wrap_form(out, quest);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t responses(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(editor_script::wrapper_part_types::topic_info_response);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, _wrapper_t::response_collection_key);
      }
      luastackchange_t speaker(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out;
         auto* mt = wrap_form(out, form->speaker.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t walk_away_topic(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out;
         auto* mt = wrap_form(out, form->walk_away_topic.get_form_stub());
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
   }
   namespace _setters {
      luastackchange_t hours_until_reset(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->days_until_reset = lua_tonumber(L, 2) / 24.0;
         self.after_edit();
         return 0;
      }
      luastackchange_t override_topic_text(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->override_topic_text = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t speaker(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::actor_base, true);
         if (!form)
            return 0;
         self.before_edit();
         form->speaker.set(*form, other->stub);
         self.after_edit();
         return 0;
      }
      luastackchange_t walk_away_topic(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         auto* other = wrapper_from_stack<wrappers::form>(L, 2);
         other->error_if_wrong_form_type(L, 2, dovah::form_type::topic, true);
         if (!form)
            return 0;
         self.before_edit();
         form->walk_away_topic.set(*form, other->stub);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "hours_until_reset",   &_getters::hours_until_reset },
      { "override_topic_text", &_getters::override_topic_text },
      { "parent",              &_getters::parent },
      { "parent_quest",        &_getters::parent_quest },
      { "parent_topic",        &_getters::parent },
      { "responses",           &_getters::responses }, // collection
      { "speaker",             &_getters::speaker },
      { "walk_away_topic",     &_getters::walk_away_topic },
   };
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "hours_until_reset",   &_setters::hours_until_reset },
      { "override_topic_text", &_setters::override_topic_text },
      { "speaker",             &_setters::speaker },
      { "walk_away_topic",     &_setters::walk_away_topic },
   };

   /*static*/ void _wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key          = _wrapper_t::response_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::responses::get_collection_length,
         .lookup_item_by_index   = &_collections::responses::lookup_item_by_index,
      });
   }
}
#pragma endregion