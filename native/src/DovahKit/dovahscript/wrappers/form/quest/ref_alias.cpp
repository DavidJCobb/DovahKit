#include "./ref_alias.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/form_stub.h"
#include "dovah/forms/ActorBase.h"
#include "dovah/forms/Quest.h"
#include "dovah/utils/story_event_member_id.h"
#include "../quest.h"
#include "./ref_alias_fill_params_at_location.h"
#include "./ref_alias_fill_params_create.h"
#include "./ref_alias_fill_params_linked_ref.h"

//
// MISSING APIS:
//  - Reference aliases
//     - Added Factions
//     - Added Inventory
//     - Added Keywords
//     - Added Packages
//     - Added Spells
//     - Package override lists
//     - Additional voicetypes
//
#include "../../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_script_apis, "The Lua API for reference aliases is incomplete.");

namespace {
   using namespace dovahscript;
   using cls = wrappers::quest_ref_alias;

   namespace alias_fill_params {
      using namespace dovah::loaded_forms::structs::alias_fill_params;
   }
   
   namespace _getters {
      template<uint32_t Flag>
      int _flag(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
         lua_pushboolean(L, !!(alias->flags & Flag));
         return 1;
      }

      int display_name(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
         return push_native_object(alias->display_name.get_form_stub());
      }
      int fill(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            return 0;

         auto& fill = alias->fill_params;
         if (auto* casted = std::get_if<alias_fill_params::ref::at_location_alias>(&fill)) {
            auto out = self;
            out.append_part(wrapper_part_types::quest_alias_fill_params);
            return core::subsystems::userdata::get().push(L, out, wrappers::quest_ref_alias_fill_params_at_location::metatable_key);
         }
         if (auto* casted = std::get_if<alias_fill_params::copy_external_alias>(&fill)) {
            return cls::wrap(L, casted->quest.get_form_stub(), casted->alias);
         }
         if (auto* casted = std::get_if<alias_fill_params::ref::create>(&fill)) {
            auto out = self;
            out.append_part(wrapper_part_types::quest_alias_fill_params);
            return core::subsystems::userdata::get().push(L, out, wrappers::quest_ref_alias_fill_params_create::metatable_key);
         }
         if (auto* casted = std::get_if<alias_fill_params::ref::find_from_event>(&fill)) {
            if (casted->member == 0) {
               lua_pushnil(L);
               return 1;
            }
            if (casted->member & 0xFFFF0000)
               return 0;

            dovah::story_event_member_id mid{ (uint16_t)casted->member };
            lua_pushlstring(L, mid.to_string().data(), 2);
            return 1;
         }
         if (auto* casted = std::get_if<alias_fill_params::ref::find_in_loaded_area>(&fill)) {
            if (casted->closest) {
               lua_pushstring(L, "find closest in loaded area");
            } else {
               lua_pushstring(L, "find random in loaded area");
            }
            return 1;
         }
         if (auto* casted = std::get_if<alias_fill_params::ref::find_near_alias>(&fill)) {
            auto out = self;
            out.append_part(wrapper_part_types::quest_alias_fill_params);
            return core::subsystems::userdata::get().push(L, out, wrappers::quest_ref_alias_fill_params_linked_ref::metatable_key);
         }
         if (auto* casted = std::get_if<alias_fill_params::ref::preassigned>(&fill)) {
            return push_native_object(casted->ref);
         }
         if (auto* casted = std::get_if<alias_fill_params::ref::unique_actor>(&fill)) {
            return push_native_object(casted->actor_base);
         }
         return 0;
      }
   }
   namespace _setters {
      template<uint32_t Flag>
      int _flag(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         self.before_edit();
         cobb::edit_bit(alias->flags, Flag, lua_toboolean(L, 2));
         self.after_edit();
         return 1;
      }

      template<typename T>
      T& _get_or_emplace_fill_params(wrapper& self, lua_State* L, cls::wrapped_type& alias) {
         if (std::holds_alternative<T>(alias.fill_params)) {
            return std::get<T>(alias.fill_params);
         }

         auto& form = alias.owner;
         if (auto* casted = std::get_if<alias_fill_params::ref::at_location_alias>(&alias.fill_params)) {
            casted->loc_ref_type.set(form, nullptr);
         } else if (auto* casted = std::get_if<alias_fill_params::ref::create>(&alias.fill_params)) {
            casted->base_form.set(form, nullptr);
         } else if (auto* casted = std::get_if<alias_fill_params::ref::preassigned>(&alias.fill_params)) {
            casted->ref.set(form, nullptr);
         } else if (auto* casted = std::get_if<alias_fill_params::ref::unique_actor>(&alias.fill_params)) {
            casted->actor_base.set(form, nullptr);
         }

         {
            auto killed = self;
            killed.append_part(wrapper_part_types::quest_alias_fill_params);
            core::subsystems::userdata::get().destroy(killed);
         }

         return alias.fill_params.emplace<T>();
      }

      template<typename Wrapper>
      void _handle_fill_rewrap(
         wrapper& self,
         cls::wrapped_type& alias,
         lua_State* L,
         int argpos
      ) {
         if (!Wrapper::validate_table(alias, L, argpos)) {
            cobb::lua::argerror(L, 2, "table contents are not valid");
         }
         self.before_edit();
         {
            auto& fill = _get_or_emplace_fill_params<typename Wrapper::wrapped_type>(self, L, alias);
            Wrapper::overwrite_with_table(alias, fill, L, argpos);
         }
         self.after_edit();
      }

      int display_name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self  = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
         auto* value = pull_form_stub_argument(L, 2, dovah::form_type::message);
         self.before_edit();
         alias->display_name.set(*self.stub->form, value);
         self.after_edit();
         return 0;
      }
      int fill(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<wrappers::quest_alias>(L);
         auto* alias = cls::unwrap(self);
         if (alias == nullptr)
            cobb::lua::error(L, "alias wrapper has no underlying object (deleted?)");
         auto& quest = alias->owner;

         if (auto* form_wrap = wrapper_from_stack<wrappers::form>(L, 2)) {
            cobb::lua::argcheck(L, !!form_wrap->stub, 2, "zombie argument");
            auto* stub = form_wrap->stub;
            if (stub->form_type == dovah::form_type::actor_base) {
               {  // require unique actor
                  auto loaded_actor = stub->load().ptr_cast<dovah::loaded_forms::ActorBase>();
                  cobb::lua::argcheck(L, !!loaded_actor, 2, "aliases can only fill from actor bases for unique actors");
                  cobb::lua::argcheck(L, !!(loaded_actor->actor_flags & dovah::loaded_forms::ActorBase::actor_flag::unique), 2, "aliases can only fill from actor bases for unique actors");
               }
               self.before_edit();
               _get_or_emplace_fill_params<alias_fill_params::ref::unique_actor>(self, L, *alias).actor_base.set(quest, stub);
               self.after_edit();
               return 0;
            }
            if (dovah::form_type_is_reference(stub->form_type)) {
               self.before_edit();
               _get_or_emplace_fill_params<alias_fill_params::ref::preassigned>(self, L, *alias).ref.set(quest, stub);
               self.after_edit();
               return 0;
            }
            cobb::lua::argerror(L, 2, "forms of this type cannot be used to fill a reference alias");
         }
         if (auto* alias_wrap = wrapper_from_stack<wrappers::quest_alias>(L, 2)) {
            auto* from_alias = cls::unwrap(*alias_wrap);
            cobb::lua::argcheck(L, !!from_alias, 2, "zombie argument");
            cobb::lua::argcheck(L, from_alias != alias, 2, "an alias cannot fill itself");
            if (from_alias->type == cls::wrapped_type::alias_type::location) {
               cobb::lua::argerror(L, 2, "to fill from a location alias, pass a table: { type = \"at location\", alias = a, loc_ref_type = b }");
            } else if (from_alias->type == cls::wrapped_type::alias_type::reference) {
               if (&from_alias->owner.stub == &alias->owner.stub)
                  cobb::lua::argerror(L, 2, "cannot fill a reference alias with another reference alias from the same quest");
               self.before_edit();
               {
                  auto& dst = _get_or_emplace_fill_params<alias_fill_params::copy_external_alias>(self, L, *alias);
                  dst.quest.set(quest, &from_alias->owner.stub);
                  dst.alias = from_alias->id;
               }
               self.after_edit();
               return 0;
            }
            cobb::lua::argerror(L, 2, "internal error (can't tell what type of alias you passed in?)");
         }
         if (lua_istable(L, 2) || lua_isuserdata(L, 2)) {
            std::string_view type;
            {
               lua_getfield(L, 2, "type");
               cobb::lua::argcheck(L, lua_isstring(L, -1), 2, "table arguments must have a \"type\" property whose value is a string");
               type = lua_tostring(L, -1);
               lua_pop(L, 1);
            }
            if (type == "at location") {
               using fill_wrapper_type = wrappers::quest_ref_alias_fill_params_at_location;
               _handle_fill_rewrap<fill_wrapper_type>(self, *alias, L, 2);
            } else if (type == "create") {
               using fill_wrapper_type = wrappers::quest_ref_alias_fill_params_create;
               _handle_fill_rewrap<fill_wrapper_type>(self, *alias, L, 2);
            } else if (type == "linked ref of") {
               using fill_wrapper_type = wrappers::quest_ref_alias_fill_params_linked_ref;
               _handle_fill_rewrap<fill_wrapper_type>(self, *alias, L, 2);
            } else {
               cobb::lua::argerror(L, 2, "unrecognized type");
            }
            return 0;
         }
         if (lua_isstring(L, 2)) {
            std::string_view value = lua_tostring(L, 2);
            if (value == "find closest in loaded area") {
               self.before_edit();
               _get_or_emplace_fill_params<alias_fill_params::ref::find_in_loaded_area>(self, L, *alias).closest = true;
               self.after_edit();
            } else if (value == "find random in loaded area") {
               self.before_edit();
               _get_or_emplace_fill_params<alias_fill_params::ref::find_in_loaded_area>(self, L, *alias).closest = false;
               self.after_edit();
            } else {
               if (value.size() == 2) {
                  dovah::story_event_member_id member;
                  member.from_string(value);

                  self.before_edit();
                  {
                     auto& dst = _get_or_emplace_fill_params<alias_fill_params::ref::find_from_event>(self, L, *alias);
                     dst.code   = quest.event;
                     dst.member = (uint16_t)member;
                  }
                  self.after_edit();
                  return 0;
               }
               cobb::lua::argerror(L, 2, "value is not a recognized string, nor a two-character event data member");
            }
            return 0;
         }
         cobb::lua::argerror(L, 2, "invalid argument");
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = {
      #pragma region Flags
         { "allow_dead",              &_getters::_flag<wrapped_type::flag::allow_dead> },
         { "allow_destroyed",         &_getters::_flag<wrapped_type::flag::allow_destroyed> },
         { "allow_disabled",          &_getters::_flag<wrapped_type::flag::allow_disabled> },
         { "clear_name_when_removed", &_getters::_flag<wrapped_type::flag::clear_name_when_removed> },
         { "is_quest_object",         &_getters::_flag<wrapped_type::flag::quest_object> },
         { "make_essential",          &_getters::_flag<wrapped_type::flag::make_essential> },
         { "make_protected",          &_getters::_flag<wrapped_type::flag::make_protected> },
         { "stores_text",             &_getters::_flag<wrapped_type::flag::stores_text> },
      #pragma endregion
      { "display_name", &_getters::display_name },
      { "fill",         &_getters::fill },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      #pragma region Flags
         { "allow_dead",              &_setters::_flag<wrapped_type::flag::allow_dead> },
         { "allow_destroyed",         &_setters::_flag<wrapped_type::flag::allow_destroyed> },
         { "allow_disabled",          &_setters::_flag<wrapped_type::flag::allow_disabled> },
         { "clear_name_when_removed", &_setters::_flag<wrapped_type::flag::clear_name_when_removed> },
         { "is_quest_object",         &_setters::_flag<wrapped_type::flag::quest_object> },
         { "make_essential",          &_setters::_flag<wrapped_type::flag::make_essential> },
         { "make_protected",          &_setters::_flag<wrapped_type::flag::make_protected> },
         { "stores_text",             &_setters::_flag<wrapped_type::flag::stores_text> },
      #pragma endregion
      { "display_name", &_setters::display_name },
      { "fill",         &_setters::fill },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) {
      define_wrapper_metatable<quest_ref_alias_fill_params_at_location>(L);
      define_wrapper_metatable<quest_ref_alias_fill_params_create>(L);
      define_wrapper_metatable<quest_ref_alias_fill_params_linked_ref>(L);
   }

   /*static*/ cls::wrapped_type* quest_ref_alias::unwrap(wrapper& w) {
      auto* alias = quest_alias::unwrap(w);
      if (alias && alias->type != dovah::loaded_forms::Alias::alias_type::reference)
         return nullptr;
      return (wrapped_type*)alias;
   }
}