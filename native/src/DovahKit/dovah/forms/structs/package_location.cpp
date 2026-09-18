#include "./package_location.h"
#include "../_common_cpp.h"

#include "../Package.h"

#include "../../notices/form_load_warnings/by_form_type/faction/interrupt_override_target_not_in_a_package.h"
#include "../../notices/form_load_warnings/by_form_type/package/invalid_interrupt_override_target.h"
#include "../../notices/form_load_warnings/by_form_type/package/target_has_an_invalid_object_type.h"
#include "../../notices/form_load_warnings/by_form_type/package/target_is_exterior_cell.h"
#include "../../notices/form_load_warnings/by_form_type/package/wrong_target_for_interrupt_override.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::faction;
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs {
   package_location::location_type package_location::get_type() const {
      return (location_type)this->data.index();
   }
   void package_location::set_type(Form& my_owner, location_type t) {
      auto prior = this->get_type();
      if (prior == t)
         return;
      this->_clear_data(my_owner);
      this->_emplace_data_for_type(t);
   }

   bool package_location::empty() const {
      if (this->radius != 0)
         return false;

      switch (this->get_type()) {
         case location_type::object_type:
            return std::get<object_type>(this->data) == object_type::none;
         case location_type::reference_alias:
            return std::get<(size_t)location_type::reference_alias>(this->data) == -1;
         case location_type::location_alias:
            return std::get<(size_t)location_type::location_alias>(this->data) == -1;
         case location_type::interrupt_override_target:
            return false;
         case location_type::package_data_target:
            return std::get<(size_t)location_type::package_data_target>(this->data) == -1;
         case location_type::self:
            return false;
      }
      

      bool handled = false;
      bool result  = false;
      std::visit(
         [&handled, &result](auto& value) {
            using value_type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<value_type, form_reference_t>) {
               result  = !value;
               handled = true;
            } else if constexpr (std::is_same_v<value_type, std::monostate>) {
               result  = true;
               handled = true;
            }
         },
         this->data
      );
      if (handled)
         return result;

      return false;
   }

   void package_location::_clear_data(Form& my_owner) {
      switch (this->get_type()) {
         // Handle forms:
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
         case name: \
            std::get<(size_t)name>(this->data).set(my_owner, nullptr); \
            break;

         CASE(location_type::reference);
         CASE(location_type::interior_cell);
         CASE(location_type::object);
         CASE(location_type::linked_ref);
         #pragma pop_macro("CASE")
      }
   }
   void package_location::_emplace_data_for_type(location_type t) {
      if (this->get_type() == t)
         return;
      switch (t) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
         case name: this->data.emplace<(size_t)name>(); break;

         CASE(location_type::reference);
         CASE(location_type::interior_cell);
         CASE(location_type::near_package_start_location);
         CASE(location_type::near_editor_location);
         CASE(location_type::object);
         CASE(location_type::object_type);
         CASE(location_type::linked_ref);
         CASE(location_type::at_package_location);
         CASE(location_type::reference_alias);
         CASE(location_type::location_alias);
         CASE(location_type::interrupt_override_target);
         CASE((location_type)11);
         CASE(location_type::self);
         #pragma pop_macro("CASE")
      }
   }

   void package_location::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, Form& my_owner) {
      location_type t = (location_type)0;
      subrecord.read(t);
      this->_emplace_data_for_type(t);
      switch (t) {
         case location_type::reference:
            {
               auto& form = this->_as_type<location_type::reference>();
               if (subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::reference, subrecord);
            }
            break;
         case location_type::interior_cell:
            {
               auto& form = this->_as_type<location_type::interior_cell>();
               if (subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::cell, subrecord);
                  if (form && form.get_form_stub()->is_exterior_cell()) {
                     specific_load_warnings::target_is_exterior_cell notice(
                        intfc.target_stub,
                        *form.get_form_stub()
                     );
                     intfc.log_load_warning(notice);
                  }
               }
            }
            break;
         case location_type::object:
            {
               auto& form = this->_as_type<location_type::object>();
               if (subrecord.read(form))
                  ; // TODO: What form types are valid here?
            }
            break;
         case location_type::object_type:
            {
               auto& data = this->_as_type<location_type::object_type>();
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
         case location_type::linked_ref:
            {
               auto& form = this->_as_type<location_type::linked_ref>();
               if (subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::keyword, subrecord);
               }
            }
            break;
         case location_type::reference_alias:
            {
               auto& alias_id = this->_as_type<location_type::reference_alias>();
               subrecord.read(alias_id);
            }
            break;
         case location_type::location_alias:
            {
               auto& alias_id = this->_as_type<location_type::location_alias>();
               subrecord.read(alias_id);
            }
            break;
         case location_type::interrupt_override_target:
            {
               auto& data = this->_as_type<location_type::interrupt_override_target>();
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
         case location_type::package_data_target:
            {
               auto& packdata_id = this->_as_type<location_type::package_data_target>();
               subrecord.read(packdata_id);
            }
            break;

         case location_type::near_package_start_location:
         case location_type::near_editor_location:
         case location_type::at_package_location:
         case location_type::self:
         default:
            subrecord.skip_bytes(4);
            break;
      }
      subrecord.read(this->radius);
   }
   void package_location::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      const auto type = this->get_type();
      subrecord.write(type);
      switch (type) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
         case name: subrecord.write(std::get<(size_t)name>(this->data)); break;

         CASE(location_type::reference);
         CASE(location_type::interior_cell);
         CASE(location_type::object);
         CASE(location_type::object_type);
         CASE(location_type::linked_ref);
         CASE(location_type::reference_alias);
         CASE(location_type::location_alias);
         CASE(location_type::interrupt_override_target);
         #pragma pop_macro("CASE")

         case location_type::package_data_target:
            {
               int32_t data = std::get<(size_t)location_type::package_data_target>(this->data);
               subrecord.write(data);
            }
            break;

         default:
            subrecord.skip_bytes(4);
            break;
      }
      subrecord.write(this->radius);
   }

   void package_location::clone_from(const package_location& src, Form& my_owner) noexcept {
      this->_clear_data(my_owner);
      switch (src.get_type()) {
         // Handle forms:
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
            case name: this->data.emplace<(size_t)name>().set(my_owner, std::get<(size_t)name>(src.data)); break;
         CASE(location_type::reference);
         CASE(location_type::interior_cell);
         CASE(location_type::object);
         CASE(location_type::linked_ref);
         #pragma pop_macro("CASE")

         default:
            this->data = src.data;
      }
      this->radius = src.radius;
   }
   void package_location::unmanaged_clone_from(const package_location& src, load_order_interfaces::form_load&) noexcept {
      switch (src.get_type()) {
         // Handle forms:
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
            case name: this->data.emplace<(size_t)name>().unmanaged_set(std::get<(size_t)name>(src.data).get_form_stub()); break;
         CASE(location_type::reference);
         CASE(location_type::interior_cell);
         CASE(location_type::object);
         CASE(location_type::linked_ref);
         #pragma pop_macro("CASE")

         default:
            this->data = src.data;
      }
      this->radius = src.radius;
   }
   void package_location::clear(Form& my_owner) noexcept {
      this->_clear_data(my_owner);
      this->radius = 0;
   }
   void package_location::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      switch (this->get_type()) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
            case name: std::get<(size_t)name>(this->data).clear_if(my_owner, other); break;
         CASE(location_type::reference);
         CASE(location_type::interior_cell);
         CASE(location_type::object);
         CASE(location_type::linked_ref);
         #pragma pop_macro("CASE")
      }
   }

   #pragma region package_location::use_info_state
   void package_location::use_info_state::generate_use_info(tes_subrecord_reader& subrecord) {
      location_type t;
      if (subrecord.read(t)) {
         switch (t) {
            case location_type::reference:
            case location_type::interior_cell:
            case location_type::object:
            case location_type::linked_ref:
               subrecord.read(this->form);
               break;
         }
      }
   }
   void package_location::use_info_state::clear() {
      this->form = 0;
   }
   void package_location::use_info_state::commit_to(form_stub_use_info_builder& uib) {
      if (this->form)
         uib.add_outbound_reference(this->form);
   }
   #pragma endregion
}