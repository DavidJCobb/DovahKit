#include "./collection_conditions.h"
#include "helpers/lua/error.h"
#include "helpers/lua/warning.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/conditions.h"
#include "dovah/utils/form_component_accessors/condition_list.h"
#include "dovah/forms/Quest.h"
#include "dovah/forms/TopicInfo.h"
#include "./condition.h"
#include "../form/form.h"

#include "dovah/data/conditions/all_function_info.h"
#include "dovah/forms/components/conditions/working_condition.h"

namespace {
   constexpr const char* collection_metatable_key = "collection<dovah.classes.condition>";
}

namespace {
   using wrapped_type = dovah::loaded_forms::components::condition_list;
}

namespace dovahscript::wrappers::collections {
   extern dovah::loaded_forms::components::condition_list* unwrap_condition_list(wrapper& self) {
      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      if (!form)
         return nullptr;

      if (form->stub.form_type == dovah::form_type::quest) {
         auto* quest = static_cast<dovah::loaded_forms::Quest*>(form);
         for (const auto& part : self.parts) {
            if (part.signature == wrapper_part_types::condition_list_quest_dialogue)
               return &quest->conditions.dialogue;
            if (part.signature == wrapper_part_types::condition_list_quest_events)
               return &quest->conditions.event;
         }
         return nullptr;
      }
      if (form->stub.form_type == dovah::form_type::topic_info) {
         //
         // TopicInfos have a bifurcated condition list.
         //
         size_t ctda_index = 0;
         for (const auto& part : self.parts) {
            if (part.signature == wrapper_part_types::condition_list) {
               ctda_index = part.index;
               break;
            }
         }
         assert(form == dynamic_cast<dovah::loaded_forms::TopicInfo*>(form));
         auto* topic_info = static_cast<dovah::loaded_forms::TopicInfo*>(form);
         if (ctda_index < topic_info->conditions.locked.size())
            return &topic_info->conditions.locked;
         return &topic_info->conditions.normal;
      }

      return dovah::utils::form_component_accessors::condition_list(*form);
   }
}

namespace {
   using namespace dovahscript;
   
   wrapper& get_collection_wrapper(lua_State* L) {
      auto* self = (wrapper*) classes::cast_to_class(L, 1, collection_metatable_key);
      if (self == nullptr) {
         cobb::lua::error(L, "function called with bad self (expected %s)", collection_metatable_key);
      }
      return *self;
   }
   wrapped_type* get_wrapped_object(wrapper& self) {
      return dovahscript::wrappers::collections::unwrap_condition_list(self);
   }
   
   int get_collection_length(lua_State* L) {
      auto& self = get_collection_wrapper(L);
      if (self.stub && self.stub->form_type == dovah::form_type::topic_info) {
         //
         // TopicInfos have a bifurcated condition list.
         //
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::TopicInfo>();
         if (!form)
            return 0;
         lua_pushinteger(L, form->conditions.locked.size() + form->conditions.normal.size());
         return 1;
      }
      auto* list = get_wrapped_object(self);
      if (!list)
         return 0;
      lua_pushinteger(L, list->size());
      return 1;
   }
   int lookup_item_by_index(lua_State* L) {
      auto& self = get_collection_wrapper(L);

      auto i = lua_tointeger(L, 2);
      if (i <= 0)
         return 0;
      //
      // Verify wrapper, and bounds-check.
      //
      if (self.stub && self.stub->form_type == dovah::form_type::topic_info) {
         //
         // TopicInfos have a bifurcated condition list.
         //
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::TopicInfo>();
         if (!form)
            return 0;
         if (i > form->conditions.locked.size() + form->conditions.normal.size())
            return 0;
      } else {
         auto* list = get_wrapped_object(self);
         if (!list)
            return 0;
         if (i > list->size())
            return 0;
      }
      --i;
      wrapper out = self;
      assert(out.is_collection);
      assert(out.parts[0].signature == wrapper_part_types::condition_list);
      out.into_collection(i);
      return core::subsystems::userdata::get().push(L, out, wrappers::condition::metatable_key);
   }
   int member_function_insert(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();

      std::optional<lua_Integer> requested_index;

      int pos_value = 2;
      if (lua_gettop(L) >= 3) {
         pos_value = 3;
         luaL_argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
         requested_index = lua_tointeger(L, 2);
         if (requested_index.value() < 1) {
            cobb::lua::error(L, "indices below 1, such as %d, are not allowed", requested_index.value());
         }
      }
      
      // Find the correct list to modify.
      wrapped_type* list_ptr = nullptr;
      auto&         self     = get_collection_wrapper(L);
      if (self.stub && self.stub->form_type == dovah::form_type::topic_info) {
         //
         // TopicInfos have a bifurcated condition list.
         //
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::TopicInfo>();
         if (!form)
            return 0;

         if (requested_index.has_value()) {
            size_t no_no_threshold = form->conditions.locked.size();
            if (requested_index.value() <= no_no_threshold)
               cobb::lua::argerror(L, 2, "requested index is inside of this TopicInfo's locked conditions");
            requested_index.value() -= no_no_threshold;
         }

         list_ptr = &form->conditions.normal;
      } else {
         list_ptr = get_wrapped_object(self);
      }
      if (!list_ptr)
         return 0;

      if (!lua_isnoneornil(L, pos_value)) {
         //
         // TODO: Allow inserting a properly-set table or string, as a condition.
         //
      }
      
      auto&  list = *list_ptr;
      auto   size = list.size();
      size_t i    = size;
      if (requested_index.has_value()) {
         i = requested_index.value();
         --i;
      }

      self.before_edit();
      if (i > size) {
         cobb::lua::error(L, "cannot insert past the end of the list");
      } else {
         list.emplace(list.begin() + i);
         //
         // So, uh, the backend's `condition_list` type is kind of reprehensible. 
         // The `emplace` function default-constructs a condition, but defaulted 
         // conditions are invalid, so this'll choke the literal instant you try 
         // to edit anything. We need to force the condition to something valid.
         //
         dovah::loaded_forms::components::conditions::working_condition working;
         working.set_function_id(dovah::conditions::function_id_by_name("GetIsID"));
         working.run_on.type = dovah::conditions::run_on_type::subject;
         working.reset_parameters();
         working.comparison.op = dovah::conditions::comparison_operator::equal;
         working.comparison.operand = 1.0F;
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
         list[i].commit(*form, working);
      }
      core::subsystems::userdata::get().insert_into_sequential_collection(self, lua_tointeger(L, 2) - 1);
      self.after_edit();

      wrapper out = self;
      assert(out.is_collection);
      if (self.stub && self.stub->form_type == dovah::form_type::topic_info) {
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::TopicInfo>();
         assert(!!form);
         out.into_collection(form->conditions.locked.size() + i);
      } else {
         out.into_collection(i);
      }
      return core::subsystems::userdata::get().push(L, out, wrappers::condition::metatable_key);
   }
   int member_function_remove(lua_State* L) {
      core::subsystems::permissions::verify_form_write_permissions();

      cobb::lua::argcheck(L, lua_isinteger(L, 2), 2, "expected an integer index");
      auto i = lua_tointeger(L, 2);
      if (i <= 0)
         return 0;
      --i;

      // Find the correct list to modify.
      wrapped_type* list_ptr = nullptr;
      auto&         self     = get_collection_wrapper(L);
      if (self.stub && self.stub->form_type == dovah::form_type::topic_info) {
         //
         // TopicInfos have a bifurcated condition list.
         //
         auto* form = self.get_loaded_form_data<dovah::loaded_forms::TopicInfo>();
         if (!form)
            return 0;

         size_t no_no_threshold = form->conditions.locked.size();
         if (i < no_no_threshold)
            cobb::lua::argerror(L, 2, "requested index is inside of this TopicInfo's locked conditions");
         i -= no_no_threshold;

         list_ptr = &form->conditions.normal;
      } else {
         list_ptr = get_wrapped_object(self);
      }
      if (!list_ptr)
         return 0;
      
      auto& list = *list_ptr;
      if (i >= list.size())
         return 0;
      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();
      self.before_edit();
      list[i].clear(*form);
      list.erase(list.begin() + i);
      self.after_edit();
      {
         wrapper to_remove = self;
         to_remove.into_collection(i);
         core::subsystems::userdata::get().remove_from_sequential_collection(to_remove);
      }
      return 0;
   }
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params condition_list = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
      .member_function_insert = &member_function_insert,
      .member_function_remove = &member_function_remove,
      //.set_item               = &set_item,
   };
}