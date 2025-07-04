#include "./int.h"
#include <memory>
#include "../../../_common_cpp.h"

#include "../../../../notices/form_load_warnings/by_form_type/package/package_data_unexpected_value_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs::custom_packages {
   /*virtual*/ void package_data_int::load_value(tes_record_reader& record, load_order_interfaces::form_load& intfc, const load_context& context) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != 'CNAM') {
         specific_load_warnings::package_data_unexpected_value_subrecord notice(
            intfc.target_stub,
            context.which,
            this->get_type(),
            subrecord.signature()
         );
         intfc.log_load_warning(notice);
         return;
      }
      subrecord.read(this->value);
      record.next_subrecord();
   }
   /*static*/ void package_data_int::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
   };
   /*virtual*/ void package_data_int::save_value(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord('CNAM');
      subrecord.write(this->value);
      subrecord.close();
   }
   /*virtual*/ package_data* package_data_int::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<package_data_int>();
      auto* copy = copy_ptr.get();

      copy->value = this->value;

      return copy_ptr.release();
   }
   /*virtual*/ void package_data_int::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
   }
   /*virtual*/ void package_data_int::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->value = false;
   }
}