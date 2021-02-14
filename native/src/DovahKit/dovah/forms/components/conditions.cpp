#include "conditions.h"
#include "../../../helpers/strings.h"
#include "../_common_cpp.h"

#include "../Package.h"
#include "../Quest.h"

namespace dovah::loaded_forms::components {
   #pragma region condition
      void condition::_set_form_reference(form_reference_t& ref, form_stub* stub) {
         if (this->is_working_copy) {
            ref.unmanaged_set(stub);
            return;
         }
         auto* form = this->owner.form;
         assert(form);
         ref.set(*form, stub);
      }
      void condition::_clear_form_reference_if(form_reference_t& ref, form_stub& target) {
         if (ref != &target)
            return;
         this->_set_form_reference(ref, nullptr);
      }

      condition::condition(const condition& other) : owner(other.owner), is_working_copy(other.is_working_copy) {
         this->flags    = other.flags;
         this->function = other.function;
         //
         this->run_on.type  = other.run_on.type;
         this->run_on.index = other.run_on.index;
         this->run_on.reference.unmanaged_set(other.run_on.reference.get_form_stub());
         //
         for (size_t i = 0; i < this->parameters.size(); ++i) {
            auto& dst = this->parameters[i];
            auto& src = other.parameters[i];
            dst.underlying = src.underlying;
            dst.dword      = src.dword;
            dst.string     = src.string;
            dst.form.unmanaged_set(src.form.get_form_stub());
         }
         this->event_parameters.function = other.event_parameters.function;
         this->event_parameters.member   = other.event_parameters.member;
         this->event_parameters.form.unmanaged_set(other.event_parameters.form.get_form_stub());
         //
         this->comparison.op = other.comparison.op;
         this->comparison.operand.constant = other.comparison.operand.constant;
         this->comparison.operand.global.unmanaged_set(other.comparison.operand.global.get_form_stub());
      }
      condition::condition(condition&& other) : owner(other.owner), is_working_copy(other.is_working_copy) {
         this->flags    = other.flags;
         this->function = other.function;
         //
         this->run_on.type  = other.run_on.type;
         this->run_on.index = other.run_on.index;
         this->run_on.reference.unmanaged_set(other.run_on.reference.get_form_stub());
         //
         for (size_t i = 0; i < this->parameters.size(); ++i) {
            auto& dst = this->parameters[i];
            auto& src = other.parameters[i];
            dst.underlying = src.underlying;
            dst.dword      = src.dword;
            std::swap(dst.string, src.string);
            dst.form.unmanaged_set(src.form.get_form_stub());
         }
         this->event_parameters.function = other.event_parameters.function;
         this->event_parameters.member   = other.event_parameters.member;
         this->event_parameters.form.unmanaged_set(other.event_parameters.form.get_form_stub());
         //
         this->comparison.op = other.comparison.op;
         this->comparison.operand.constant = other.comparison.operand.constant;
         this->comparison.operand.global.unmanaged_set(other.comparison.operand.global.get_form_stub());
      }

      condition_parameter_type* condition::get_argument_type(uint8_t index) const noexcept {
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

      condition condition::make_working_copy() const noexcept {
         condition wc(this->owner, true);
         wc.clone_from(*this);
         return wc;
      }
      
      #pragma region Accessors
         void condition::set_function_id(uint16_t id) noexcept {
            if (id == this->function)
               return;
            //
            for (auto& p : this->parameters) {
               this->_set_form_reference(p.form, nullptr);
               p.string.clear();
               p.dword = 0;
            }
            this->_set_form_reference(this->event_parameters.form, nullptr);
            this->function = id;
         }
         const condition_function* condition::get_function() const noexcept {
            return condition_function::lookup_by_id(this->function);
         }
         void condition::set_function(const condition_function* f) noexcept {
            this->set_function_id(f->id);
         }
         //
         const condition_parameter condition::get_parameter(uint8_t i) const {
            auto& source = this->parameters[i];
            condition_parameter readonly;
            readonly.dword      = source.dword;
            readonly.form       = source.form.get_form_stub();
            readonly.string     = source.string;
            readonly.underlying = source.underlying;
            return readonly;
         }
         void condition::set_parameter(uint8_t i, const condition_parameter& value) {
            auto u = this->get_argument_underlying_type(i);
            if (value.underlying != u)
               return;
            //
            auto& p = this->parameters[i];
            this->_set_form_reference(p.form, nullptr);
            p.dword = 0;
            p.string.clear();
            //
            switch (u) {
               case condition_parameter_underlying_type::aliasID:
               case condition_parameter_underlying_type::character:
               case condition_parameter_underlying_type::int_signed:
               case condition_parameter_underlying_type::int_unsigned:
               case condition_parameter_underlying_type::package_data:
               case condition_parameter_underlying_type::quest_stage:
                  p.dword = value.dword;
                  break;
               case condition_parameter_underlying_type::float32:
                  p.float32 = value.float32;
                  break;
               case condition_parameter_underlying_type::string:
                  p.string = value.string;
                  break;
               case condition_parameter_underlying_type::formID:
                  this->_set_form_reference(p.form, value.form);
                  break;
            }
         }
         //
         const condition_event_parameters& condition::get_event_parameters() const {
            return this->event_parameters;
         }
         void condition::set_event_parameters(const condition_event_parameters& value) {
            auto* f = condition_function::lookup_by_id(this->function);
            if (!f || !f->uses_event_data)
               return;
            this->event_parameters.function = value.function;
            this->event_parameters.member   = value.member;
            this->_set_form_reference(this->event_parameters.form, value.form.get_form_stub());
         }

         void condition::set_comparison(const comparison_t& cmp) noexcept {
            this->comparison.op = cmp.op;
            if (cmp.operand.global) {
               this->set_comparison_operand(cmp.operand.global);
            } else {
               this->set_comparison_operand(cmp.operand.constant);
            }
         }
         void condition::set_comparison_operand(float operand) {
            this->flags &= ~flag::compare_to_global;
            this->_set_form_reference(this->comparison.operand.global, nullptr);
            this->comparison.operand.constant = operand;
         }
         void condition::set_comparison_operand(form_stub* operand) {
            this->flags |= flag::compare_to_global;
            this->_set_form_reference(this->comparison.operand.global, operand);
         }

         void condition::modify_flags(flags_t f, bool clear_or_set) noexcept {
            if (f & flag::use_aliases)
               this->set_uses_aliases(clear_or_set);
            if (f & flag::use_package_data)
               this->set_uses_package_data(clear_or_set);
            f &= ~(flag::use_aliases | flag::use_package_data);
            cobb::edit_bit(this->flags, f, clear_or_set);
         }
         void condition::set_uses_aliases(bool f) {
            bool prior = this->flags & flag::use_aliases;
            if (prior == f)
               return;
            if (!prior) {
               //
               // The flag wasn't already set, so find any form-type parameters that would have their underlying 
               // types changed by the flag, and clear their values.
               //
               for (int i = 0; i < this->parameters.size(); ++i) {
                  auto* type  = this->get_argument_type(i);
                  auto  under = this->get_argument_underlying_type(i);
                  if (type->allow_overrides && under == condition_parameter_underlying_type::formID)
                     this->_set_form_reference(this->parameters[i].form, nullptr);
               }
            }
            cobb::edit_bit(this->flags, flag::use_aliases, f);
         }
         void condition::set_uses_package_data(bool f) {
            bool prior = this->flags & flag::use_package_data;
            if (prior == f)
               return;
            if (!prior) {
               //
               // The flag wasn't already set, so find any form-type parameters that would have their underlying 
               // types changed by the flag, and clear their values.
               //
               for (int i = 0; i < this->parameters.size(); ++i) {
                  auto* type  = this->get_argument_type(i);
                  auto  under = this->get_argument_underlying_type(i);
                  if (type->allow_overrides && under == condition_parameter_underlying_type::formID)
                     this->_set_form_reference(this->parameters[i].form, nullptr);
               }
            }
            cobb::edit_bit(this->flags, flag::use_package_data, f);
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
                           condition_parameter_in_situ value;
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
         void condition::clone_from(const condition& source) noexcept {
            this->clear();
            //
            this->flags = source.flags;
            this->comparison.op = source.comparison.op;
            this->comparison.operand.constant = source.comparison.operand.constant;
            if (this->flags & flag::compare_to_global)
               this->_set_form_reference(this->comparison.operand.global, source.comparison.operand.global.get_form_stub());
            else
               this->_set_form_reference(this->comparison.operand.global, nullptr);
            //
            this->set_function(source.get_function());
            for (int i = 0; i < this->parameters.size(); ++i)
               this->set_parameter(i, source.get_parameter(i));
            this->set_event_parameters(source.get_event_parameters());
            //
            this->run_on.type  = source.run_on.type;
            this->run_on.index = source.run_on.index;
            //
            form_stub* run_on_ptr = nullptr;
            switch (this->run_on.type) {
               case run_on_type::subject:
               case run_on_type::target:
               case run_on_type::combat_target:
               case run_on_type::linked_ref:
               case run_on_type::quest_alias:
               case run_on_type::package_data:
               case run_on_type::event_data:
                  break;
               case run_on_type::reference:
                  run_on_ptr = source.run_on.reference.get_form_stub();
                  break;
            }
            this->_set_form_reference(this->run_on.reference, run_on_ptr);
         }
         void condition::sever_outbound_references_to(form_stub& target) noexcept {
            this->_clear_form_reference_if(this->comparison.operand.global, target);
            this->_clear_form_reference_if(this->run_on.reference, target);
            this->_clear_form_reference_if(this->event_parameters.form, target);
            //
            for (auto& param : this->parameters)
               this->_clear_form_reference_if(param.form, target);
         }
         void condition::clear() {
            for (auto& p : this->parameters) {
               this->_set_form_reference(p.form, nullptr);
               p.string.clear();
               p.dword = 0;
            }
            //
            this->_set_form_reference(this->run_on.reference, nullptr);
            this->_set_form_reference(this->event_parameters.form, nullptr);
            this->_set_form_reference(this->comparison.operand.global, nullptr);
            //
            this->flags = 0;
            this->comparison.op = operator_type::equal;
            this->comparison.operand.constant = 0.0F;
            this->run_on.index = -1;
            this->event_parameters.function = 0;
            this->event_parameters.member   = 0;
            //
            this->set_function(0);
         }
      #pragma endregion

      /*static*/ void append_to_condition_list(form_stub& dst_owner, std::vector<condition>& dst, tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         dst.emplace_back(dst_owner).read(record, intfc);
      }
      /*static*/ void condition::clone_condition_list(form_stub& dst_owner, std::vector<condition>& dst, const std::vector<condition>& src, bool append) {
         if (append) {
            dst.reserve(dst.size() + src.size());
         } else {
            for (auto& cnd : dst)
               cnd.clear();
            dst.clear();
            dst.reserve(src.size());
         }
         for (auto& cnd : src)
            dst.emplace_back(dst_owner).clone_from(cnd);
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