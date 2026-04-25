#include "./collection_conditions.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/components/conditions.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/utils/form_component_accessors/condition_list.h"
#include "dovah/forms/Quest.h"
#include "dovah/forms/TopicInfo.h"
#include "./condition.h"

#include "dovah/data/conditions/all_function_info.h"
#include "dovah/forms/components/conditions/working_condition.h"

namespace {
   constexpr std::string_view collection_metatable_key = "collection<dovah.classes.condition>";
}

namespace dovahscript::wrappers::collections {
   extern dovah::loaded_forms::components::condition_list* unwrap_condition_list(wrapper& self) {
      return unwrap_condition_list_and_index(self).first;
   }
   extern std::pair<dovah::loaded_forms::components::condition_list*, size_t> unwrap_condition_list_and_index(wrapper& w, size_t ctda_index) {
      auto* form = w.get_loaded_form_data<dovah::loaded_forms::Form>();
      if (!form)
         return {};

      if (form->stub.form_type == dovah::form_type::quest) {
         auto* quest = static_cast<dovah::loaded_forms::Quest*>(form);
         for (const auto& part : w.parts) {
            if (part.signature == wrapper_part_types::condition_list_quest_dialogue)
               return { &quest->conditions.dialogue, part.index };
            if (part.signature == wrapper_part_types::condition_list_quest_events)
               return { &quest->conditions.event, part.index };
         }
         return {};
      }
      if (form->stub.form_type == dovah::form_type::topic_info) {
         //
         // TopicInfos have a bifurcated condition list.
         //
         assert(form == dynamic_cast<dovah::loaded_forms::TopicInfo*>(form));
         auto*  topic_info   = static_cast<dovah::loaded_forms::TopicInfo*>(form);
         size_t locked_count = topic_info->conditions.locked.size();
         if (ctda_index < locked_count)
            return { &topic_info->conditions.locked, ctda_index };
         return { &topic_info->conditions.normal, ctda_index - locked_count };
      }

      size_t index = 0;
      for (size_t i = 0; i < w.parts.size(); ++i) {
         if (w.parts[i].signature == wrapper_part_types::condition_list) {
            index = w.parts[i].index;
            break;
         }
      }
      return { dovah::utils::form_component_accessors::condition_list(*form), index };
   }
   extern std::pair<dovah::loaded_forms::components::condition_list*, size_t> unwrap_condition_list_and_index(wrapper& w) {
      size_t ctda_index = 0;
      if (w.stub && w.stub->form_type == dovah::form_type::topic_info) {
         for (const auto& part : w.parts) {
            if (part.signature == wrapper_part_types::condition_list) {
               ctda_index = part.index;
               break;
            }
         }
      }
      return unwrap_condition_list_and_index(w, ctda_index);
   }
}

#include "dovahscript/api_helpers/native_lists/member_function_spec.h"
#include "dovahscript/api_helpers/native_lists/common/pull_collection.h"
#include "dovahscript/api_helpers/native_lists/all_definition_params.h"

namespace {
   using namespace dovahscript;

   struct member_function_spec : public api_helpers::native_lists::member_function_spec {
      using collection_wrapped_type = dovah::loaded_forms::components::condition_list;
      using value_wrapper_type      = wrappers::condition;
      using value_stored_type       = value_wrapper_type::wrapped_type;
      using value_working_type      = dovah::loaded_forms::components::conditions::working_condition;

      static constexpr const bool allow_insertions_past_end = false;

      static constexpr const auto pull_collection = &api_helpers::native_lists::common::pull_collection<collection_metatable_key>;

      static std::pair<collection_wrapped_type*, collection_wrapped_type*> unwrap_collection(wrapper& self) {
         if (self.stub && self.stub->form_type == dovah::form_type::topic_info) {
            auto* form = self.get_loaded_form_data<dovah::loaded_forms::TopicInfo>();
            if (!form)
               return { nullptr, nullptr };
            return { &form->conditions.locked, &form->conditions.normal };
         }
         return { nullptr, dovahscript::wrappers::collections::unwrap_condition_list(self) };
      }

      static value_working_type pull_value(lua_State* L, int pos) {
         auto& self    = pull_collection(L);
         auto  context = value_wrapper_type::context_of(self);
         return value_wrapper_type::pull_from_table(L, pos, context);
      }
      static value_working_type initialize_value() {
         //
         // The constructors for `working_condition` and `condition` are actually pretty rancid. 
         // Conditions are not default-constructed with valid data, so if we don't set valid data 
         // by hand here, then the program will choke.
         //
         value_working_type working;
         working.set_function_id(dovah::conditions::function_id_by_name("GetIsID"));
         working.run_on.type = dovah::conditions::run_on_type::subject;
         working.reset_parameters();
         working.comparison.op = dovah::conditions::comparison_operator::equal;
         working.comparison.operand = 1.0F;
         return working;
      }
      static int push_value(lua_State* L, wrapper& collection, size_t zero_based_item_index) {
         assert(collection.is_collection);
         assert(collection.stub);
         wrapper out = collection;
         out.into_collection(zero_based_item_index);
         return core::subsystems::userdata::get().push(L, out, value_wrapper_type::metatable_key);
      }
      static void store_value(const value_working_type& src, value_stored_type& dst, dovah::loaded_forms::Form& dst_form) {
         dst.commit(dst_form, src);
      }
   };
}

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params condition_list = api_helpers::native_lists::all_definition_params<collection_metatable_key, member_function_spec>;
}