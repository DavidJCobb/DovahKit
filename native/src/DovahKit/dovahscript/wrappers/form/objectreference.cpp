#include "objectreference.h"
#include <array>
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/istablelike.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"

#include "../../wrapper.h"
#include "../../core/classes.h"
#include "../../core/collections.h"
#include "../../pull_native_object.h"
#include "../../push_native_object.h"
#include "../../lua_libraries/form_types.h"

#include "../../api_helpers/pull_rotation.h"

#include "../../../dovah/form_stub_helpers.h"
#include "../../../dovah/forms/Form.h"
#include "../../../dovah/forms/ObjectReference.h"
#include "../../../dovah/notice_code_list.h"
#include "objectreference/position.h"
#include "objectreference/rotation.h"

#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for ObjectReferences is incomplete.");
//
// - Missing:
//    - All extra data
//    - Door open state
//    - Position setter (individual axes; see objectreference_position)
//    - Position setter (overwriting the whole "position" member)
//       - Should accept a vector3 or any vector-like object
//    - Rotation setter (overwriting the whole "rotation" member)
//       - Should accept an euler, vector3, matrix3x3, quaternion, or any "euler/vector3-like," "matrix-like," or "quaternion-like" object
//

namespace {
   using namespace dovahscript;
   using cls          = wrappers::objectreference;
   using wrapped_type = dovah::loaded_forms::ObjectReference;

   namespace _helpers {
      struct _flag_definition {
         const char* name;
         uint32_t value;
         std::initializer_list<uint8_t> base_form_types; // empty = matches all base form types
      };

      using ft = dovah::form_type;
      constexpr std::array refr_flags = {
         _flag_definition{ "persistent",              wrapped_type::form_flag::persistent },
         _flag_definition{ "disabled",                wrapped_type::form_flag::disabled },
         _flag_definition{ "hide_from_local_map",     wrapped_type::form_flag::hide_from_local_map_a,    { ft::door } },
         _flag_definition{ "doesnt_light_water",      wrapped_type::form_flag::doesnt_light_water,       { ft::light } },
         _flag_definition{ "is_inaccessible",         wrapped_type::form_flag::inaccessible,             { ft::door } },
         _flag_definition{ "hide_from_local_map",     wrapped_type::form_flag::hide_from_local_map_b,    { ft::activator, ft::statik, ft::tree } },
         _flag_definition{ "has_motion_blur",         wrapped_type::form_flag::motion_blur,              { ft::movable_static } },
         _flag_definition{ "starts_dead",             wrapped_type::form_flag::starts_dead,              { ft::actor } },
         _flag_definition{ "has_motion_blur",         wrapped_type::form_flag::motion_blur,              { ft::movable_static } },
         _flag_definition{ "visible_when_distant",    wrapped_type::form_flag::visible_when_distant,     { ft::activator, ft::statik, ft::tree } },
         _flag_definition{ "starts_dead",             wrapped_type::form_flag::starts_dead,              { ft::actor } },
         _flag_definition{ "is_full_lod",             wrapped_type::form_flag::is_full_lod },
         _flag_definition{ "never_fades",             wrapped_type::form_flag::never_fades,              { ft::light } },
         _flag_definition{ "doesnt_light_landscape",  wrapped_type::form_flag::doesnt_light_landscape,   { ft::light } },
         _flag_definition{ "no_ai_acquire",           wrapped_type::form_flag::no_ai_acquire,            { ft::actor, ft::container, ft::light, ft::ammo, ft::apparatus, ft::armor, ft::book, ft::ingredient, ft::key, ft::leveled_item, ft::misc_item, ft::note, ft::potion, ft::scroll, ft::soul_gem, ft::weapon } },
         //_flag_definition{ "filter",                 wrapped_type::form_flag::filter },       // expose this only once we're sure what it even friggin' does
         //_flag_definition{ "bounding_box",           wrapped_type::form_flag::bounding_box }, // expose this only once we're sure what it even friggin' does
         _flag_definition{ "reflected_by_cell_water", wrapped_type::form_flag::reflected_by_auto_water },
         _flag_definition{ "dont_havok_settle",       wrapped_type::form_flag::dont_havok_settle,        { ft::actor, ft::activator, ft::addon_node, ft::door, ft::light, ft::movable_static, ft::statik, ft::tree, ft::ammo, ft::apparatus, ft::armor, ft::book, ft::ingredient, ft::key, ft::leveled_item, ft::misc_item, ft::note, ft::potion, ft::scroll, ft::soul_gem, ft::weapon } },
         //_flag_definition{ "multibound",              wrapped_type::form_flag::multibound },  // expose this only once we're sure what it even friggin' does
      };

      const _flag_definition* get_definition_by_flag(uint32_t flag, dovah::form_type_t type) noexcept {
         for (auto& def : refr_flags) {
            if (def.value != flag)
               continue;
            auto& list = def.base_form_types;
            if (list.size()) {
               auto it = std::find(list.begin(), list.end(), type);
               if (it == list.end())
                  continue;
            }
            return &def;
         }
         return nullptr;
      }
   }

   namespace _getters {
      int _flag(lua_State* L) { // generic getter for any form flag; relies on an upvalue to indicate the flag; references a list near the top of this file
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base  = dovah::form_stub_helpers::get_base_form(self.stub);
         //
         constexpr int index_flag  = lua_upvalueindex(1);
         assert(lua_type(L, index_flag) == LUA_TNUMBER);
         uint32_t flag = lua_tointeger(L, index_flag);
         assert(flag && "The flag should not be zero. Storage/conversion error?");
         //
         const auto* definition = _helpers::get_definition_by_flag(flag, base ? base->formType : dovah::form_type::none);
         if (!definition) {
            return 0;
         }
         auto  flags = self.stub->get_record_flags();
         lua_pushboolean(L, (flags & flag) != 0);
         return 1;
      }
      //
      int base_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(self.stub);
         return push_native_object(base);
      }
      int parent_cell(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = self.stub->get_parent_form();
         if (!base || base->formType != dovah::form_type::cell)
            return 0;
         return push_native_object(base);
      }
      int position(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::refr_position);
         return core::subsystems::userdata::get().push(L, out, wrappers::objectreference_position::metatable_key);
      }
      int rotation(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         wrapper out = self;
         out.append_part(wrapper_part_types::refr_rotation);
         return core::subsystems::userdata::get().push(L, out, wrappers::objectreference_rotation::metatable_key);
      }
   }
   namespace _setters {
      int _flag(lua_State* L) { // generic setter for any form flag; relies on an upvalue to indicate the flag; references a list near the top of this file
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         bool  value = lua_toboolean(L, 2);
         auto* base  = dovah::form_stub_helpers::get_base_form(self.stub);
         //
         constexpr int index_flag  = lua_upvalueindex(1);
         assert(lua_type(L, index_flag) == LUA_TNUMBER);
         uint32_t flag = lua_tointeger(L, index_flag);
         assert(flag && "The flag should not be zero. Storage/conversion error?");
         //
         const auto* definition = _helpers::get_definition_by_flag(flag, base ? base->formType : dovah::form_type::none);
         if (!definition) {
            if (base)
               cobb::lua::error(L, "this flag is not available for references whose base forms are of this type");
            cobb::lua::error(L, "this flag is not available for references with no base form");
         }
         self.before_edit();
         self.stub->edit_record_flags(definition->value, value);
         self.after_edit();
         return 0;
      }
      //
      int base_form(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2);
         cobb::lua::argcheck(L, value != nullptr, 2, "form expected");
         if (!dovah::form_type_info::form_type_is_base_form(value->formType)) {
            cobb::lua::argerror(L, 2, "the provided form is not a base form");
         }
         if (!form)
            return 0;
         self.before_edit();
         form->base_form.set(*form, value);
         self.after_edit();
         return 0;
      }
      int position(lua_State* L) {
         static constexpr const std::array axis_names = { "x", "y", "z" };
         //
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         cobb::lua::argcheck(L, cobb::lua::istablelike(L, 2), 2, "table or userdata (position) expected");
         lua_settop(L, 2);
         for (size_t i = 0; i < axis_names.size(); ++i) {
            lua_getfield(L, 2, axis_names[i]);
            if (!lua_isnumber(L, -1)) {
               lua_pop(L, 1);
               lua_geti(L, 2, i + 1);
               if (!lua_isnumber(L, -1))
                  cobb::lua::error(L, "neither argument.%s nor argument[%d] were numbers", axis_names[i], i + 1);
            }
         }
         cobb::vector3<float> position;
         position.x = lua_tonumber(L, 3);
         position.y = lua_tonumber(L, 4);
         position.z = lua_tonumber(L, 5);
         lua_pop(L, axis_names.size());
         //
         self.before_edit();
         auto code = form->set_position(position);
         self.after_edit();
         if (code != dovah::default_notice_code) {
            switch (code) {
               using _ = dovah::notice_code;
               case _::cannot_set_position_of_orphaned_reference:
                  cobb::lua::error(L, "this reference has no parent cell (PlayerRef?), so its position cannot safely be set");
               case _::desired_position_is_outside_of_desired_cell: // shouldn't happen, as we're not requesting a specific cell
                  break;
               case _::failed_to_create_cell_to_move_reference_to:
                  cobb::lua::error(L, "the desired position lies outside of any existing cells, and DovahKit was unable to create a new cell");
               case _::cannot_reparent_hardcoded_reference:
                  cobb::lua::error(L, "hardcoded references cannot safely be reparented");
               case _::operation_not_allowed_on_form_working_copy: // shouldn't happen, as Lua should never be operating on working copies
                  break;
            }
            cobb::lua::error(L, "an internal error occurred while trying to set the form's position");
         }
         return 0;
      }
      int rotation(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* form = self.get_loaded_form_data<wrapped_type>();
         //
         cobb::euler value;
         if (!api_helpers::pull_rotation(L, 2, value)) {
            cobb::lua::argerror(L, 2, "the argument was not recognizable as an euler, matrix3x3, or a quaternion, nor a list of 3, 9, or 4 numbers");
         }
         if (!form)
            return 0;
         self.before_edit();
         form->rotation = { value.x, value.y, value.z };
         self.after_edit();
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "base_form",   &_getters::base_form },
      { "parent_cell", &_getters::parent_cell },
      { "position",    &_getters::position },
      { "rotation",    &_getters::rotation },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "base_form", &_setters::base_form },
      { "position",  &_getters::position },
      { "rotation",  &_getters::rotation },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      //int index_class       = lua_absindex(L, -3);
      int index_getter_list = lua_absindex(L, -2);
      int index_setter_list = lua_absindex(L, -1);
      //
      for (auto& def : _helpers::refr_flags) {
         lua_pushinteger(L, def.value);
         lua_pushcclosure(L, &_getters::_flag, 3);
         lua_setfield(L, index_getter_list, def.name);
         //
         lua_pushinteger(L, def.value);
         lua_pushcclosure(L, &_setters::_flag, 3);
         lua_setfield(L, index_setter_list, def.name);
      }
   }
}