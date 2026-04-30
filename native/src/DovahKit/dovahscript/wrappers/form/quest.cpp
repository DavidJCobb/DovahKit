#include "quest.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"

#include "../../wrapper.h"
#include "../../core/classes.h"
#include "../../core/collections.h"
#include "../../push_native_object.h"
#include "../../lua_libraries/form_types.h"

#include "../../../dovah/forms/Form.h"
#include "../../../dovah/forms/Quest.h"
#include "quest/alias.h"
#include "quest/collection_aliases.h"
#include "quest/collection_aliases_by_id.h"

#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for quests is incomplete.");

#include "dovah/form_stubs/helpers/for_each_quest_dialogue_branch.h"
#include "dovah/form_stubs/helpers/for_each_quest_scene.h"
#include "dovah/form_stubs/helpers/for_each_quest_topic.h"
#include "./quest/loc_alias.h"
#include "./quest/ref_alias.h"
#include "../form_components/collection_conditions.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::quest;
   using wrapped_type = dovah::loaded_forms::Quest;
}

namespace {
   struct quest_type_name {
      wrapped_type::quest_type_t value;
      const char* name;
   };
   constexpr std::array quest_type_names = {
      quest_type_name{ wrapped_type::quest_type::none,             "none" },
      quest_type_name{ wrapped_type::quest_type::mages_guild,      "college of winterhold" },
      quest_type_name{ wrapped_type::quest_type::main,             "main" },
      quest_type_name{ wrapped_type::quest_type::thieves_guild,    "thieves guild" },
      quest_type_name{ wrapped_type::quest_type::dark_brotherhood, "dark brotherhood" },
      quest_type_name{ wrapped_type::quest_type::companions,       "companions" },
      quest_type_name{ wrapped_type::quest_type::miscellaneous,    "misc" },
      quest_type_name{ wrapped_type::quest_type::daedric,          "daedric" },
      quest_type_name{ wrapped_type::quest_type::sidequest,        "sidequest" },
      quest_type_name{ wrapped_type::quest_type::civil_war,        "civil_war" },
      quest_type_name{ wrapped_type::quest_type::dlc_dawnguard,    "dlc: dawnguard" },
      quest_type_name{ wrapped_type::quest_type::dlc_dragonborn,   "dlc: dragonborn" },
   };

   static std::optional<uint32_t> _id_for_new_alias(wrapped_type& form) {
      auto id = form.next_alias_id;
      for (; id < std::numeric_limits<uint16_t>::max(); ++id) {
         if (!form.lookup_alias_by_id(id))
            break;
      }
      if (id == std::numeric_limits<uint16_t>::max()) {
         for (id = 0; id < form.next_alias_id; ++id) {
            if (!form.lookup_alias_by_id(id))
               break;
         }
         if (id == form.next_alias_id)
            return {};
      }
      return id;
   }
}

#pragma region form
namespace {
   namespace _methods {
      int create_loc_alias(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;

         dovah::loaded_forms::LocationAlias* result = nullptr;

         self.before_edit();
         {
            auto id_opt = _id_for_new_alias(*form);
            if (!id_opt.has_value()) {
               cobb::lua::error(L, "no alias IDs available");
            }
            auto id = id_opt.value();
            if (id < form->next_alias_id) {
               cobb::lua::warning(L, "no new alias IDs available; recycling an old ID");
            } else {
               form->next_alias_id = id + 1;
            }
            auto*& alias_ptr = form->aliases.emplace_back();
            alias_ptr = result = new dovah::loaded_forms::LocationAlias(*form);
            alias_ptr->id = id;
         }
         self.after_edit();

         return wrappers::quest_loc_alias::wrap(L, self.stub, result);
      }
      int create_ref_alias(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;

         dovah::loaded_forms::ReferenceAlias* result = nullptr;

         self.before_edit();
         {
            auto id_opt = _id_for_new_alias(*form);
            if (!id_opt.has_value()) {
               cobb::lua::error(L, "no alias IDs available");
            }
            auto id = id_opt.value();
            if (id < form->next_alias_id) {
               cobb::lua::warning(L, "no new alias IDs available; recycling an old ID");
            } else {
               form->next_alias_id = id + 1;
            }
            auto*& alias_ptr = form->aliases.emplace_back();
            alias_ptr = result = new dovah::loaded_forms::ReferenceAlias(*form);
            alias_ptr->id = id;
         }
         self.after_edit();

         return wrappers::quest_ref_alias::wrap(L, self.stub, result);
      }
      int get_all_dialogue_branches(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub) {
            lua_createtable(L, 0, 0);
            return 1;
         }

         size_t expected = 0;
         dovah::form_stub_helpers::for_each_quest_dialogue_branch(*stub, [&expected](dovah::form_stub& branch) {
            ++expected;
         });

         lua_createtable(L, expected, 0);
         int i   = 0;
         int pos = lua_gettop(L);
         dovah::form_stub_helpers::for_each_quest_dialogue_branch(*stub, [L, &i, &pos](dovah::form_stub& branch) {
            int wcount = push_native_object(&branch);
            while (wcount--)
               lua_rawseti(L, pos, ++i);
         });
         assert(lua_gettop(L) == pos);
         return 1;
      }
      int get_all_dialogue_topics(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub) {
            lua_createtable(L, 0, 0);
            return 1;
         }

         size_t expected = 0;
         dovah::form_stub_helpers::for_each_quest_topic(*stub, [&expected](dovah::form_stub& topic) {
            ++expected;
         });

         lua_createtable(L, expected, 0);
         int i   = 0;
         int pos = lua_gettop(L);
         dovah::form_stub_helpers::for_each_quest_topic(*stub, [L, &i, &pos](dovah::form_stub& topic) {
            int wcount = push_native_object(&topic);
            while (wcount--)
               lua_rawseti(L, pos, ++i);
         });
         assert(lua_gettop(L) == pos);
         return 1;
      }
      int get_all_scenes(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* stub = self.stub;
         if (!stub) {
            lua_createtable(L, 0, 0);
            return 1;
         }

         size_t expected = 0;
         dovah::form_stub_helpers::for_each_quest_scene(*stub, [&expected](dovah::form_stub& topic) {
            ++expected;
         });

         lua_createtable(L, expected, 0);
         int i   = 0;
         int pos = lua_gettop(L);
         dovah::form_stub_helpers::for_each_quest_scene(*stub, [L, &i, &pos](dovah::form_stub& scene) {
            int wcount = push_native_object(&scene);
            while (wcount--)
               lua_rawseti(L, pos, ++i);
         });
         assert(lua_gettop(L) == pos);
         return 1;
      }
   }
   namespace _getters {
      template<wrapped_type::quest_flag::type Flag>
      int _quest_flag(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushboolean(L, (form->flags & Flag) != 0);
         return 1;
      }

      int aliases(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(dovahscript::wrapper_part_types::quest_alias);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::quest_alias_set.registry_key);
      }
      int aliases_by_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(dovahscript::wrapper_part_types::quest_alias_by_id);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::quest_alias_by_id_set.registry_key);
      }
      int dialogue_conditions(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return dovahscript::wrapper_likes::native_lists::condition_list::push(L, self, dovahscript::wrapper_likes::native_lists::condition_list::signatures::quest_dialogue);
      }
      int event_conditions(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         return dovahscript::wrapper_likes::native_lists::condition_list::push(L, self, dovahscript::wrapper_likes::native_lists::condition_list::signatures::quest_events);
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->name.c_str());
         return 1;
      }
      int object_window_category(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushstring(L, form->filter.c_str());
         return 1;
      }
      int priority(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->priority);
         return 1;
      }
      int quest_flags(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->flags);
         return 1;
      }
      int quest_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         using qt = wrapped_type::quest_type::type;
         for (const auto& e : quest_type_names) {
            if (form->quest_type == e.value) {
               lua_pushstring(L, e.name);
               return 1;
            }
         }
         lua_pushinteger(L, form->quest_type);
         return 1;
      }
   }
   namespace _setters {
      template<wrapped_type::quest_flag::type Flag>
      int _quest_flag(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "expected boolean");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         cobb::edit_bit(form->flags, Flag, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }

      int name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->name = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int object_window_category(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->filter = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      int priority(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->priority = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      int quest_flags(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isinteger(L, 2), 2, "expected integer");
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->flags = lua_tointeger(L, 2);
         self.after_edit();
         return 0;
      }
      int quest_type(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         lua_settop(L, 2);
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto  atype = lua_type(L, 2);
         //
         wrapped_type::quest_type_t value = wrapped_type::quest_type::none;
         switch (atype) {
            case LUA_TNONE:
            case LUA_TNIL:
               value = wrapped_type::quest_type::none;
               break;
            case LUA_TSTRING:
               {
                  bool  match = false;
                  auto* text  = lua_tostring(L, 2);
                  for (const auto& e : quest_type_names) {
                     if (_stricmp(text, e.name) == 0) {
                        value = e.value;
                        match = true;
                        break;
                     }
                  }
                  if (match)
                     break;
                  int isnum;
                  value = lua_tonumberx(L, 2, &isnum);
                  if (!isnum)
                     cobb::lua::error(L, "string \"%s\" is not a recognized quest type name", text);
               }
               break;
            case LUA_TNUMBER:
               {
                  int isnum;
                  value = lua_tointegerx(L, 2, &isnum);
                  if (!isnum)
                     cobb::lua::error(L, "expected a string name or an integer, preferring the former; got a non-integer number");
                  lua_warning(L, "quest.quest_type is being assigned an integer, not a named value; is this intentional?", 0);
               }
               break;
            default:
               cobb::lua::error(L, "expected a string name or an integer, preferring the former; got a %s", lua_typename(L, atype));
         }
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         self.before_edit();
         form->quest_type = value;
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "create_loc_alias",          &_methods::create_loc_alias },
      { "create_ref_alias",          &_methods::create_ref_alias },
      { "get_all_dialogue_branches", &_methods::get_all_dialogue_branches },
      { "get_all_dialogue_topics",   &_methods::get_all_dialogue_topics },
      { "get_all_scenes",            &_methods::get_all_scenes },
   };
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      #pragma region Quest flags
         { "allow_repeated_stages",        &_getters::_quest_flag<wrapped_type::quest_flag::allow_repeated_stages> },
         { "exclude_from_dialogue_export", &_getters::_quest_flag<wrapped_type::quest_flag::exclude_from_dialogue_export> },
         { "run_once",                     &_getters::_quest_flag<wrapped_type::quest_flag::run_once> },
         { "start_game_enabled",           &_getters::_quest_flag<wrapped_type::quest_flag::start_game_enabled> },
         { "warn_on_alias_fill_failure",   &_getters::_quest_flag<wrapped_type::quest_flag::warn_on_alias_fill_failure> },
      #pragma endregion
      { "aliases",                &_getters::aliases }, // collection
      { "aliases_by_id",          &_getters::aliases_by_id }, // collection
      { "dialogue_conditions",    &_getters::dialogue_conditions }, // collection
      { "event_conditions",       &_getters::event_conditions }, // collection
      { "name",                   &_getters::name },
      { "object_window_category", &_getters::object_window_category },
      { "priority",               &_getters::priority },
      { "quest_flags",            &_getters::quest_flags },
      { "quest_type",             &_getters::quest_type },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      #pragma region Quest flags
         { "allow_repeated_stages",        &_setters::_quest_flag<wrapped_type::quest_flag::allow_repeated_stages> },
         { "exclude_from_dialogue_export", &_setters::_quest_flag<wrapped_type::quest_flag::exclude_from_dialogue_export> },
         { "run_once",                     &_setters::_quest_flag<wrapped_type::quest_flag::run_once> },
         { "start_game_enabled",           &_setters::_quest_flag<wrapped_type::quest_flag::start_game_enabled> },
         { "warn_on_alias_fill_failure",   &_setters::_quest_flag<wrapped_type::quest_flag::warn_on_alias_fill_failure> },
      #pragma endregion
      { "name",                   &_setters::name },
      { "object_window_category", &_setters::object_window_category },
      { "priority",               &_setters::priority },
      { "quest_flags",            &_setters::quest_flags },
      { "quest_type",             &_setters::quest_type },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::quest_alias_set);
      define_collection_metatable(L, collections::quest_alias_by_id_set);
   }
}
#pragma endregion