#include "land_texture.h"
#include "../systems/messaging.h"
#include "../systems/permissions.h"
#include "../systems/userdata.h"

#include "../classes.h"
#include "../wrapper_util.h"
#include "../collections.h"

#include "land_texture/havok.h"

#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"

namespace {
   using namespace editor_script;
   using _wrapper_t     = wrappers::land_texture;
   using _loaded_form_t = dovah::loaded_forms::LandTexture;
}

#pragma region Collection: "grasses"
namespace {
   using namespace editor_script;

   namespace _collections::entries {
      static constexpr auto collection_key = _wrapper_t::grasses_collection_key;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->grasses.size());
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         auto  i    = lua_tointeger(L, 2);
         auto& list = form->grasses;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         return wrap_and_push_form(L, list[i]);
      }
      luastackchange_t member_function_insert(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_collection_wrapper(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         //
         int  pos_value = 2;
         bool has_index = false;
         //
         if (lua_gettop(L) >= 3) {
            has_index = true;
            pos_value = 3;
            luaL_argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
         }
         dovah::form_stub* target = nullptr;
         if (!lua_isnoneornil(L, pos_value)) {
            auto* w = wrapper_from_stack<wrappers::form>(L, pos_value);
            if (!w)
               cobb::lua::error(L, "expected a form or nil");
            target = w->stub;
         } else
            return 0;
         //
         if (!form)
            return 0;
         auto& list = form->grasses;
         auto  size = list.size();
         int   i    = size + 1;
         if (has_index) {
            i = lua_tointeger(L, 2);
            if (i < 1)
               cobb::lua::error(L, "indices below 1, such as %d, are not allowed", i);
            --i;
         }
         if (i >= size) {
            if (i > size) {
               cobb::lua::warning(L, "index %s is out of bounds; nil elements will be created between the end of the list and the new element", lua_tolstring(L, 2, nullptr));
            }
            list.resize(i + 1);
         } else {
            list.emplace(list.begin() + i);
         }
         self.before_edit();
         list[i].set(*form, target);
         self.after_edit();
         return 0;
      }
      luastackchange_t member_function_remove(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected an integer index");
         int isnum;
         int i = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "expected an integer index");
         if (!form)
            return 0;
         auto& list = form->grasses;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         self.before_edit();
         list[i].set(*form, nullptr);
         list.erase(list.begin() + i);
         self.after_edit();
         return 0;
      }
      luastackchange_t set_item(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         constexpr auto index_self  = 1;
         constexpr auto index_key   = 2;
         constexpr auto index_value = 3;
         //
         dovah::form_stub* target = nullptr;
         if (!lua_isnoneornil(L, index_value)) {
            auto* w = wrapper_from_stack<wrappers::form>(L, index_value);
            if (!w)
               cobb::lua::error(L, "you can only overwrite formlist entries with forms or nil");
            target = w->stub;
         }
         auto& self = get_collection_wrapper(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         int v = 0;
         int i = lua_tointegerx(L, index_key, &v);
         if (!v)
            cobb::lua::error(L, "indices in a formlist's entry list must be integers");
         if (i < 1)
            cobb::lua::error(L, "indices below 1, such as %d, are not allowed", i);
         --i;
         //
         auto& list = form->grasses;
         auto  size = list.size();
         if (i >= size) {
            if (i > size) {
               cobb::lua::warning(L, "index %s is out of bounds; nil elements will be created between the end of the list and the new element", lua_tolstring(L, index_key, nullptr));
            }
            list.resize(i + 1);
         }
         self.before_edit();
         list[i].set(*form, target);
         self.after_edit();
         return 0;
      }
   }
}
#pragma endregion

#pragma region form
namespace {
   namespace _getters {
      luastackchange_t physics(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::land_texture_physics);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::land_texture_havok::metatable_key);
      }
      luastackchange_t grasses(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(editor_script::wrapper_part_types::land_texture_grass);
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, _wrapper_t::grasses_collection_key);
      }
      luastackchange_t specular_exponent(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->specular_exponent);
         return 1;
      }
      luastackchange_t texture_set(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form = self.get_loaded_form_data<_loaded_form_t>();
         if (!form)
            return 0;
         return wrap_and_push_form(L, form->texture_set);
      }
   }
   namespace _setters {
      luastackchange_t specular_exponent(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum,      2, "integer expected");
         luaL_argcheck(L, value >= 0, 2, "the specular exponent cannot be negative");
         luaL_argcheck(L, value < 31, 2, "the specular exponent cannot exceed 30");
         if (!form)
            return 0;
         self.before_edit();
         form->specular_exponent = value;
         self.after_edit();
         return 0;
      }
      luastackchange_t texture_set(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<_wrapper_t>(L);
         auto* form  = self.get_loaded_form_data<_loaded_form_t>();
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::texture_set);
         if (!form)
            return 0;
         self.before_edit();
         form->texture_set.set(*form, value);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_methods = no_functions;
   
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_getters = {
      { "grasses",           &_getters::grasses },
      { "physics",           &_getters::physics },
      { "specular_exponent", &_getters::specular_exponent },
      { "texture_set",       &_getters::texture_set },
   };
   /*static*/ std::initializer_list<luaL_Reg> _wrapper_t::metatable_setters = {
      { "specular_exponent", &_setters::specular_exponent },
      { "texture_set",       &_setters::texture_set },
   };

   /*static*/ void _wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key           = _wrapper_t::grasses_collection_key,
         .garbage_collection     = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::entries::get_collection_length,
         .lookup_item_by_index   = &_collections::entries::lookup_item_by_index,
         .member_function_insert = &_collections::entries::member_function_insert,
         .member_function_remove = &_collections::entries::member_function_remove,
         .set_item               = &_collections::entries::set_item,
      });
   }
}
#pragma endregion