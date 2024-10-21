#include "conditions.h"
#include "../../../helpers/strings.h"
#include "../_common_cpp.h"

#include "../../data/conditions/all_function_info.h"
#include "../../data/conditions/event_function.h"
#include "../../data/conditions/parameter_underlying_type.h"

#include "../Package.h"
#include "../Quest.h"

#include "./conditions/working_condition.h"
#include "./conditions/working_parameter.h"

namespace dovah::loaded_forms::components {
   #pragma region condition
      const dovah::conditions::parameter_typeinfo* condition::get_argument_type(uint8_t index) const noexcept {
         if (index >= 2)
            return nullptr;
         const auto* func = this->get_function();
         if (!func)
            return nullptr;
         auto a = func->argument_types[index];
         if (a->is_union()) {
            assert(index != 0 && "No behavior defined for a condition function whose first argument type is a union!");
            assert(func->argument_types[index - 1] != nullptr);
            return a->resolve_union_type(*func->argument_types[index - 1], this->parameters[index - 1].dword);
         }
         return a;
      }
      dovah::conditions::parameter_underlying_type condition::get_argument_underlying_type(uint8_t index) const noexcept {
         const auto* a = this->get_argument_type(index);
         if (!a)
            return dovah::conditions::parameter_underlying_type::none;
         if (a->allow_type_overrides) {
            if (this->flags & flag::use_aliases)
               return dovah::conditions::parameter_underlying_type::alias;
            if (this->flags & flag::use_package_data)
               return dovah::conditions::parameter_underlying_type::package_data;
         }
         return a->underlying_type;
      }
      
      #pragma region Accessors
         const dovah::conditions::function_info* condition::get_function() const noexcept {
            return dovah::conditions::function_info_by_id(this->function);
         }
         const conditions::working_parameter condition::get_parameter(uint8_t i) const {
            if (i >= this->parameters.size())
               return {};
            auto& src = this->parameters[i];

            conditions::working_parameter dst;

            auto underlying = src.underlying;
            if (this->flags & (flag::use_aliases | flag::use_package_data)) {
               if (auto* typeinfo = this->get_argument_type(i)) {
                  if (typeinfo->allow_type_overrides) {
                     underlying = (this->flags & flag::use_aliases) ? dovah::conditions::parameter_underlying_type::alias : dovah::conditions::parameter_underlying_type::package_data;
                  }
               }
            }
            switch (underlying) {
               using enum dovah::conditions::parameter_underlying_type;
               case alias:
               case int_unsigned:
               case package_data:
               case quest_stage:
                  dst = (uint32_t)src.dword;
                  break;

               case character:
                  dst = (char)(src.dword & 0xFF);
                  break;
               case float32:
                  dst = src.float32;
                  break;
               case form:
                  dst = src.form.get_form_stub();
                  break;
               case enumeration:
               case int_signed:
                  dst = src.integer;
                  break;
               case string:
                  dst = src.string;
                  break;
            }

            return dst;
         }
      #pragma endregion

      bool condition::refers_to_form(const form_stub* target) const noexcept {
         if (this->run_on.reference == target)
            return true;
         if (this->event_parameters.form == target)
            return true;
         if (auto* operand = std::get_if<form_reference_t>(&this->comparison.operand))
            if (*operand == target)
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
               this->comparison.op = (comparison_operator)((type >> 5) & 7);
               this->flags = type & 0x1F;
            }
            subrecord.skip_bytes(3);
            if (this->flags & flag::compare_to_global) {
               auto& dst = this->comparison.operand.emplace<form_reference_t>();
               subrecord.unchecked_read(dst);
               intfc.warn_if_ref_is_wrong_type(dst, form_type::global, subrecord.signature());
            } else {
               auto& dst = this->comparison.operand.emplace<float>();
               subrecord.unchecked_read(dst);
            }
            subrecord.unchecked_read(this->function);
            subrecord.skip_bytes(2);
            {
               const auto* func = this->get_function();
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
                        auto underlying = this->get_argument_underlying_type(i);
                        if (underlying == dovah::conditions::parameter_underlying_type::form) {
                           auto* arg_type   = func->argument_types[i];
                           auto& allowed    = arg_type->allowed_form_types;
                           auto& value_form = this->parameters[i].form;
                           subrecord.unchecked_read(value_form);
                           
                           if (std::holds_alternative<dovah::form_type>(allowed)) {
                              intfc.warn_if_ref_is_wrong_type(value_form, std::get<dovah::form_type>(allowed), subrecord.signature());
                           } else if (std::holds_alternative<dovah::conditions::parameter_typeinfo::form_type_list_info>(allowed)) {
                              auto* value_stub = value_form.get_form_stub();
                              if (value_stub && !arg_type->allows_form_type(value_stub->form_type)) {
                                 //
                                 // In order to list the allowed form types in the error message, we gotta pack 'em in 
                                 // a std::vector, unfortunately.
                                 //
                                 auto& allowed_info = std::get<dovah::conditions::parameter_typeinfo::form_type_list_info>(allowed);

                                 std::vector<dovah::form_type> types;
                                 types.resize(allowed_info.size);
                                 for (size_t i = 0; i < allowed_info.size; ++i)
                                    types[i] = allowed_info.types[i];

                                 intfc.warn_if_ref_is_wrong_type(value_form, types, subrecord.signature());
                              }
                           }
                        } else {
                           subrecord.unchecked_read(this->parameters[i].dword);
                        }
                        this->parameters[i].underlying = underlying;
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
            intfc.warn_if_ref_is_wrong_type(this->run_on.reference, form_type::reference, subrecord.signature());
            //
            // End of CTDA subrecord.
            //
            auto next = record.peek_next_subrecord_type();
            if (next != 'CIS1' && next != 'CIS2')
               return true;
            if (next == 'CIS1') {
               auto& sub = record.next_subrecord();
               sub.read(this->parameters[0].string);
               //
               next = record.peek_next_subrecord_type();
            }
            if (next == 'CIS2') {
               auto& sub = record.next_subrecord();
               sub.read(this->parameters[1].string);
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
               auto func = dovah::conditions::function_info_by_id(function);
               if (func) {
                  if (func->uses_event_data) {
                     subrecord.skip_bytes(4);
                     subrecord.unchecked_read(formID);
                     uib.add_outbound_reference(formID);
                  } else {
                     for (int i = 0; i < 2; ++i) {
                        auto* type  = func->argument_types[i];
                        if (i == 1 && type->is_union()) { // resolve the union
                           type = type->resolve_union_type(*type, firstValue);
                        }
                        //
                        auto under = type->underlying_type;
                        if (type->allow_type_overrides) {
                           if (uses_aliases)
                              under = dovah::conditions::parameter_underlying_type::alias;
                           if (uses_packdata)
                              under = dovah::conditions::parameter_underlying_type::package_data;
                        }
                        //
                        if (under == dovah::conditions::parameter_underlying_type::form) {
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

            bool comparison_is_to_global = std::holds_alternative<form_reference_t>(this->comparison.operand);

            {
               uint8_t type = this->flags | (uint8_t(this->comparison.op) << 5); // flags | (operator << 5)
               cobb::edit_bit(type, flag::compare_to_global, comparison_is_to_global);
               subrecord.write(type);
            }
            subrecord.skip_bytes(3);
            if (comparison_is_to_global) {
               subrecord.write(std::get<form_reference_t>(this->comparison.operand));
            } else {
               assert(std::holds_alternative<float>(this->comparison.operand));
               subrecord.write(std::get<float>(this->comparison.operand));
            }
            subrecord.write(this->function);
            subrecord.skip_bytes(2);
            {
               const auto* func = this->get_function();
               if (func) {
                  if (func->uses_event_data) {
                     subrecord.write(this->event_parameters.function);
                     subrecord.write(this->event_parameters.member);
                     subrecord.write(this->event_parameters.form);
                  } else {
                     for (int i = 0; i < this->parameters.size(); i++) {
                        if (this->get_argument_underlying_type(i) == dovah::conditions::parameter_underlying_type::form)
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
            if (this->get_argument_underlying_type(0) == dovah::conditions::parameter_underlying_type::string) {
               auto& CIS1 = record.open_next_subrecord('CIS1');
               CIS1.write(this->parameters[0].string);
               CIS1.close();
            }
            if (this->get_argument_underlying_type(1) == dovah::conditions::parameter_underlying_type::string) {
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
            if (std::holds_alternative<form_reference_t>(source.comparison.operand)) {
               auto& dst = this->comparison.operand.emplace<form_reference_t>();
               dst.set(my_owner, std::get<form_reference_t>(source.comparison.operand));
               //
               this->flags |= flag::compare_to_global;
            } else {
               this->comparison.operand = source.comparison.operand;
               //
               this->flags &= ~flag::compare_to_global;
            }
            
            this->function = source.function;
            auto* func = dovah::conditions::function_info_by_id(this->function);
            if (func && func->uses_event_data) {
               this->event_parameters.function = source.event_parameters.function;
               this->event_parameters.member   = source.event_parameters.member;
               this->event_parameters.form.set(my_owner, source.event_parameters.form);
            } else {
               for (int i = 0; i < this->parameters.size(); ++i) {
                  auto& dst = this->parameters[i];
                  auto& src = source.parameters[i];
                  dst.underlying = src.underlying;
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
            if (auto* operand = std::get_if<form_reference_t>(&this->comparison.operand))
               operand->clear_if(my_owner, target);

            this->run_on.reference.clear_if(my_owner, target);

            this->event_parameters.form.clear_if(my_owner, target);
            //
            for (auto& param : this->parameters)
               param.form.clear_if(my_owner, target);
         }
         void condition::clear(loaded_forms::Form& my_owner) {
            this->flags = 0;

            this->comparison.op = comparison_operator::equal;
            if (auto* operand = std::get_if<form_reference_t>(&this->comparison.operand))
               operand->set(my_owner, nullptr);
            else
               this->comparison.operand = 0.0F;

            this->run_on.index = -1;
            this->run_on.reference.set(my_owner, nullptr);

            for (auto& p : this->parameters) {
               p.form.set(my_owner, nullptr);
               p.string.clear();
               p.dword = 0;
            }

            this->event_parameters.function = 0;
            this->event_parameters.member   = 0;
            this->event_parameters.form.set(my_owner, nullptr);
            
            this->function = 0;
         }
      #pragma endregion

      void condition::commit(loaded_forms::Form& my_owner, const conditions::working_condition& source) {
         assert(source.valid());

         this->clear(my_owner);

         this->function = source.function;

         this->flags = 0;
         if (source.flags.or_linked)
            this->flags |= flag::or_linked;
         if (source.flags.swap_subject_and_target)
            this->flags |= flag::swap_subject_and_target;
         switch (source.override_types_with) {
            using enum conditions::parameter_type_override;
            case alias:
               this->flags |= flag::use_aliases;
               break;
            case package_data:
               this->flags |= flag::use_package_data;
               break;
         }
         if (std::holds_alternative<float>(source.comparison.operand))
            this->flags |= flag::compare_to_global;
         
         const auto* func = dovah::conditions::function_info_by_id(this->function);
         assert(func != nullptr);
         if (func->uses_event_data) {
            assert(source.event_parameters.has_value());
            auto& src_ep = source.event_parameters.value();
            this->event_parameters.form.set(my_owner, dovah::conditions::event_function_uses_form(src_ep.function) ? src_ep.form : nullptr);
            this->event_parameters.function = src_ep.function;
            this->event_parameters.member   = src_ep.member;
         } else {
            assert(!source.event_parameters.has_value());
            for (size_t i = 0; i < this->parameters.size(); ++i) {
               auto& dst = this->parameters[i];
               auto& src = source.parameters[i];
               
               auto* typeinfo   = this->get_argument_type(i);
               auto  underlying = this->get_argument_underlying_type(i);
               if (typeinfo->is_union() && i > 0) {
                  typeinfo = typeinfo->resolve_union_type(*this->get_argument_type(i - 1), this->parameters[i - 1].dword);
                  if (typeinfo)
                     underlying = typeinfo->underlying_type;
               }
               switch (underlying) {
                  case dovah::conditions::parameter_underlying_type::none:
                     dst.dword = 0;
                     break;
                  case dovah::conditions::parameter_underlying_type::character:
                     dst.dword = std::get<char>(src);
                     break;
                  case dovah::conditions::parameter_underlying_type::enumeration:
                  case dovah::conditions::parameter_underlying_type::int_signed:
                     dst.integer = std::get<int32_t>(src);
                     break;
                  case dovah::conditions::parameter_underlying_type::float32:
                     dst.float32 = std::get<float>(src);
                     break;
                  case dovah::conditions::parameter_underlying_type::form:
                     dst.form.set(my_owner, std::get<form_stub*>(src));
                     break;
                  case dovah::conditions::parameter_underlying_type::string:
                     dst.string = std::get<std::string>(src);
                     break;
                  default:
                     dst.dword = std::get<uint32_t>(src);
                     break;
               }
               dst.underlying = underlying;
            }
         }
         
         this->comparison.op = source.comparison.op;
         if (auto* operand = std::get_if<form_stub*>(&source.comparison.operand)) {
            auto& dst = this->comparison.operand.emplace<form_reference_t>();
            dst.set(my_owner, *operand);
            //
            this->flags |= flag::compare_to_global;
         } else {
            if (auto* dst = std::get_if<form_reference_t>(&this->comparison.operand)) {
               dst->set(my_owner, nullptr); // ensure use info is updated
            }
            this->comparison.operand = std::get<float>(source.comparison.operand);
            this->flags &= ~flag::compare_to_global;
         }
         
         this->run_on.type = source.run_on.type;
         if (auto* ref_ptr = std::get_if<form_stub*>(&source.run_on.entity)) {
            this->run_on.reference.set(my_owner, *ref_ptr);
         } else if (auto* casted = std::get_if<uint32_t>(&source.run_on.entity)) {
            this->run_on.index = *casted;
         } else {
            this->run_on.index = 0;
         }
      }
   #pragma endregion

   #pragma region condition_list
   void condition_list::clear(loaded_forms::Form& my_containing_form) noexcept {
      for (auto& cnd : *this)
         cnd.clear(my_containing_form);
      std::vector<condition>::clear();
   }

   void condition_list::append_all_of(loaded_forms::Form& my_containing_form, const std::vector<condition>& other) {
      this->reserve(this->size() + other.size());
      for (auto& cnd : other)
         this->emplace_back().clone_from(cnd, my_containing_form);
   }
   bool condition_list::read_next(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      return this->emplace_back().read(record, intfc);
   }

   void condition_list::append(loaded_forms::Form& my_containing_form, const conditions::working_condition& src) {
      this->emplace_back().commit(my_containing_form, src);
   }
   void condition_list::append_all_of(loaded_forms::Form& my_containing_form, const std::vector<conditions::working_condition>& src_list) {
      for(auto& src : src_list)
         this->emplace_back().commit(my_containing_form, src);
   }
   #pragma endregion
}