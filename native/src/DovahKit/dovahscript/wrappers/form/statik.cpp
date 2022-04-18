#include "statik.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/istablelike.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../pull_native_object.h"
#include "../../push_native_object.h"
#include "../../wrapper.h"

#include "../../../dovah/forms/Static.h"
#include "static/directional_material.h"
#include "static/distant_lod_paths.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::statik;
   using wrapped_type = cls::wrapped_type;

   namespace _getters {
      int directional_material(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::static_directional_material);
         return core::subsystems::userdata::get().push(L, out, wrappers::static_directional_material::metatable_key);
      }
      int distant_lod_paths(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         if (!form)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::static_distant_lod_paths);
         return core::subsystems::userdata::get().push(L, out, wrappers::static_distant_lod_paths::metatable_key);
      }
      //
      template<uint32_t flag> int _form_flag(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         lua_pushboolean(L, (self.stub->get_record_flags() & flag));
         return 1;
      }
   }
   namespace _setters {
      int directional_material(lua_State* L) {
         using dm_t = wrapped_type::directional_material_data;
         //
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         //
         auto* arg_dm = wrapper_from_stack<wrappers::static_directional_material>(L, 2);
         if (!arg_dm) {
            if (lua_isnil(L, 2)) {
               if (!form)
                  return 0;
               self.before_edit();
               form->directional_material.material_object.set(*form, nullptr);
               form->directional_material = {};
               self.after_edit();
               return 0;
            }
            if (cobb::lua::istablelike(L, 2)) {
               if (!form)
                  return 0;
               lua_settop(L, 2);
               dm_t::flags_t     flags = 0;
               float             max_angle;
               dovah::form_stub* mat_form = nullptr;
               //
               lua_getfield(L, 2, "material_object");
               {
                  auto* mato = wrapper_from_stack<wrappers::form>(L, 3);
                  cobb::lua::argcheck(L, mato != nullptr, 2, "arg.material_object must be a material object form");
                  mato->error_if_wrong_form_type(L, 2, dovah::form_type::material_object, true);
                  mat_form = mato->stub;
               }
               //
               lua_settop(L, 2);
               lua_getfield(L, 2, "max_angle");
               cobb::lua::argcheck(L, lua_isnumber(L, 3), 2, "arg.max_angle must be a number");
               max_angle = lua_tonumber(L, 3);
               //
               lua_settop(L, 2);
               lua_getfield(L, 2, "is_snow");
               if (!lua_isnil(L, 3)) {
                  cobb::lua::argcheck(L, lua_isboolean(L, 3), 2, "arg.is_snow must be a boolean");
                  if (lua_toboolean(L, 3))
                     flags |= dm_t::flag::is_snow;
               }
               //
               self.before_edit();
               form->directional_material.flags = flags;
               form->directional_material.material_object.set(*form, mat_form);
               form->directional_material.max_angle = max_angle;
               self.after_edit();
               //
               return 0;
            }
            cobb::lua::argerror(L, 2, "expected nil, another static_directional_material, or a table with appropriate fields");
         }
         //
         // Copy data from another STAT/DNAM:
         //
         if (arg_dm->stub == self.stub) // skip self-assignment
            return 0;
         self.before_edit();
         auto& arg_data = arg_dm->get_loaded_form_data<wrapped_type>()->directional_material;
         form->directional_material.flags = arg_data.flags;
         form->directional_material.material_object.set(*form, arg_data.material_object);
         form->directional_material.max_angle = arg_data.max_angle;
         self.after_edit();
         //
         return 0;
      }
      int distant_lod_paths(lua_State* L) {
         using dm_t = wrapped_type::directional_material_data;
         //
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         //
         if (cobb::lua::istablelike(L, 2)) {
            lua_settop(L, 2);
            for (size_t i = 0; i < 4; ++i) {
               lua_geti(L, 2, i);
               if (!(lua_isstring(L, 3) || lua_isnil(L, 3)))
                  cobb::lua::error(L, "arg[%d] is not a string or nil", i);
               lua_settop(L, 2);
            }
            if (!form)
               return 0;
            self.before_edit();
            for (size_t i = 0; i < 4; ++i) {
               lua_geti(L, 2, i);
               if (lua_isnil(L, 3)) {
                  form->distant_lod_paths[i].clear();
               } else {
                  form->distant_lod_paths[i] = lua_tostring(L, 3);
               }
               lua_settop(L, 2);
            }
            self.after_edit();
            return 0;
         }
         if (lua_isnil(L, 2)) {
            if (!form)
               return 0;
            self.before_edit();
            for (auto& path : form->distant_lod_paths)
               path.clear();
            self.after_edit();
            return 0;
         }
         cobb::lua::argerror(L, 2, "nil or a table expected");
      }
      //
      template<uint32_t flag> int _form_flag(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "expected string");
         if (!self.stub)
            return 0;
         self.before_edit();
         self.stub->edit_record_flags(flag, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "directional_material", &_getters::directional_material },
      { "distant_lod_paths",    &_getters::distant_lod_paths },
      //
      { "addon_lod_object",     &_getters::_form_flag<wrapped_type::form_flag::addon_lod_object> },
      { "allow_child_interact", &_getters::_form_flag<wrapped_type::form_flag::child_can_use> },
      { "has_currents",         &_getters::_form_flag<wrapped_type::form_flag::has_currents> },
      { "has_distant_lod",      &_getters::_form_flag<wrapped_type::form_flag::has_distant_lod> },
      { "has_tree_lod",         &_getters::_form_flag<wrapped_type::form_flag::has_tree_lod> },
      { "hide_from_local_map",  &_getters::_form_flag<wrapped_type::form_flag::hide_from_local_map> },
      { "is_marker",            &_getters::_form_flag<wrapped_type::form_flag::is_marker> },
      { "is_obstacle",          &_getters::_form_flag<wrapped_type::form_flag::obstacle> },
      { "never_fades",          &_getters::_form_flag<wrapped_type::form_flag::never_fades> },
      { "show_in_world_map",    &_getters::_form_flag<wrapped_type::form_flag::show_in_world_map> },
      { "uses_hd_lod_texture",  &_getters::_form_flag<wrapped_type::form_flag::uses_hd_lod_texture> },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "directional_material", &_setters::directional_material },
      { "distant_lod_paths",    &_setters::distant_lod_paths },
      //
      { "addon_lod_object",     &_setters::_form_flag<wrapped_type::form_flag::addon_lod_object> },
      { "allow_child_interact", &_setters::_form_flag<wrapped_type::form_flag::child_can_use> },
      { "has_currents",         &_setters::_form_flag<wrapped_type::form_flag::has_currents> },
      { "has_distant_lod",      &_setters::_form_flag<wrapped_type::form_flag::has_distant_lod> },
      { "has_tree_lod",         &_setters::_form_flag<wrapped_type::form_flag::has_tree_lod> },
      { "hide_from_local_map",  &_setters::_form_flag<wrapped_type::form_flag::hide_from_local_map> },
      { "is_marker",            &_setters::_form_flag<wrapped_type::form_flag::is_marker> },
      { "is_obstacle",          &_setters::_form_flag<wrapped_type::form_flag::obstacle> },
      { "never_fades",          &_setters::_form_flag<wrapped_type::form_flag::never_fades> },
      { "show_in_world_map",    &_setters::_form_flag<wrapped_type::form_flag::show_in_world_map> },
      { "uses_hd_lod_texture",  &_setters::_form_flag<wrapped_type::form_flag::uses_hd_lod_texture> },
   };
}
#pragma endregion