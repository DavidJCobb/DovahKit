#include "./working_condition.h"
#include <cassert>
#include <stdexcept>
#include "../../../data/conditions/all_function_info.h"
#include "../conditions.h"

namespace dovah::loaded_forms::components::conditions {
   working_condition::working_condition(const condition& src) {
      using backend_condition_flags = condition::flag;
      using src_type = dovah::loaded_forms::components::condition;

      this->flags.or_linked               = src.get_flags() & backend_condition_flags::or_linked;
      this->flags.swap_subject_and_target = src.get_flags() & backend_condition_flags::swap_subject_and_target;
      if (src.get_flags() & backend_condition_flags::use_aliases) {
         this->override_types_with = parameter_type_override::alias;
      } else if (src.get_flags() & backend_condition_flags::use_package_data) {
         this->override_types_with = parameter_type_override::package_data;
      }

      this->function = src.get_function_id();

      auto* function_info = dovah::conditions::function_info_by_id(this->function);

      for (size_t i = 0; i < this->parameters.size(); ++i) {
         this->parameters[i] = src.get_parameter(i);
      }
      if (function_info && function_info->uses_event_data) {
         auto& src_param = src.get_event_parameters();
         auto& dst_param = this->event_parameters.emplace();
         dst_param.function = src_param.function;
         dst_param.member   = src_param.member;
         dst_param.form     = src_param.form.get_form_stub();
      }
      {  // Comparison
         auto& src_data = src.get_comparison();
         auto& dst_data = this->comparison;
         dst_data.op = src_data.op;
         if (std::holds_alternative<form_reference_t>(src_data.operand)) {
            dst_data.operand = std::get<form_reference_t>(src_data.operand).get_form_stub();
         } else {
            dst_data.operand = std::get<float>(src_data.operand);
         }
      }
      {  // Run On
         auto& src_data = src.get_run_on_data();
         auto& dst_data = this->run_on;
         dst_data.type = src_data.type;
         switch (dst_data.type) {
            case run_on_type::reference:
               dst_data.entity = src_data.reference.get_form_stub();
               break;
            case run_on_type::package_data:
            case run_on_type::quest_alias:
            case run_on_type::event_data:
               dst_data.entity = src_data.index;
               break;
         }
      }
   }

   bool working_condition::valid() const {
      auto* function_info = dovah::conditions::function_info_by_id(this->function);
      if (!function_info)
         return false;

      if (auto* operand_ptr = std::get_if<form_stub*>(&this->comparison.operand)) {
         const form_stub* operand_stub = *operand_ptr;
         if (operand_stub) {
            if (operand_stub->form_type != form_type::global)
               return false;
         }
      }

      switch (this->run_on.type) {
         case run_on_type::reference:
            if (!std::holds_alternative<form_stub*>(this->run_on.entity))
               return false;
            break;
         case run_on_type::combat_target:
         case run_on_type::linked_ref:
         case run_on_type::subject:
         case run_on_type::target:
            if (!std::holds_alternative<std::monostate>(this->run_on.entity))
               return false;
            break;
         case run_on_type::event_data:
         case run_on_type::package_data:
         case run_on_type::quest_alias:
            if (!std::holds_alternative<uint32_t>(this->run_on.entity))
               return false;
            break;
      }

      if (function_info->uses_event_data) {
         if (!this->event_parameters.has_value())
            return false;
         for (const auto& parameter : this->parameters)
            if (!std::holds_alternative<std::monostate>(parameter))
               return false;
      } else {
         if (this->event_parameters.has_value())
            return false;
         for (size_t i = 0; i < this->parameters.size(); ++i)
            if (!this->is_parameter_valid(i))
               return false;
      }

      return true;
   }
   bool working_condition::is_parameter_valid(size_t n) const {
      if (n >= this->parameters.size())
         return false;

      auto* function_info = dovah::conditions::function_info_by_id(this->function);
      if (!function_info)
         return false;

      if (function_info->uses_event_data) {
         if (!std::holds_alternative<std::monostate>(this->parameters[n]))
            return false;
         return true;
      }
      
      auto* typeinfo  = function_info->argument_types[n];
      auto& parameter = this->parameters[n];
      if (typeinfo == &dovah::conditions::parameter_types::None) {
         return std::holds_alternative<std::monostate>(parameter);
      }

      auto underlying = this->get_argument_underlying_type(n);
      switch (underlying) {
         using enum dovah::conditions::parameter_underlying_type;
         case alias:
         case int_unsigned:
         case package_data:
         case quest_stage:
            return std::holds_alternative<uint32_t>(parameter);
         case enumeration:
         case int_signed:
            return std::holds_alternative<int32_t>(parameter);
         case float32:
            return std::holds_alternative<float>(parameter);
         case character:
            return std::holds_alternative<char>(parameter);
         case string:
            return std::holds_alternative<std::string>(parameter);
         case form:
            return std::holds_alternative<form_stub*>(parameter);
      }
      return false;
   }

   const dovah::conditions::parameter_typeinfo* working_condition::get_argument_typeinfo(size_t index) const {
      if (index >= std::tuple_size_v<decltype(parameters)>)
         throw std::out_of_range("condition parameter index out of range");

      auto* function_info = dovah::conditions::function_info_by_id(this->function);
      if (!function_info)
         return nullptr;
      auto* arg_typeinfo = function_info->argument_types[index];
      if (arg_typeinfo->is_union()) {
         auto& union_info = arg_typeinfo->union_decider.value();
         assert(index != 0 && "No behavior defined for a condition function whose first argument type is a union!");
         assert(function_info->argument_types[index - 1] == union_info.decide_by);

         auto& prev = this->parameters[index - 1];
         if (!std::holds_alternative<int32_t>(prev))
            return nullptr;

         return (union_info.decider)(std::get<int32_t>(prev));
      }
      return arg_typeinfo;
   }
   const dovah::conditions::parameter_typeinfo* working_condition::get_effective_argument_typeinfo(size_t index) const noexcept {
      auto* typeinfo = this->get_argument_typeinfo(index);
      if (typeinfo->allow_type_overrides) {
         switch (this->override_types_with) {
            case parameter_type_override::alias:
               return &dovah::conditions::parameter_types::Alias;
            case parameter_type_override::package_data:
               return &dovah::conditions::parameter_types::PackageData;
         }
      }
      return typeinfo;
   }
   dovah::conditions::parameter_underlying_type working_condition::get_argument_underlying_type(size_t index) const noexcept {
      auto* typeinfo = this->get_argument_typeinfo(index);
      if (!typeinfo)
         return dovah::conditions::parameter_underlying_type::none;
      if (typeinfo->allow_type_overrides) {
         switch (this->override_types_with) {
            case parameter_type_override::alias:
               return dovah::conditions::parameter_underlying_type::alias;
            case parameter_type_override::package_data:
               return dovah::conditions::parameter_underlying_type::package_data;
         }
      }
      return typeinfo->underlying_type;
   }

   void working_condition::set_function_id(uint16_t id) {
      using typeinfo        = dovah::conditions::parameter_typeinfo;
      using underlying_type = dovah::conditions::parameter_underlying_type;

      if (id == this->function)
         return;

      std::array<const typeinfo*, 2> prior_types;
      for (size_t i = 0; i < this->parameters.size(); ++i)
         prior_types[i] = this->get_effective_argument_typeinfo(i);

      this->function = id;

      const auto* function_info = dovah::conditions::function_info_by_id(this->function);
      if (function_info->uses_event_data) {
         this->parameters = {};
         if (!this->event_parameters.has_value())
            this->event_parameters.emplace();
         return;
      }
      this->event_parameters = {};

      for (size_t i = 0; i < this->parameters.size(); ++i) {
         auto& parameter = this->parameters[i];

         const auto* prior_type = prior_types[i];
         const auto* after_type = this->get_effective_argument_typeinfo(i);
         if (prior_type == after_type)
            continue;

         bool invalid = true;
         switch (prior_type->underlying_type) {
            case underlying_type::float32:
               if (auto* casted = std::get_if<float>(&parameter)) {
                  switch (after_type->underlying_type) {
                     case underlying_type::int_signed:
                        invalid   = false;
                        parameter = (int32_t)*casted;
                        break;
                     case underlying_type::int_unsigned:
                        invalid   = false;
                        parameter = (uint32_t)*casted;
                        break;
                  }
               }
               break;
            case underlying_type::int_signed:
               if (auto* casted = std::get_if<int32_t>(&parameter)) {
                  switch (after_type->underlying_type) {
                     case underlying_type::float32:
                        invalid   = false;
                        parameter = (float)*casted;
                        break;
                     case underlying_type::int_unsigned:
                        invalid   = false;
                        parameter = (uint32_t)*casted;
                        break;
                  }
               }
               break;
            case underlying_type::int_unsigned:
               if (auto* casted = std::get_if<uint32_t>(&parameter)) {
                  switch (after_type->underlying_type) {
                     case underlying_type::float32:
                        invalid   = false;
                        parameter = (float)*casted;
                        break;
                     case underlying_type::int_signed:
                        invalid   = false;
                        parameter = (int32_t)*casted;
                        break;
                  }
               }
               break;
            case underlying_type::character:
               if (auto* casted = std::get_if<char>(&parameter)) {
                  invalid = !after_type->allows_char(*casted);
               }
               break;
            case underlying_type::enumeration:
               invalid = !std::holds_alternative<int32_t>(parameter);
               break;
            case underlying_type::form:
               if (auto* casted = std::get_if<form_stub*>(&parameter)) {
                  form_stub* stub = *casted;
                  if (stub && after_type->allows_form_type(stub->form_type)) {
                     invalid = false;
                  }
               }
               break;
         }
         if (invalid)
            this->reset_parameter(i);
      }
   }
   void working_condition::set_type_override(parameter_type_override d) {
      using underlying_type = dovah::conditions::parameter_underlying_type;

      if (this->override_types_with == d)
         return;

      std::array<underlying_type, 2> prior_types;
      for (size_t i = 0; i < this->parameters.size(); ++i)
         prior_types[i] = this->get_argument_underlying_type(i);

      this->override_types_with = d;

      for (size_t i = 0; i < this->parameters.size(); ++i) {
         auto prior_type = prior_types[i];
         auto after_type = this->get_argument_underlying_type(i);
         if (prior_type == after_type)
            continue;
         this->reset_parameter(i);
      }
   }
   void working_condition::reset_parameter(size_t i) {
      auto& parameter  = this->parameters[i];
      auto* typeinfo   = this->get_argument_typeinfo(i);
      auto  underlying = this->get_argument_underlying_type(i);
      switch (underlying) {
         using enum dovah::conditions::parameter_underlying_type;
         case none:
            parameter = {};
            break;
         case alias:
            parameter = (uint32_t)-1;
            break;
         case character:
            if (typeinfo->character_values.empty())
               parameter = 'L';
            else
               parameter = typeinfo->character_values[0];
            break;
         case enumeration:
            assert(typeinfo->enumeration_info.has_value());
            parameter = (int32_t)typeinfo->enumeration_info.value().members[0].value;
            break;
         case float32:
            parameter = (float)0.0F;
            break;
         case form:
            parameter = (form_stub*)nullptr;
            break;
         case int_signed:
            parameter = int32_t{};
            break;
         case int_unsigned:
            parameter = uint32_t{};
            break;
         case package_data:
            parameter = (uint32_t)-1;
            break;
         case quest_stage:
            parameter = uint32_t{};
            break;
         case string:
            parameter.emplace<std::string>();
            break;
      }
      //
      // If we're resetting the first argument, and the second argument is a union whose type 
      // is decided by the first argument, then reset the second argument too:
      //
      if (i == 0) {
         auto* function_info = dovah::conditions::function_info_by_id(this->function);
         if (function_info) {
            auto* arg_typeinfo = function_info->argument_types[1];
            if (arg_typeinfo && arg_typeinfo->is_union())
               this->reset_parameter(1);
         }
      }
   }
   void working_condition::reset_parameters() {
      auto* function_info = dovah::conditions::function_info_by_id(this->function);
      if (!function_info) {
         this->event_parameters = {};
         this->parameters = {};
         return;
      }
      if (function_info->uses_event_data) {
         this->event_parameters.emplace();
         this->parameters = {};
         return;
      }

      this->event_parameters = {};
      for (size_t i = 0; i < this->parameters.size(); ++i) {
         this->reset_parameter(i);
      }
   }
   void working_condition::reset_run_on_entity() {
      switch (this->run_on.type) {
         case run_on_type::combat_target:
         case run_on_type::linked_ref:
         case run_on_type::subject:
         case run_on_type::target:
            this->run_on.entity = {};
            break;
         case run_on_type::event_data:
         case run_on_type::package_data:
         case run_on_type::quest_alias:
            this->run_on.entity = (uint32_t) -1;
            break;
         case run_on_type::reference:
            this->run_on.entity = (dovah::form_stub*)nullptr;
            break;
      }
   }

   bool working_condition::refers_to_form(const dovah::form_stub* target) const noexcept {
      auto _eq = [target](const auto& variant) {
         return std::holds_alternative<dovah::form_stub*>(variant) && std::get<dovah::form_stub*>(variant) == target;
      };

      for (auto& param : this->parameters)
         if (_eq(param))
            return true;
      if (this->event_parameters.has_value())
         if (this->event_parameters.value().form == target)
            return true;
      if (_eq(this->comparison.operand))
         return true;
      if (_eq(this->run_on.entity))
         return true;

      return false;
   }

   bool working_condition::sever_outbound_references_to(const dovah::form_stub* target) noexcept {
      bool changes_made = false;

      auto _sever = [&changes_made, target](auto& variant) {
         if (!std::holds_alternative<dovah::form_stub*>(variant))
            return;
         auto& pointer = std::get<dovah::form_stub*>(variant);
         if (pointer != target)
            return;
         changes_made = true;
         pointer      = nullptr;
      };

      for (auto& param : this->parameters)
         _sever(param);
      if (this->event_parameters.has_value()) {
         if (auto& form = this->event_parameters.value().form; form == target) {
            form = nullptr;
            changes_made = true;
         }
      }
      _sever(this->comparison.operand);
      _sever(this->run_on.entity);

      return changes_made;
   }
}