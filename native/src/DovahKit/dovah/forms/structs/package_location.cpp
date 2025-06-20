#include "./package_location.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::structs {
   package_location_type package_location::get_type() const {
      return (package_location_type)this->data.index();
   }
   void package_location::set_type(Form& my_owner, package_location_type t) {
      auto prior = this->get_type();
      if (prior == t)
         return;
      this->_clear_data(my_owner);
      this->_emplace_data_for_type(t);
   }

   void package_location::_clear_data(Form& my_owner) {
      switch (this->get_type()) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
         case name: \
            std::get<(size_t)name>(this->data).set(my_owner, nullptr); \
            break;

         CASE(package_location_type::near_reference);
         CASE(package_location_type::in_cell);
         CASE(package_location_type::object_id);
         CASE(package_location_type::near_linked_reference);
         #pragma pop_macro("CASE")
      }
   }
   void package_location::_emplace_data_for_type(package_location_type t) {
      if (this->get_type() == t)
         return;
      switch (t) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
         case name: this->data.emplace<(size_t)name>(); break;

         CASE(package_location_type::near_reference);
         CASE(package_location_type::in_cell);
         CASE(package_location_type::near_package_start_location);
         CASE(package_location_type::near_editor_location);
         CASE(package_location_type::object_id);
         CASE(package_location_type::object_type);
         CASE(package_location_type::near_linked_reference);
         CASE(package_location_type::at_package_location);
         CASE(package_location_type::reference_alias);
         CASE(package_location_type::location_alias);
         CASE((package_location_type)10);
         CASE((package_location_type)11);
         CASE(package_location_type::near_self);
         #pragma pop_macro("CASE")
      }
   }

   void package_location::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      package_location_type t = (package_location_type)0;
      subrecord.read(t);
      this->_emplace_data_for_type(t);
      switch (t) {
         case package_location_type::near_reference:
            {
               auto& form = std::get<(size_t)package_location_type::near_reference>(this->data);
               if (subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::reference, subrecord);
               }
            }
            break;
         case package_location_type::in_cell:
            {
               auto& form = std::get<(size_t)package_location_type::in_cell>(this->data);
               if (subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::cell, subrecord);
               }
            }
            break;
         case package_location_type::object_id:
            {
               auto& form = std::get<(size_t)package_location_type::object_id>(this->data);
               if (subrecord.read(form)) {
                  ; // TODO: What form types are valid here?
               }
            }
            break;
         case package_location_type::object_type:
            {
               auto& v = std::get<(size_t)package_location_type::object_type>(this->data);
               if (subrecord.read(v)) {
                  // TODO: Warn if v is out of bounds
               }
            }
            break;
         case package_location_type::near_linked_reference:
            {
               auto& form = std::get<(size_t)package_location_type::near_linked_reference>(this->data);
               if (subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::keyword, subrecord);
               }
            }
            break;
         case package_location_type::reference_alias:
            {
               auto& alias_id = std::get<(size_t)package_location_type::reference_alias>(this->data);
               subrecord.read(alias_id);
            }
            break;
         case package_location_type::location_alias:
            {
               auto& alias_id = std::get<(size_t)package_location_type::location_alias>(this->data);
               subrecord.read(alias_id);
            }
            break;

         case package_location_type::near_package_start_location:
         case package_location_type::near_editor_location:
         case package_location_type::at_package_location:
         case (package_location_type)10: // TODO: identify this and confirm it has no data
         case (package_location_type)11: // TODO: identify this and confirm it has no data
         case package_location_type::near_self:
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
         case package_location_type::near_reference:
            subrecord.write(std::get<(size_t)package_location_type::near_reference>(this->data));
            break;
         case package_location_type::in_cell:
            subrecord.write(std::get<(size_t)package_location_type::in_cell>(this->data));
            break;
         case package_location_type::object_id:
            subrecord.write(std::get<(size_t)package_location_type::object_id>(this->data));
            break;
         case package_location_type::object_type:
            subrecord.write(std::get<(size_t)package_location_type::object_type>(this->data));
            break;
         case package_location_type::near_linked_reference:
            subrecord.write(std::get<(size_t)package_location_type::near_linked_reference>(this->data));
            break;
         case package_location_type::reference_alias:
            subrecord.write(std::get<(size_t)package_location_type::reference_alias>(this->data));
            break;
         case package_location_type::location_alias:
            subrecord.write(std::get<(size_t)package_location_type::location_alias>(this->data));
            break;

         case package_location_type::near_package_start_location:
         case package_location_type::near_editor_location:
         case package_location_type::at_package_location:
         case (package_location_type)10: // TODO: identify this and confirm it has no data
         case (package_location_type)11: // TODO: identify this and confirm it has no data
         case package_location_type::near_self:
         default:
            subrecord.skip_bytes(4);
            break;
      }
      subrecord.write(this->radius);
   }

   void package_location::clone_from(const package_location& src, Form& my_owner) noexcept {
      this->_clear_data(my_owner);
      switch (src.get_type()) {
         #pragma push_macro("CASE")
         #undef CASE
         #define CASE(name) \
            case name: this->data.emplace<(size_t)name>().set(my_owner, std::get<(size_t)name>(src.data)); break;
         CASE(package_location_type::near_reference);
         CASE(package_location_type::in_cell);
         CASE(package_location_type::object_id);
         CASE(package_location_type::near_linked_reference);
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
         CASE(package_location_type::near_reference);
         CASE(package_location_type::in_cell);
         CASE(package_location_type::object_id);
         CASE(package_location_type::near_linked_reference);
         #pragma pop_macro("CASE")
      }
   }

   #pragma region package_location::use_info_state
   void package_location::use_info_state::generate_use_info(tes_subrecord_reader& subrecord) {
      package_location_type t;
      if (subrecord.read(t)) {
         switch (t) {
            case package_location_type::near_reference:
            case package_location_type::in_cell:
            case package_location_type::object_id:
            case package_location_type::near_linked_reference:
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