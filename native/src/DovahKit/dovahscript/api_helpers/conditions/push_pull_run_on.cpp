#include "./push_pull_run_on.h"
#include "lua.h"
#include "dovah/data/conditions/run_on_type.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/form_types.h"
#include "dovah/forms/Quest.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrappers/form/form.h"
#include "dovahscript/wrappers/form/quest/alias.h"
#include "editor/core.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<working_run_on_params, std::string_view> pull_run_on(lua_State* L, int pos, const context_type& context) {
      if (lua_isstring(L, pos)) {
         std::string_view arg = lua_tostring(L, pos);
         if (arg == "combat target") {
            return working_run_on_params{ .type = dovah::conditions::run_on_type::combat_target };
         }
         if (arg == "linked ref") {
            return working_run_on_params{ .type = dovah::conditions::run_on_type::linked_ref };
         }
         if (arg == "player") {
            auto& editor = DovahKitCore::get();
            if (!editor.has_data())
               return std::unexpected("cannot set the run-on type to the player because no data is loaded in the editor");
            auto* ref = editor.get_form(dovah::hardcoded_form_ids::PlayerRef);
            if (!ref)
               return std::unexpected("an internal error occurred: could not locate PlayerRef");
            return working_run_on_params{
               .type   = dovah::conditions::run_on_type::reference,
               .entity = ref
            };
         }
         if (arg == "subject") {
            return working_run_on_params{ .type = dovah::conditions::run_on_type::subject };
         }
         if (arg == "target") {
            return working_run_on_params{ .type = dovah::conditions::run_on_type::target };
         }
         return std::unexpected("unrecognized run-on type");
      }
      auto* alias_wrapper = wrapper_from_stack<wrappers::quest_alias>(L, pos);
      if (alias_wrapper) {
         auto* alias = wrappers::quest_alias::unwrap(*alias_wrapper);
         if (!alias)
            return std::unexpected("passed-in quest alias is missing (deleted?)");
         if (!context.quest)
            return std::unexpected("this condition has no owning quest, and so cannot be set to run on a quest alias");
         if (&alias->owner.stub != context.quest)
            return std::unexpected("the passed-in quest alias does not belong to this condition's owning quest");
         return working_run_on_params{
            .type   = dovah::conditions::run_on_type::quest_alias,
            .entity = alias->id
         };
      } else {
         auto* form_wrapper = wrapper_from_stack<wrappers::form>(L, pos);
         if (!form_wrapper)
            return std::unexpected("form or string expected");
         if (!form_wrapper->stub || !dovah::form_type_is_reference(form_wrapper->stub->form_type))
            return std::unexpected("the passed-in form is not a ref");
         return working_run_on_params{
            .type   = dovah::conditions::run_on_type::reference,
            .entity = form_wrapper->stub
         };
      }
      // 
      // TODO: in the future, handle package data and event data
      //
   }

   extern void push_run_on(lua_State* L, const condition_type::run_on_data& src, const context_type& context) {
      switch (src.type) {
         case dovah::conditions::run_on_type::combat_target:
            lua_pushstring(L, "combat target");
            return;
         case dovah::conditions::run_on_type::event_data:
            lua_pushnumber(L, src.index); // TODO: push an event-data object instead
            return;
         case dovah::conditions::run_on_type::linked_ref:
            lua_pushstring(L, "linked ref");
            return;
         case dovah::conditions::run_on_type::quest_alias:
            if (auto* quest = context.quest) {
               int c = wrappers::quest_alias::wrap(L, quest, src.index);
               if (c > 0) {
                  if (c > 1)
                     lua_pop(L, c - 1);
                  return;
               }
            }
            break;
         case dovah::conditions::run_on_type::package_data:
            lua_pushnumber(L, src.index); // TODO: push a package-data object instead
            return;
         case dovah::conditions::run_on_type::reference:
            {
               auto* stub = src.reference.get_form_stub();
               if (stub && stub->formID == dovah::hardcoded_form_ids::PlayerRef) {
                  lua_pushstring(L, "player");
                  return;
               }
               int c = push_native_object(src.reference);
               if (c > 0) {
                  if (c > 1)
                     lua_pop(L, c - 1);
                  return;
               }
            }
            break;
         case dovah::conditions::run_on_type::subject:
            lua_pushstring(L, "subject");
            return;
         case dovah::conditions::run_on_type::target:
            lua_pushstring(L, "target");
            return;
      }
      lua_pushnil(L);
      return;
   }
}