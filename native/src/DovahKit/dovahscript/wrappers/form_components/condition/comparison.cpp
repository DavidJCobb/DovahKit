#include "./comparison.h"
#include "helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_if_form_cannot_be_edited.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "../condition.h"
#include "dovahscript/api_helpers/conditions/push_pull_comparison_operand.h"
#include "dovahscript/api_helpers/conditions/push_pull_comparison_operator.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::condition_comparison;
   using working_type = dovah::loaded_forms::components::conditions::working_condition;

   namespace _getters {
      int operator_(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         auto& comparison = data->get_comparison();
         api_helpers::conditions::push_comparison_operator(L, comparison.op);
         return 1;
      }
      int operand(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         auto& operand = data->get_comparison().operand;
         api_helpers::conditions::push_comparison_operand(L, operand);
         return 1;
      }
   }
   namespace _setters {
      int operator_(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         
         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         if (!wrappers::condition::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");

         auto result = api_helpers::conditions::pull_comparison_operator(L, 2);
         if (result.has_value()) {
            self.before_edit();
            {
               working_type working(*data);
               working.comparison.op = result.value();
               data->commit(*form, working);
            }
            self.after_edit();
         } else {
            cobb::lua::argerror(L, 2, result.error().data());
         }
         return 0;
      }
      int operand(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         api_helpers::fail_if_form_cannot_be_edited(L, self.stub);
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         auto* data = wrappers::condition::unwrap(self);
         if (data == nullptr)
            cobb::lua::error(L, "condition_comparison wrapper has no underlying object (deleted?)");
         if (!wrappers::condition::is_not_locked(self))
            cobb::lua::error(L, "this is a locked condition on a topic info; it cannot be edited");
         
         auto result = api_helpers::conditions::pull_comparison_operand(L, 2);
         if (result.has_value()) {
            self.before_edit();
            {
               working_type working(*data);
               working.comparison.operand = result.value();
               data->commit(*form, working);
            }
            self.after_edit();
         } else {
            cobb::lua::argerror(L, 2, result.error().data());
         }
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "operator", &_getters::operator_ },
      { "operand",  &_getters::operand },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "operator", &_setters::operator_ },
      { "operand",  &_setters::operand },
   };
}