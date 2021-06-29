#include "quest.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../classes.h"
#include "../util.h"
#include "../wrapper_util.h"
#include "../collections.h"

#include "quest/alias.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::quest;
   using _loaded_form_t = dovah::loaded_forms::Quest;
}

#pragma region Collection: "aliases"
namespace {
   using namespace editor_script;

   namespace _collections::aliases {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, _wrapper_t::alias_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", _wrapper_t::alias_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form) {
            lua_pushinteger(L, 0);
            return 1;
         }
         lua_pushinteger(L, form->aliases.size());
         return 1;
      }
      luastackchange_t lookup_item_by_name(lua_State* L) {
         //
         // args: wrapper<papyrus_root>, name
         //
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         const char* name = lua_tostring(L, 2);
         if (!name)
            return 0;
         auto& list = form->aliases;
         auto  size = list.size();
         for (size_t i = 0; i < size; ++i) {
            const auto* alias = list[i];
            if (stricmp(alias->name.c_str(), name) == 0)
               return wrappers::quest_alias::wrap(L, self, alias);
         }
         return 0;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         auto  i    = lua_tointeger(L, 2);
         auto& list = form->aliases;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         return wrappers::quest_alias::wrap(L, self, list[i]);
      }
      luastackchange_t get_all_item_names(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         auto& list = form->aliases;
         //
         lua_createtable(L, 0, list.size());
         auto index_tbl = lua_gettop(L);
         //
         for (const auto* alias : list) {
            lua_pushboolean(L, true);
            lua_setfield(L, index_tbl, alias->name.c_str());
         }
         return 1;
      }
   }
}
#pragma endregion
#pragma region Collection: "aliases_by_id"
namespace {
   using namespace editor_script;

   namespace _collections::aliases_by_id {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, _wrapper_t::alias_id_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", _wrapper_t::alias_id_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form) {
            lua_pushinteger(L, 0);
            return 1;
         }
         uint32_t max = 0;
         for (auto* alias : form->aliases)
            if (alias && alias->id > max)
               max = alias->id;
         lua_pushinteger(L, max);
         return 1;
      }
      luastackchange_t lookup_item_by_name(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         int   isnum;
         auto  i     = lua_tointegerx(L, 2, &isnum);
         if (!isnum)
            return 0;
         auto* alias = form->lookup_alias_by_id(i);
         if (!alias)
            return 0;
         return wrappers::quest_alias::wrap(L, self, alias);
      }
      luastackchange_t get_all_item_names(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         auto& list = form->aliases;
         //
         lua_createtable(L, 0, list.size());
         auto index_tbl = lua_gettop(L);
         //
         for (const auto* alias : list) {
            lua_pushboolean(L, true);
            lua_rawseti(L, index_tbl, alias->id);
         }
         return 1;
      }
   }
}
#pragma endregion

namespace {
   struct quest_type_name {
      _loaded_form_t::quest_type_t value;
      const char* name;
   };
   constexpr std::array quest_type_names = {
      quest_type_name{ _loaded_form_t::quest_type::none,             "none" },
      quest_type_name{ _loaded_form_t::quest_type::mages_guild,      "college of winterhold" },
      quest_type_name{ _loaded_form_t::quest_type::main,             "main" },
      quest_type_name{ _loaded_form_t::quest_type::thieves_guild,    "thieves guild" },
      quest_type_name{ _loaded_form_t::quest_type::dark_brotherhood, "dark brotherhood" },
      quest_type_name{ _loaded_form_t::quest_type::companions,       "companions" },
      quest_type_name{ _loaded_form_t::quest_type::miscellaneous,    "misc" },
      quest_type_name{ _loaded_form_t::quest_type::daedric,          "daedric" },
      quest_type_name{ _loaded_form_t::quest_type::sidequest,        "sidequest" },
      quest_type_name{ _loaded_form_t::quest_type::civil_war,        "civil_war" },
      quest_type_name{ _loaded_form_t::quest_type::dlc_dawnguard,    "dlc: dawnguard" },
      quest_type_name{ _loaded_form_t::quest_type::dlc_dragonborn,   "dlc: dragonborn" },
   };
}

#pragma region form
namespace {
   namespace _getters {
      luastackchange_t aliases(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(editor_script::wrapper_part_types::quest_alias);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, _wrapper_t::alias_collection_key);
      }
      luastackchange_t aliases_by_id(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(editor_script::wrapper_part_types::quest_alias_by_id);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, _wrapper_t::alias_id_collection_key);
      }
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->name.c_str());
         return 1;
      }
      luastackchange_t object_window_category(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushstring(L, form->filter.c_str());
         return 1;
      }
      luastackchange_t priority(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushnumber(L, form->priority);
         return 1;
      }
      luastackchange_t quest_flags(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->flags);
         return 1;
      }
      luastackchange_t quest_type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         using qt = _loaded_form_t::quest_type::type;
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
      luastackchange_t name(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->name = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t object_window_category(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "expected string");
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->filter = lua_tostring(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t priority(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected number");
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->priority = lua_tonumber(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t quest_flags(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "expected integer");
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->flags = lua_tointeger(L, 2);
         self.after_edit();
         return 0;
      }
      luastackchange_t quest_type(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         lua_settop(L, 2);
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto  atype = lua_type(L, 2);
         //
         _loaded_form_t::quest_type_t value = _loaded_form_t::quest_type::none;
         switch (atype) {
            case LUA_TNONE:
            case LUA_TNIL:
               value = _loaded_form_t::quest_type::none;
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
                     luaL_error(L, "string \"%s\" is not a recognized quest type name", text);
               }
               break;
            case LUA_TNUMBER:
               {
                  int isnum;
                  value = lua_tointegerx(L, 2, &isnum);
                  if (!isnum)
                     luaL_error(L, "expected a string name or an integer, preferring the former; got a non-integer number");
                  lua_warning(L, "quest.quest_type is being assigned an integer, not a named value; is this intentional?", 0);
               }
               break;
            default:
               luaL_error(L, "expected a string name or an integer, preferring the former; got a %s", lua_typename(L, atype));
         }
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         self.before_edit();
         form->quest_type = value;
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "aliases",                &_getters::aliases }, // collection
      { "aliases_by_id",          &_getters::aliases_by_id }, // collection
      { "name",                   &_getters::name },
      { "object_window_category", &_getters::object_window_category },
      { "priority",               &_getters::priority },
      { "quest_flags",            &_getters::quest_flags },
      { "quest_type",             &_getters::quest_type },
   };
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "name",                   &_setters::name },
      { "object_window_category", &_setters::object_window_category },
      { "priority",               &_setters::priority },
      { "quest_flags",            &_setters::quest_flags },
      { "quest_type",             &_setters::quest_type },
   };

   /*static*/ void _wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key          = _wrapper_t::alias_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_all_item_names     = &_collections::aliases::get_all_item_names,
         .get_collection_length  = &_collections::aliases::get_collection_length,
         .items_are_named        = true,
         .lookup_item_by_name    = &_collections::aliases::lookup_item_by_name,
         .lookup_item_by_index   = &_collections::aliases::lookup_item_by_index,
      });
      define_collection_metatable(L, {
         .registry_key          = _wrapper_t::alias_id_collection_key,
         .garbage_collection    = &wrapper::__gc,
         //
         .get_all_item_names     = &_collections::aliases_by_id::get_all_item_names,
         .get_collection_length  = &_collections::aliases_by_id::get_collection_length,
         .items_are_named        = true,
         .lookup_item_by_name    = &_collections::aliases_by_id::lookup_item_by_name,
      });
   }
}
#pragma endregion