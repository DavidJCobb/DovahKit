#include "objectreference.h"
#include <array>
#include <span>
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

   // Helpers for form flags
   namespace _helpers {
      using ft = dovah::form_type;

      template<typename T> concept IsFormTypeValue = std::is_same_v<T, dovah::form_type::type> || std::is_same_v<T, dovah::form_type_t>;

      //
      // Constexpr-friendly list of form types, initializable from std::array.
      //
      struct form_type_list {
         public:
            using value_type = dovah::form_type_t;
            static constexpr size_t value_count = 25;

            using list_type = std::array<value_type, value_count>;
            using iterator       = list_type::iterator;
            using const_iterator = list_type::const_iterator;

         private:
            list_type list  = {};
            size_t    count = 0;
         public:
            constexpr form_type_list() {}
            constexpr form_type_list(std::initializer_list<value_type> values) {
               for (auto v : values)
                  this->list[this->count++] = v;
            }
            template<size_t S> constexpr form_type_list(const std::array<value_type, S> arr) {
               static_assert(S < value_count, "The form_type_list::list member needs to be larger in order to hold all possible lists.");
               for (auto v : arr)
                  this->list[this->count++] = v;
            }

            constexpr bool empty() const noexcept { return this->count != 0; }
            constexpr size_t size() const noexcept { return this->count; }

            constexpr value_type& operator[](size_t i) noexcept { return this->list[i]; }
            constexpr const value_type& operator[](size_t i) const noexcept { return this->list[i]; }

            iterator begin() noexcept { return this->list.begin(); }
            iterator end() noexcept { return this->list.begin(); }
            const_iterator begin() const noexcept { return this->list.cbegin(); }
            const_iterator end() const noexcept { return this->list.cbegin(); }
            const_iterator cbegin() const noexcept { return this->list.cbegin() + this->count; }
            const_iterator cend() const noexcept { return this->list.cbegin() + this->count; }

            inline constexpr bool contains(value_type t) const noexcept {
               for (auto v : list)
                  if (v == t)
                     return true;
               return false;
            }
      };

      template<dovah::form_type_t... Types> static constexpr auto form_types = std::array{ Types... };

      template<typename Ta, typename Tb, size_t Sa, size_t Sb> requires IsFormTypeValue<Ta> && IsFormTypeValue<Tb>
      static constexpr const std::array<dovah::form_type_t, Sa + Sb> concatenate_form_type_lists(const std::array<Ta, Sa> a, const std::array<Tb, Sb> b) {
         std::array<dovah::form_type_t, Sa + Sb> out = {};
         size_t i = 0;
         for (auto v : a)
            out[i++] = (dovah::form_type_t)v;
         for (auto v : b)
            out[i++] = (dovah::form_type_t)v;
         return out;
      }

      static constexpr auto all_item_types = form_types<ft::ammo, ft::apparatus, ft::armor, ft::book, ft::ingredient, ft::key, ft::leveled_item, ft::light, ft::misc_item, ft::note, ft::potion, ft::scroll, ft::soul_gem, ft::weapon>;

      struct _flag_definition {
         const char*    name  = nullptr;
         uint32_t       value = 0;
         form_type_list base_form_types; // empty = matches all base form types
      };

      // Master list of all REFR flags. Not used by code; rather, used to generate 
      // sub-lists at compile-time.
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
         _flag_definition{ "no_ai_acquire",           wrapped_type::form_flag::no_ai_acquire,            concatenate_form_type_lists(all_item_types, std::array{ ft::actor, ft::container }) },
         //_flag_definition{ "filter",                 wrapped_type::form_flag::filter },       // expose this only once we're sure what it even friggin' does
         //_flag_definition{ "bounding_box",           wrapped_type::form_flag::bounding_box }, // expose this only once we're sure what it even friggin' does
         _flag_definition{ "reflected_by_cell_water", wrapped_type::form_flag::reflected_by_auto_water },
         _flag_definition{ "dont_havok_settle",       wrapped_type::form_flag::dont_havok_settle,        concatenate_form_type_lists(all_item_types, std::array{ ft::actor, ft::activator, ft::addon_node, ft::door, ft::movable_static, ft::statik, ft::tree }) },
         _flag_definition{ "no_respawn",              wrapped_type::form_flag::no_respawn,               concatenate_form_type_lists(all_item_types, std::array{ ft::activator, ft::addon_node, ft::door, ft::movable_static, ft::statik, ft::tree }) },
         _flag_definition{ "ground",                  wrapped_type::form_flag::ground },
         //_flag_definition{ "multibound",              wrapped_type::form_flag::multibound },  // expose this only once we're sure what it even friggin' does
      };
      static constexpr size_t type_specific_flag_count = ([]() {
         size_t i = 0;
         for (auto& e : refr_flags)
            if (!e.base_form_types.empty())
               ++i;
         return i;
      })();

      // Auto-generated sub-list of just the non-form-type-specific flags.
      constexpr auto global_flags = ([]() {
         constexpr size_t size = refr_flags.size();
         std::array<_flag_definition, size - type_specific_flag_count> out = {};
         //
         size_t j = 0;
         for (size_t i = 0; i < size; ++i) {
            if (refr_flags[i].base_form_types.empty()) {
               out[j] = refr_flags[i];
               ++j;
            }
         }
         return out;
      })();
      // Auto-generated sub-list of just the form-type-specific flags.
      constexpr auto type_specific_flags = ([]() {
         constexpr size_t size = refr_flags.size();
         std::array<_flag_definition, type_specific_flag_count> out = {};
         //
         size_t j = 0;
         for (size_t i = 0; i < size; ++i) {
            if (!refr_flags[i].base_form_types.empty()) {
               out[j] = refr_flags[i];
               ++j;
            }
         }
         return out;
      })();
      static_assert(global_flags.size()        < refr_flags.size()); // basic correctness checks
      static_assert(type_specific_flags.size() < refr_flags.size());

      // Flags-mask consisting of all flags that can be type-specific. Used to quickly 
      // decide which of the above two sub-lists to search.
      static constexpr uint32_t all_type_specific_flags = ([]() {
         uint32_t x = 0;
         for (auto& e : type_specific_flags)
            x |= e.value;
         return x;
      })();

      bool flag_is_valid_for_form(uint32_t flag, const dovah::form_stub* refr) noexcept {
         if (_helpers::all_type_specific_flags & flag) {
            auto* base = dovah::form_stub_helpers::get_base_form(refr);
            auto  type = base->formType;
            for (auto& def : _helpers::type_specific_flags) {
               if (def.value != flag)
                  continue;
               if (def.base_form_types.contains(type))
                  return true;
            }
            // Fall through. There is at least one flag, 0x40000000, whose meaning is "no respawn" for lots of dynamic objects but "ground" for everything else
         }
         for (auto& def : _helpers::global_flags)
            if (def.value == flag)
               return true;
         return false;
      }
   }

   namespace _getters {
      int _flag(lua_State* L) { // generic getter for any form flag; relies on an upvalue to indicate the flag; references a list near the top of this file
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         if (!self.stub)
            return 0;
         uint32_t flag;
         {
            constexpr int index_flag = lua_upvalueindex(1);
            assert(lua_type(L, index_flag) == LUA_TNUMBER);
            flag = lua_tointeger(L, index_flag);
            assert(flag && "The flag should not be zero. Storage/conversion error?");
         }
         if (!_helpers::flag_is_valid_for_form(flag, self.stub))
            return 0;
         auto flags = self.stub->get_record_flags();
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
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.stub)
            return 0;
         uint32_t flag;
         {
            constexpr int index_flag = lua_upvalueindex(1);
            assert(lua_type(L, index_flag) == LUA_TNUMBER);
            flag = lua_tointeger(L, index_flag);
            assert(flag && "The flag should not be zero. Storage/conversion error?");
         }
         if (!_helpers::flag_is_valid_for_form(flag, self.stub))
            cobb::lua::error(L, "this flag is not available for references whose base forms are of this type");
         self.before_edit();
         self.stub->edit_record_flags(flag, lua_toboolean(L, 2));
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
      { "position",  &_setters::position },
      { "rotation",  &_setters::rotation },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      //int index_class       = lua_absindex(L, -3);
      int index_getter_list = lua_absindex(L, -2);
      int index_setter_list = lua_absindex(L, -1);
      //
      for (auto& def : _helpers::refr_flags) {
         lua_pushinteger(L, def.value);
         lua_pushcclosure(L, &_getters::_flag, 1);
         lua_setfield(L, index_getter_list, def.name);
         //
         lua_pushinteger(L, def.value);
         lua_pushcclosure(L, &_setters::_flag, 1);
         lua_setfield(L, index_setter_list, def.name);
      }
   }
}