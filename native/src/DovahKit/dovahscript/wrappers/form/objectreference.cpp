#include "objectreference.h"
#include <array>
#include <span>
#include "helpers/lua/error.h"
#include "helpers/lua/istablelike.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"

#include "../../wrapper.h"
#include "../../core/classes.h"
#include "../../core/collections.h"
#include "../../pull_native_object.h"
#include "../../push_native_object.h"
#include "../../lua_libraries/form_types.h"

#include "../../api_helpers/pull_rotation.h"

#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/exceptions/object_reference_move_failed.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/ObjectReference.h"
#include "./objectreference/position.h"
#include "./objectreference/rotation.h"

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

   constexpr const auto all_item_types = std::array{
      dovah::form_type::ammo,
      dovah::form_type::apparatus,
      dovah::form_type::armor,
      dovah::form_type::book,
      dovah::form_type::ingredient,
      dovah::form_type::key,
      dovah::form_type::leveled_item,
      dovah::form_type::light,
      dovah::form_type::misc_item,
      dovah::form_type::note,
      dovah::form_type::potion,
      dovah::form_type::scroll,
      dovah::form_type::soul_gem,
      dovah::form_type::weapon
   };
   
   template<auto FormTypes, bool AllItems>
   bool _form_type_matches(dovah::form_type desired) {
      for (auto ft : FormTypes)
         if (ft == desired)
            return true;
      if constexpr (AllItems) {
         for (auto ft : all_item_types)
            if (ft == desired)
               return true;
      }
      return false;
   }

   constexpr bool can_override_navmesh_gen(dovah::form_type ft) {
      switch (ft) {
         case dovah::form_type::activator:
         case dovah::form_type::container:
         case dovah::form_type::movable_static:
         case dovah::form_type::statik:
         case dovah::form_type::static_collection:
            return true;
      }
      return false;
   }

   namespace _getters {
      template<uint32_t Flag>
      int _common_record_flag(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto flags = self.stub->get_record_flags();
         lua_pushboolean(L, (flags & Flag) != 0);
         return 1;
      }
      template<uint32_t Flag, auto FormTypes, bool AllItems = false>
      int _typed_record_flag(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base)
            return 0;
         bool relevant = _form_type_matches<FormTypes, AllItems>(base->form_type);
         if (!relevant)
            return 0;
         auto flags = self.stub->get_record_flags();
         lua_pushboolean(L, (flags & Flag) != 0);
         return 1;
      }
      
      int base_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         return push_native_object(base);
      }
      int hide_from_local_map(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base)
            return 0;
         auto flags = self.stub->get_record_flags();
         switch (base->form_type) {
            case dovah::form_type::door:
               lua_pushboolean(L, (flags & wrapped_type::form_flag::hide_from_local_map_a));
               return 1;
            case dovah::form_type::activator:
            case dovah::form_type::statik:
            case dovah::form_type::tree:
               lua_pushboolean(L, (flags & wrapped_type::form_flag::hide_from_local_map_b));
               return 1;
         }
         return 0;
      }
      int is_full_lod(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base || base->form_type == dovah::form_type::light)
            return 0;
         auto flags = self.stub->get_record_flags();
         lua_pushboolean(L, (flags & wrapped_type::form_flag::is_full_lod));
         return 1;
      }
      int is_sky_marker(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base || base->formID != dovah::hardcoded_form_ids::XMarkerHeading)
            return 0;
         auto flags = self.stub->get_record_flags();
         lua_pushboolean(L, (flags & wrapped_type::form_flag::is_sky_marker));
         return 1;
      }
      int navmesh_generation_override(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (base) {
            if (!can_override_navmesh_gen(base->form_type))
               return 0;
         } else {
            return 0;
         }
         auto flags = self.stub->get_record_flags();
         if (flags & wrapped_type::form_flag::navmesh_generation_filter) {
            lua_pushstring(L, "filter");
            return 1;
         }
         if (flags & wrapped_type::form_flag::navmesh_generation_obb) {
            lua_pushstring(L, "obb");
            return 1;
         }
         if (flags & wrapped_type::form_flag::navmesh_generation_ground) {
            lua_pushstring(L, "ground");
            return 1;
         }
         lua_pushnil(L);
         return 1;
      }
      int parent_cell(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         auto* base = self.stub->get_parent_form();
         if (!base || base->form_type != dovah::form_type::cell)
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
      template<uint32_t Flag>
      int _common_record_flag(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         self.before_edit();
         self.stub->edit_record_flags(Flag, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      template<uint32_t Flag, auto FormTypes, bool AllItems = false>
      int _typed_record_flag(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base)
            cobb::lua::error(L, "this flag is not available for references whose base forms are of this type");
         bool relevant = _form_type_matches<FormTypes, AllItems>(base->form_type);
         if (!relevant)
            cobb::lua::error(L, "this flag is not available for references whose base forms are of this type");
         self.before_edit();
         self.stub->edit_record_flags(Flag, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      
      int base_form(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* form  = self.get_loaded_form_data<wrapped_type>();
         auto* value = pull_form_stub_argument(L, 2);
         cobb::lua::argcheck(L, value != nullptr, 2, "form expected");
         if (!dovah::form_type_is_base_form(value->form_type)) {
            cobb::lua::argerror(L, 2, "the provided form is not a base form");
         }
         if (!form)
            return 0;
         self.before_edit();
         form->base_form.set(*form, value);
         self.after_edit();
         return 0;
      }
      int hide_from_local_map(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base)
            cobb::lua::error(L, "this flag is not available for references whose base forms are of this type");
         uint32_t flag = 0;
         switch (base->form_type) {
            case dovah::form_type::door:
               flag = wrapped_type::form_flag::hide_from_local_map_a;
               break;
            case dovah::form_type::activator:
            case dovah::form_type::statik:
            case dovah::form_type::tree:
               flag = wrapped_type::form_flag::hide_from_local_map_b;
               break;
            default:
               cobb::lua::error(L, "this flag is not available for references whose base forms are of this type");
         }
         self.before_edit();
         self.stub->edit_record_flags(flag, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int is_full_lod(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base || base->form_type == dovah::form_type::light)
            cobb::lua::error(L, "this flag is not available for references whose base forms are of this type");
         self.before_edit();
         self.stub->edit_record_flags(wrapped_type::form_flag::is_full_lod, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int is_sky_marker(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (!base || base->formID != dovah::hardcoded_form_ids::XMarkerHeading)
            cobb::lua::error(L, "this flag is only available for references whose base forms are XMarkerHeading");
         self.before_edit();
         self.stub->edit_record_flags(wrapped_type::form_flag::is_sky_marker, lua_toboolean(L, 2));
         self.after_edit();
         return 0;
      }
      int navmesh_generation_override(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isstring(L, 2) || lua_isnil(L, 2), 2, "string or nil expected");
         if (!self.stub)
            return 0;
         auto* base = dovah::form_stub_helpers::get_base_form(*self.stub);
         if (base) {
            if (!can_override_navmesh_gen(base->form_type)) {
               if (lua_isnil(L, 2))
                  return 0;
               cobb::lua::error(L, "this property is not available for references whose base forms are of this type");
            }
         } else {
            if (lua_isnil(L, 2))
               return 0;
            cobb::lua::error(L, "this property is not available for references whose base forms are of this type");
         }

         uint32_t keep = 0;
         if (lua_isstring(L, 2)) {
            std::string_view value = lua_tostring(L, 2);
            if (value == "filter")
               keep = wrapped_type::form_flag::navmesh_generation_filter;
            else if (value == "obb")
               keep = wrapped_type::form_flag::navmesh_generation_obb;
            else if (value == "ground")
               keep = wrapped_type::form_flag::navmesh_generation_ground;
            else
               cobb::lua::argerror(L, 2, "unrecognized navmesh generation override type");
         }
         constexpr const auto all_flags = (
            wrapped_type::form_flag::navmesh_generation_filter |
            wrapped_type::form_flag::navmesh_generation_obb |
            wrapped_type::form_flag::navmesh_generation_ground
         );

         self.before_edit();
         self.stub->edit_record_flags(all_flags, false);
         self.stub->edit_record_flags(keep,      true);
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
         {
            using exception  = dovah::exceptions::object_reference_move_failed;
            using error_code = exception::error_code;

            self.before_edit();
            try {
               form->set_position(position);
            } catch (const exception& ex) {
               self.after_edit(); // TODO: Add a way to signal a failed edit (i.e. so we don't mark the form as edited). TODO: RAII (in lieu of no "finally" blocks).
               switch (ex.code) {
                  case error_code::reference_is_orphaned:
                     cobb::lua::error(L, "this reference has no parent cell (PlayerRef?), so its position cannot safely be set");
                  case error_code::desired_position_is_outside_of_desired_cell: // shouldn't happen, as we're not requesting a specific cell
                     break;
                  case error_code::failed_to_create_destination_cell:
                     cobb::lua::error(L, "the desired position lies outside of any existing cells, and DovahKit was unable to create a new cell");
                  case error_code::reference_is_hardcoded:
                     cobb::lua::error(L, "hardcoded references cannot safely be reparented");
                  case error_code::operation_not_allowed_on_form_working_copy: // shouldn't happen, as Lua should never be operating on working copies
                     break;
               }
               cobb::lua::error(L, "an internal error occurred while trying to set the form's position");
               return 0;
            }
            self.after_edit();
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
            if (lua_type(L, 2) == LUA_TTABLE) {
               lua_len(L, 2);
               bool faux_euler = (lua_tonumber(L, -1) == 3);
               lua_pop(L, 1);
               if (faux_euler)
                  cobb::lua::argerror(L, 2, "you must construct and pass an euler object; a table of three numbers is not sufficient, because we can't tell whether those are degrees or radians");
            }
            cobb::lua::argerror(L, 2, "the argument was not recognizable as an euler, matrix3x3, or a quaternion, nor a list of 9 (matrix) or 4 (quaternion) numbers");
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

namespace {
   using form_type   = dovah::form_type;
   using record_flag = wrapped_type::form_flag;
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      #pragma region Record flags
         #pragma region Common
            { "disabled",                &_getters::_common_record_flag<record_flag::disabled> },
            { "is_multibound",           &_getters::_common_record_flag<record_flag::multibound> },
            { "persistent",              &_getters::_common_record_flag<record_flag::persistent> },
            { "reflected_by_auto_water", &_getters::_common_record_flag<record_flag::reflected_by_auto_water> },
            { "turn_off_fire",           &_getters::_common_record_flag<record_flag::turn_off_fire> },
         #pragma endregion
         #pragma region Typed
            { "casts_shadows",          &_getters::_typed_record_flag<record_flag::casts_shadows,          (std::array{ form_type::light })> },
            { "doesnt_light_landscape", &_getters::_typed_record_flag<record_flag::doesnt_light_landscape, (std::array{ form_type::light })> },
            { "doesnt_light_water",     &_getters::_typed_record_flag<record_flag::doesnt_light_water,     (std::array{ form_type::light })> },
            {
               "dont_havok_settle",
               &_getters::_typed_record_flag<
                  record_flag::dont_havok_settle,
                  (std::array{
                     form_type::activator,
                     form_type::actor_base,
                     form_type::addon_node,
                     form_type::door,
                     form_type::light,
                     form_type::movable_static,
                     form_type::statik,
                     form_type::tree
                  }),
                  true // AllItems
               >
            },
            { "inaccessible",           &_getters::_typed_record_flag<record_flag::inaccessible,           (std::array{ form_type::door })> },
            { "motion_blur",            &_getters::_typed_record_flag<record_flag::motion_blur,            (std::array{ form_type::movable_static })> },
            {
               "no_ai_acquire",
               &_getters::_typed_record_flag<record_flag::no_ai_acquire, (std::array{ form_type::actor_base, form_type::container, form_type::light }), true>
            },
            {
               "no_respawn",
               &_getters::_typed_record_flag<
                  record_flag::no_respawn,
                  (std::array{ form_type::addon_node, form_type::door, form_type::light, form_type::movable_static, form_type::statik, form_type::tree }),
                  true // AllItems
               >
            },
            { "never_fades",            &_getters::_typed_record_flag<record_flag::never_fades,            (std::array{ form_type::light })> },
            { "starts_dead",            &_getters::_typed_record_flag<record_flag::starts_dead,            (std::array{ form_type::actor_base })> },
            { "visible_when_distant",   &_getters::_typed_record_flag<record_flag::visible_when_distant,   (std::array{ form_type::activator, form_type::statik, form_type::tree })> },
         #pragma endregion
      #pragma endregion
      { "base_form",           &_getters::base_form },
      { "hide_from_local_map", &_getters::hide_from_local_map },
      { "is_full_lod",         &_getters::is_full_lod },
      { "is_sky_marker",       &_getters::is_sky_marker },
      { "navmesh_generation_override", &_getters::navmesh_generation_override },
      { "parent_cell",         &_getters::parent_cell },
      { "position",            &_getters::position },
      { "rotation",            &_getters::rotation },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      #pragma region Record flags
         #pragma region Common
            { "disabled",                &_setters::_common_record_flag<record_flag::disabled> },
            { "is_multibound",           &_setters::_common_record_flag<record_flag::multibound> },
            { "persistent",              &_setters::_common_record_flag<record_flag::persistent> },
            { "reflected_by_auto_water", &_setters::_common_record_flag<record_flag::reflected_by_auto_water> },
            { "turn_off_fire",           &_setters::_common_record_flag<record_flag::turn_off_fire> },
         #pragma endregion
         #pragma region Typed
            { "casts_shadows",          &_setters::_typed_record_flag<record_flag::casts_shadows,          (std::array{ form_type::light })> },
            { "doesnt_light_landscape", &_setters::_typed_record_flag<record_flag::doesnt_light_landscape, (std::array{ form_type::light })> },
            { "doesnt_light_water",     &_setters::_typed_record_flag<record_flag::doesnt_light_water,     (std::array{ form_type::light })> },
            {
               "dont_havok_settle",
               &_setters::_typed_record_flag<
                  record_flag::dont_havok_settle,
                  (std::array{
                     form_type::activator,
                     form_type::actor_base,
                     form_type::addon_node,
                     form_type::door,
                     form_type::light,
                     form_type::movable_static,
                     form_type::statik,
                     form_type::tree
                  }),
                  true // AllItems
               >
            },
            { "inaccessible",           &_setters::_typed_record_flag<record_flag::inaccessible,           (std::array{ form_type::door })> },
            { "motion_blur",            &_setters::_typed_record_flag<record_flag::motion_blur,            (std::array{ form_type::movable_static })> },
            {
               "no_ai_acquire",
               &_setters::_typed_record_flag<record_flag::no_ai_acquire, (std::array{ form_type::actor_base, form_type::container, form_type::light }), true>
            },
            {
               "no_respawn",
               &_setters::_typed_record_flag<
                  record_flag::no_respawn,
                  (std::array{ form_type::activator, form_type::addon_node, form_type::door, form_type::light, form_type::movable_static, form_type::statik, form_type::tree }),
                  true // AllItems
               >
            },
            { "never_fades",            &_setters::_typed_record_flag<record_flag::never_fades,            (std::array{ form_type::light })> },
            { "starts_dead",            &_setters::_typed_record_flag<record_flag::starts_dead,            (std::array{ form_type::actor_base })> },
            { "visible_when_distant",   &_setters::_typed_record_flag<record_flag::visible_when_distant,   (std::array{ form_type::activator, form_type::statik, form_type::tree })> },
         #pragma endregion
      #pragma endregion
      { "base_form",           &_setters::base_form },
      { "hide_from_local_map", &_setters::hide_from_local_map },
      { "is_full_lod",         &_setters::is_full_lod },
      { "is_sky_marker",       &_setters::is_sky_marker },
      { "navmesh_generation_override", &_setters::navmesh_generation_override },
      { "position",            &_setters::position },
      { "rotation",            &_setters::rotation },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
   }
}