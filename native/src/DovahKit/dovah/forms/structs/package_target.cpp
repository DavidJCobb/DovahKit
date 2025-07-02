#include "./package_target.h"
#include "../_common_cpp.h"

#include "../Package.h"

#include "../../notices/form_load_warnings/by_form_type/faction/interrupt_override_target_not_in_a_package.h"
#include "../../notices/form_load_warnings/by_form_type/package/invalid_interrupt_override_target.h"
#include "../../notices/form_load_warnings/by_form_type/package/target_has_an_invalid_object_type.h"
#include "../../notices/form_load_warnings/by_form_type/package/wrong_target_for_interrupt_override.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::faction;
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs {
   package_target::target_type package_target::get_type() const {
      return (target_type)this->data.index();
   }
   void package_target::set_type(Form& my_owner, package_target::target_type t) {
      auto prior = this->get_type();
      if (prior == t)
         return;
      this->_clear_data(my_owner);
      this->_emplace_data_for_type(t);
   }

   void package_target::_clear_data(Form& my_owner) {
      switch (this->get_type()) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
         case name: \
            std::get<(size_t)name>(this->data).set(my_owner, nullptr); \
            break;

         CASE(target_type::reference);
         CASE(target_type::object);
         CASE(target_type::linked_ref);
         #pragma pop_macro("CASE")
      }
   }
   void package_target::_emplace_data_for_type(target_type t) {
      if (this->get_type() == t)
         return;
      switch (t) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
         case name: this->data.emplace<(size_t)name>(); break;

         CASE(target_type::reference);
         CASE(target_type::object);
         CASE(target_type::object_type);
         CASE(target_type::linked_ref);
         CASE(target_type::reference_alias);
         CASE(target_type::interrupt_override_target);
         CASE(target_type::self);
         #pragma pop_macro("CASE")
      }
   }

   void package_target::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, Form& my_owner) {
      target_type t = (target_type)0;
      subrecord.read(t);
      this->_emplace_data_for_type(t);
      switch (t) {
         case target_type::reference:
            {
               auto& form = *this->as_type<target_type::reference>();
               if (subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::reference, subrecord);
               }
            }
            break;
         case target_type::object:
            {
               auto& form = *this->as_type<target_type::object>();
               if (subrecord.read(form)) {
                  ; // TODO: What form types are valid here?
               }
            }
            break;
         case target_type::object_type:
            {
               auto& data = *this->as_type<target_type::object_type>();
               if (subrecord.read(data)) {
                  if ((uint32_t)data > (uint32_t)object_type::actors_any) {
                     specific_load_warnings::target_has_an_invalid_object_type notice(
                        intfc.target_stub,
                        (std::underlying_type_t<object_type>)data
                     );
                     intfc.log_load_warning(notice);
                  }
               }
            }
            break;
         case target_type::linked_ref:
            {
               auto& form = *this->as_type<target_type::linked_ref>();
               if (subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::keyword, subrecord);
               }
            }
            break;
         case target_type::reference_alias:
            {
               auto& alias_id = *this->as_type<target_type::reference_alias>();
               subrecord.read(alias_id);
            }
            break;
         case target_type::interrupt_override_target:
            {
               auto& data = *this->as_type<target_type::interrupt_override_target>();
               subrecord.read(data);

               bool valid = false;
               switch (data) {
                  case interrupt_override_target::threat_to_spectate:
                  case interrupt_override_target::corpse_to_observe:
                  case interrupt_override_target::ref_to_guard:
                  case interrupt_override_target::trespasser:
                  case interrupt_override_target::combat_target:
                     valid = true;
                     break;
               }

               if (my_owner.stub.form_type == form_type::package) {
                  if (valid) {
                     auto& casted = (Package&)my_owner;
                     if (casted.interrupt_override != packages::interrupt_override_for_target(data)) {
                        specific_load_warnings::wrong_target_for_interrupt_override notice(
                           intfc.target_stub,
                           data,
                           packages::interrupt_override_for_target(data),
                           casted.interrupt_override
                        );
                        intfc.log_load_warning(notice);
                     }
                  } else {
                     specific_load_warnings::invalid_interrupt_override_target notice(
                        intfc.target_stub,
                        (std::underlying_type_t<interrupt_override_target>)data
                     );
                     intfc.log_load_warning(notice);
                  }
               } else {
                  specific_load_warnings::interrupt_override_target_not_in_a_package notice(
                     intfc.target_stub,
                     data
                  );
                  intfc.log_load_warning(notice);
               }
            }
            break;

         case target_type::self:
         default:
            subrecord.skip_bytes(4);
            break;
      }
      subrecord.read(this->distance);
   }
   void package_target::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      const auto type = this->get_type();
      subrecord.write(type);
      switch (type) {
         case target_type::reference:
            subrecord.write(std::get<(size_t)target_type::reference>(this->data));
            break;
         case target_type::object:
            subrecord.write(std::get<(size_t)target_type::object>(this->data));
            break;
         case target_type::object_type:
            subrecord.write(std::get<(size_t)target_type::object_type>(this->data));
            break;
         case target_type::linked_ref:
            subrecord.write(std::get<(size_t)target_type::linked_ref>(this->data));
            break;
         case target_type::reference_alias:
            subrecord.write(std::get<(size_t)target_type::reference_alias>(this->data));
            break;
         case target_type::interrupt_override_target:
            subrecord.write(std::get<(size_t)target_type::interrupt_override_target>(this->data));
            break;
         case target_type::self:
         default:
            subrecord.skip_bytes(4);
            break;
      }
      subrecord.write(this->distance);
   }

   void package_target::clone_from(const package_target& src, Form& my_owner) noexcept {
      this->_clear_data(my_owner);
      switch (src.get_type()) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
            case name: this->data.emplace<(size_t)name>().set(my_owner, std::get<(size_t)name>(src.data)); break;
         CASE(target_type::reference);
         CASE(target_type::object);
         CASE(target_type::linked_ref);
         #pragma pop_macro("CASE")

         default:
            this->data = src.data;
      }
      this->distance = src.distance;
   }
   void package_target::unmanaged_clone_from(const package_target& src, load_order_interfaces::form_load&) noexcept {
      switch (src.get_type()) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
            case name: this->data.emplace<(size_t)name>().unmanaged_set(std::get<(size_t)name>(src.data).get_form_stub()); break;
         CASE(target_type::reference);
         CASE(target_type::object);
         CASE(target_type::linked_ref);
         #pragma pop_macro("CASE")

         default:
            this->data = src.data;
      }
      this->distance = src.distance;
   }
   void package_target::clear(Form& my_owner) noexcept {
      this->_clear_data(my_owner);
      this->distance = 0;
   }
   void package_target::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      switch (this->get_type()) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
            case name: std::get<(size_t)name>(this->data).clear_if(my_owner, other); break;
         CASE(target_type::reference);
         CASE(target_type::object);
         CASE(target_type::linked_ref);
         #pragma pop_macro("CASE")
      }
   }

   #pragma region package_target::use_info_state
   void package_target::use_info_state::generate_use_info(tes_subrecord_reader& subrecord) {
      target_type t;
      if (subrecord.read(t)) {
         switch (t) {
            case target_type::reference:
            case target_type::object:
            case target_type::linked_ref:
               subrecord.read(this->form);
               break;
            default:
               this->form = {};
               break;
         }
      }
   }
   void package_target::use_info_state::clear() {
      this->form = {};
   }
   void package_target::use_info_state::commit_to(form_stub_use_info_builder& uib) {
      if (this->form)
         uib.add_outbound_reference(this->form);
   }
   #pragma endregion
}