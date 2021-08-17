#include "quest.h"
#include "../../../helpers/lua/error.h"
#include "../../core/subsystems/coordinator.h"
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

#ifndef _DEBUG
   #pragma message("WARNING: Are you compiling in Release? The Lua API for quests is incomplete!")
#endif

namespace {
   using namespace dovahscript;
   using cls          = wrappers::quest;
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
}

#pragma region form
namespace {
   namespace _getters {
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
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "aliases",                &_getters::aliases }, // collection
      { "aliases_by_id",          &_getters::aliases_by_id }, // collection
      { "name",                   &_getters::name },
      { "object_window_category", &_getters::object_window_category },
      { "priority",               &_getters::priority },
      { "quest_flags",            &_getters::quest_flags },
      { "quest_type",             &_getters::quest_type },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "name",                   &_setters::name },
      { "object_window_category", &_setters::object_window_category },
      { "priority",               &_setters::priority },
      { "quest_flags",            &_setters::quest_flags },
      { "quest_type",             &_setters::quest_type },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, collections::quest_alias_set);
      define_collection_metatable(L, collections::quest_alias_by_id_set);
   }
}
#pragma endregion