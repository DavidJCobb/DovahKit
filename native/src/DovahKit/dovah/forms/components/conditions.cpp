#include "conditions.h"
#include "../../../helpers/strings.h"
#include "../_common_cpp.h"

#include "../Package.h"
#include "../Quest.h"

namespace dovah::loaded_forms::components {
   #pragma region condition
      condition_parameter_type* condition::get_argument_type(uint8_t index) const noexcept {
         if (index >= 2)
            return nullptr;
         auto func = condition_function::lookup_by_id(this->function);
         if (!func)
            return nullptr;
         auto a = func->argument_types[index];
         if (a->is_union()) {
            assert(index != 0 && "No behavior defined for a condition function whose first argument type is a union!");
            return a->resolve_union(func->argument_types[index - 1], this->get_parameter(index - 1));
         }
         return a;
      }
      condition_parameter_underlying_type condition::get_argument_underlying_type(uint8_t index) const noexcept {
         auto a = this->get_argument_type(index);
         if (a) {
            if (a->allow_overrides) {
               if (this->flags & flag::use_aliases)
                  return condition_parameter_underlying_type::aliasID;
               if (this->flags & flag::use_package_data)
                  return condition_parameter_underlying_type::package_data;
            }
            return a->underlying;
         }
         return condition_parameter_underlying_type::none;
      }
      
      #pragma region Accessors
         const condition_function* condition::get_function() const noexcept {
            return condition_function::lookup_by_id(this->function);
         }
         const condition_parameter condition::get_parameter(uint8_t i) const {
            if (i >= this->parameters.size())
               return condition_parameter();
            auto& source = this->parameters[i];
            condition_parameter readonly;
            readonly.dword      = source.dword;
            readonly.form       = source.form.get_form_stub();
            readonly.string     = source.string;
            readonly.underlying = source.underlying;
            return readonly;
         }
      #pragma endregion

      bool condition::refers_to_form(const form_stub* target) const noexcept {
         if (this->run_on.reference == target)
            return true;
         if (this->event_parameters.form == target)
            return true;
         if (this->comparison.operand.global == target)
            return true;
         for (auto& p : this->parameters)
            if (p.form == target)
               return true;
         return false;
      }
      
      #pragma region Form boilerplate
         bool condition::read(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
            auto& subrecord = record.get_current_subrecord();
            assert(subrecord.signature() == 'CTDA' && "Condition::read should only be called just after the CTDA subrecord is opened.");
            if (!subrecord.is_in_bounds(0x14))
               return false;
            {
               uint8_t type; // flags | (operator << 5)
               subrecord.unchecked_read(type);
               this->comparison.op = (operator_type)((type >> 5) & 7);
               this->flags = type & 0x1F;
            }
            subrecord.skip_bytes(3);
            if (this->flags & flag::compare_to_global) {
               subrecord.unchecked_read(this->comparison.operand.global);
               intfc.log_load_warning(
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::global, intfc.target_stub, this->comparison.operand.global)
               );
            } else {
               subrecord.unchecked_read(this->comparison.operand.constant);
            }
            subrecord.unchecked_read(this->function);
            subrecord.skip_bytes(2);
            {
               auto func = condition_function::lookup_by_id(this->function);
               if (func) {
                  if (!subrecord.is_in_bounds(8))
                     return false;
                  if (func->uses_event_data) {
                     //
                     // Read GetEventData parameters.
                     //
                     subrecord.unchecked_read(this->event_parameters.function);
                     subrecord.unchecked_read(this->event_parameters.member);
                     subrecord.unchecked_read(this->event_parameters.form);
                  } else {
                     //
                     // Read standard parameters.
                     //
                     for (int i = 0; i < this->parameters.size(); i++) {
                        if (this->get_argument_underlying_type(i) == condition_parameter_underlying_type::formID) {
                           auto* arg_type   = func->argument_types[i];
                           auto& allowed    = arg_type->allowed_form_types;
                           auto& value_form = this->parameters[i].form;
                           subrecord.unchecked_read(value_form);
                           //
                           if (allowed.size() == 1) {
                              intfc.log_load_warning(
                                 detailed_notice::warn_if_wrong_type(subrecord.signature(), arg_type->allowed_form_types[0], intfc.target_stub, value_form)
                              );
                           } else if (auto* stub = value_form.get_form_stub()) {
                              if (!arg_type->allows_form_type(stub->formType)) {
                                 intfc.log_load_warning(
                                    detailed_notice::warn_if_wrong_type(subrecord.signature(), {}, intfc.target_stub, value_form)
                                 );
                              }
                           }
                        } else {
                           subrecord.unchecked_read(this->parameters[i].dword);
                        }
                     }
                  }
               } else {
                  //
                  // Unknown function. TODO: warning
                  //
                  subrecord.skip_bytes(8);
               }
            }
            //
            if (!subrecord.is_in_bounds(12))
               return false;
            subrecord.unchecked_read(this->run_on.type);
            subrecord.unchecked_read(this->run_on.reference);
            if (this->run_on.type == run_on_type::event_data) {
               subrecord.read_signature(this->run_on.index);
            } else {
               subrecord.unchecked_read(this->run_on.index);
            }
            intfc.log_load_warning(
               detailed_notice::warn_if_not_object_reference(subrecord.signature(), intfc.target_stub, this->run_on.reference)
            );
            //
            // End of CTDA subrecord.
            //
            auto next = record.peek_next_subrecord_type();
            if (next != 'CIS1' && next != 'CIS2')
               return true;
            if (next == 'CIS1') {
               auto& sub = record.next_subrecord();
               sub.to_string(this->parameters[0].string);
               //
               next = record.peek_next_subrecord_type();
            }
            if (next == 'CIS2') {
               auto& sub = record.next_subrecord();
               sub.to_string(this->parameters[1].string);
            }
            return true;
         }
         /*static*/ void condition::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
            auto& subrecord = record.get_current_subrecord();
            assert(subrecord.signature() == 'CTDA' && "Condition::read should only be called just after the CTDA subrecord is opened.");
            if (!subrecord.is_in_bounds(0x14))
               return;
            uint8_t   type;
            uint16_t  function;
            form_id_t formID;
            subrecord.unchecked_read(type);
            subrecord.skip_bytes(3);
            if (type & flag::compare_to_global) {
               subrecord.unchecked_read(formID);
               uib.add_outbound_reference(formID);
            } else
               subrecord.skip_bytes(4);
            subrecord.unchecked_read(function);
            subrecord.skip_bytes(2);
            {
               bool uses_aliases  = type & flag::use_aliases;
               bool uses_packdata = type & flag::use_package_data;
               uint32_t firstValue = 0; // needed for when the second arg is a union
               //
               auto func = condition_function::lookup_by_id(function);
               if (func) {
                  if (func->uses_event_data) {
                     subrecord.skip_bytes(4);
                     subrecord.unchecked_read(formID);
                     uib.add_outbound_reference(formID);
                  } else {
                     for (int i = 0; i < 2; ++i) {
                        auto* type  = func->argument_types[i];
                        if (i == 1 && type->is_union()) { // resolve the union
                           condition_parameter value;
                           value.dword = firstValue;
                           type = type->resolve_union(type, value);
                        }
                        //
                        auto under = type->underlying;
                        if (type->allow_overrides) {
                           if (uses_aliases)
                              under = condition_parameter_underlying_type::aliasID;
                           if (uses_packdata)
                              under = condition_parameter_underlying_type::package_data;
                        }
                        //
                        if (under == condition_parameter_underlying_type::formID) {
                           subrecord.unchecked_read(formID);
                           uib.add_outbound_reference(formID);
                        } else {
                           subrecord.skip_bytes(4);
                        }
                     }
                  }
               } else {
                  subrecord.skip_bytes(8);
               }
               //
               if (!subrecord.is_in_bounds(12))
                  return;
               subrecord.skip_bytes(4);
               subrecord.unchecked_read(formID);
               uib.add_outbound_reference(formID);
               subrecord.skip_bytes(4);
            }
            auto next = record.peek_next_subrecord_type();
            if (next != 'CIS1' && next != 'CIS2')
               return;
            if (next == 'CIS1') {
               record.next_subrecord();
               next = record.peek_next_subrecord_type();
            }
            if (next == 'CIS2')
               record.next_subrecord();
         }
         void condition::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
            auto& subrecord = record.open_next_subrecord('CTDA');
            {
               uint8_t type = this->flags | (uint8_t(this->comparison.op) << 5); // flags | (operator << 5)
               subrecord.write(type);
            }
            subrecord.skip_bytes(3);
            if (this->flags & flag::compare_to_global)
               subrecord.write(this->comparison.operand.global);
            else
               subrecord.write(this->comparison.operand.constant);
            subrecord.write(this->function);
            subrecord.skip_bytes(2);
            {
               auto* func = condition_function::lookup_by_id(this->function);
               if (func) {
                  if (func->uses_event_data) {
                     subrecord.write(this->event_parameters.function);
                     subrecord.write(this->event_parameters.member);
                     subrecord.write(this->event_parameters.form);
                  } else {
                     for (int i = 0; i < this->parameters.size(); i++) {
                        if (this->get_argument_underlying_type(i) == condition_parameter_underlying_type::formID)
                           subrecord.write(this->parameters[i].form);
                        else
                           subrecord.write(this->parameters[i].dword);
                     }
                  }
               } else {
                  subrecord.skip_bytes(8);
               }
            }
            //
            subrecord.write(this->run_on.type);
            subrecord.write(this->run_on.reference);
            if (this->run_on.type == run_on_type::event_data) {
               subrecord.write_signature(this->run_on.index);
            } else {
               subrecord.write(this->run_on.index);
            }
            //
            subrecord.close();
            //
            if (this->get_argument_underlying_type(0) == condition_parameter_underlying_type::string) {
               auto& CIS1 = record.open_next_subrecord('CIS1');
               CIS1.write(this->parameters[0].string);
               CIS1.close();
            }
            if (this->get_argument_underlying_type(1) == condition_parameter_underlying_type::string) {
               auto& CIS1 = record.open_next_subrecord('CIS2');
               CIS1.write(this->parameters[1].string);
               CIS1.close();
            }
         }
         void condition::clone_from(const condition& source, loaded_forms::Form& my_owner) noexcept {
            this->clear(my_owner);
            //
            this->flags = source.flags;
            this->comparison.op = source.comparison.op;
            this->comparison.operand.constant = source.comparison.operand.constant;
            if (this->flags & flag::compare_to_global)
               this->comparison.operand.global.set(my_owner, source.comparison.operand.global.get_form_stub());
            //
            this->function = source.function;
            auto* func = condition_function::lookup_by_id(this->function);
            if (func && func->uses_event_data) {
               this->event_parameters.function = source.event_parameters.function;
               this->event_parameters.member   = source.event_parameters.member;
               this->event_parameters.form.set(my_owner, source.event_parameters.form);
            } else {
               for (int i = 0; i < this->parameters.size(); ++i) {
                  auto& dst = this->parameters[i];
                  auto& src = source.parameters[i];
                  dst.dword  = src.dword;
                  dst.string = src.string;
                  dst.form.set(my_owner, src.form);
               }
            }
            //
            this->run_on.type  = source.run_on.type;
            this->run_on.index = source.run_on.index;
            if (this->run_on.type == run_on_type::reference)
               this->run_on.reference.set(my_owner, source.run_on.reference);
         }
         void condition::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
            this->comparison.operand.global.clear_if(my_owner, target);
            this->run_on.reference.clear_if(my_owner, target);
            this->event_parameters.form.clear_if(my_owner, target);
            //
            for (auto& param : this->parameters)
               param.form.clear_if(my_owner, target);
         }
         void condition::clear(loaded_forms::Form& my_owner) {
            for (auto& p : this->parameters) {
               p.form.set(my_owner, nullptr);
               p.string.clear();
               p.dword = 0;
            }
            //
            this->run_on.reference.set(my_owner, nullptr);
            this->event_parameters.form.set(my_owner, nullptr);
            this->comparison.operand.global.set(my_owner, nullptr);
            //
            this->flags = 0;
            this->comparison.op = operator_type::equal;
            this->comparison.operand.constant = 0.0F;
            this->run_on.index = -1;
            this->event_parameters.function = 0;
            this->event_parameters.member   = 0;
            //
            this->function = 0;
         }
      #pragma endregion

      working_condition condition::make_working_copy() const noexcept {
         working_condition out;
         //
         out.flags    = this->flags;
         out.function = this->function;
         for (size_t i = 0; i < this->parameters.size(); ++i) {
            auto& dst = out.parameters[i];
            auto& src = this->parameters[i];
            dst.underlying = src.underlying;
            dst.dword  = src.dword;
            dst.form   = src.form.get_form_stub();
            dst.string = src.string;
         }
         out.event_parameters.function = this->event_parameters.function;
         out.event_parameters.member   = this->event_parameters.member;
         out.event_parameters.form     = this->event_parameters.form.get_form_stub();
         out.comparison.op               = this->comparison.op;
         out.comparison.operand.constant = this->comparison.operand.constant;
         out.comparison.operand.global   = this->comparison.operand.global.get_form_stub();
         out.run_on.type      = this->run_on.type;
         out.run_on.index     = this->run_on.index;
         out.run_on.reference = this->run_on.reference.get_form_stub();
         //
         return out;
      }
      void condition::commit(loaded_forms::Form& my_owner, working_condition& source) {
         this->clear(my_owner);
         this->function = source.function;
         this->flags    = source.flags;
         //
         auto* func = condition_function::lookup_by_id(this->function);
         if (func && func->uses_event_data) {
            this->event_parameters.form.set(my_owner, source.event_parameters.form);
            this->event_parameters.function = source.event_parameters.function;
            this->event_parameters.member   = source.event_parameters.member;
         } else {
            for (size_t i = 0; i < this->parameters.size(); ++i) {
               auto& dst = this->parameters[i];
               auto& src = source.parameters[i];
               //
               switch (this->get_argument_underlying_type(i)) {
                  case condition_parameter_underlying_type::float32:
                     dst.float32 = src.float32;
                     break;
                  case condition_parameter_underlying_type::formID:
                     dst.form.set(my_owner, src.form);
                     break;
                  case condition_parameter_underlying_type::string:
                     dst.string = src.string;
                     break;
                  default:
                     dst.dword = src.dword;
               }
            }
         }
         //
         this->comparison.op = source.comparison.op;
         this->comparison.operand.constant = source.comparison.operand.constant;
         this->comparison.operand.global.set(my_owner, (this->flags & flag::compare_to_global) ? source.comparison.operand.global : nullptr);
         //
         this->run_on.type  = source.run_on.type;
         this->run_on.index = source.run_on.index;
         this->run_on.reference.set(my_owner, this->run_on.type == run_on_type::reference ? source.run_on.reference : nullptr);
      }
   #pragma endregion

   #pragma region working_condition
   condition_parameter_type* working_condition::get_argument_type(uint8_t index) const noexcept {
      if (index >= 2)
         return nullptr;
      auto func = condition_function::lookup_by_id(this->function);
      if (!func)
         return nullptr;
      auto a = func->argument_types[index];
      if (a->is_union()) {
         assert(index != 0 && "No behavior defined for a condition function whose first argument type is a union!");
         return a->resolve_union(func->argument_types[index - 1], this->parameters[index - 1]);
      }
      return a;
   }
   condition_parameter_underlying_type working_condition::get_argument_underlying_type(uint8_t index) const noexcept {
      auto a = this->get_argument_type(index);
      if (a) {
         if (a->allow_overrides) {
            if (this->flags & flag::use_aliases)
               return condition_parameter_underlying_type::aliasID;
            if (this->flags & flag::use_package_data)
               return condition_parameter_underlying_type::package_data;
         }
         return a->underlying;
      }
      return condition_parameter_underlying_type::none;
   }

   bool working_condition::refers_to_form(const form_stub* target) const noexcept {
      if (this->run_on.reference == target)
         return true;
      if (this->event_parameters.form == target)
         return true;
      if (this->comparison.operand.global == target)
         return true;
      for (auto& p : this->parameters)
         if (p.form == target)
            return true;
      return false;
   }

   void working_condition::fix_parameter_types() {
      auto* func = condition_function::lookup_by_id(this->function);
      if (func && func->uses_event_data) {
         for (auto& p : this->parameters) {
            p.dword = 0;
            p.form  = nullptr;
            p.string.clear();
            p.underlying = condition_parameter_underlying_type::none;
         }
         return;
      }
      for (size_t i = 0; i < this->parameters.size(); ++i) {
         auto& p = this->parameters[i];
         auto  u = this->get_argument_underlying_type(i);
         if (p.underlying != u) {
            p.form = nullptr;
            p.string.clear();
            p.underlying = u;
            switch (u) {
               case condition_parameter_underlying_type::aliasID:
               case condition_parameter_underlying_type::package_data:
                  p.dword = -1;
                  break;
               default:
                  p.dword = 0;
                  break;
            }
         }
      }
   }
   #pragma endregion

   #pragma region condition_list
   void condition_list::clear(loaded_forms::Form& my_owner) noexcept {
      for (auto& cnd : *this)
         cnd.clear(my_owner);
      std::vector<condition>::clear();
   }

   void condition_list::append_all_of(loaded_forms::Form& my_owner, const std::vector<condition>& other) {
      this->reserve(this->size() + other.size());
      for (auto& cnd : other)
         this->emplace_back().clone_from(cnd, my_owner);
   }
   bool condition_list::read_next(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      return this->emplace_back().read(record, intfc);
   }
   #pragma endregion

   #pragma region condition_context
   condition_context::condition_context(form_stub& owner, bool prefer_working_copy) : owner(&owner), prefer_working_copy(prefer_working_copy) {
      if (owner.formType == form_type::quest) {
         this->quest = &owner;
      } else if (owner.formType == form_type::package) {
         this->package = &owner;
         //
         // TODO: get owning quest
         //
      } else if (owner.formType == form_type::scene) {
         //
         // TODO: get owning quest
         //
      } else if (owner.formType == form_type::topic) {
         //
         // TODO: get owning quest
         //
      } else if (owner.formType == form_type::topic_info) {
         //
         // TODO: get owning quest
         //
      }
      //
      if (auto* s = this->package) {
         if (s->formType != form_type::package)
            this->package = nullptr;
         else
            this->loaded.package = s->load().ptr_cast<loaded_forms::Package>();
      }
      if (auto* s = this->quest) {
         if (s->formType != form_type::quest)
            this->quest = nullptr;
         else
            this->loaded.quest = s->load().ptr_cast<loaded_forms::Quest>();
      }
   }
   loaded_forms::Package* condition_context::get_owning_package() const noexcept {
      if (!this->package)
         return nullptr;
      if (this->prefer_working_copy) {
         auto* wc = this->package->get_working_copy<loaded_forms::Package>();
         if (wc)
            return wc;
      }
      return this->loaded.package.unwrap();
   }
   loaded_forms::Quest* condition_context::get_owning_quest() const noexcept {
      if (!this->quest)
         return nullptr;
      if (this->prefer_working_copy) {
         auto* wc = this->package->get_working_copy<loaded_forms::Quest>();
         if (wc)
            return wc;
      }
      return this->loaded.quest.unwrap();
   }
   #pragma endregion
}