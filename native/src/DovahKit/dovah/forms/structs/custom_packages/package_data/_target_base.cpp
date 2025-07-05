#include "./_target_base.h"
#include <memory>
#include "../../../_common_cpp.h"

#include "../../../../notices/form_load_warnings/by_form_type/package/package_data_unexpected_value_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs::custom_packages {
   /*virtual*/ void package_data_target_base::load_value(tes_record_reader& record, load_order_interfaces::form_load& intfc, const load_context& context) /*override*/ {
      auto& my_owner  = *intfc.target_stub.form;
      auto& subrecord = record.get_current_subrecord();
      switch (subrecord.signature()) {
         case package_target::subrecord_legacy:
         case package_target::subrecord_modern:
            this->data.load(subrecord, intfc, my_owner);
            break;
         default:
            specific_load_warnings::package_data_unexpected_value_subrecord notice(
               intfc.target_stub,
               context.which,
               this->get_type(),
               subrecord.signature()
            );
            intfc.log_load_warning(notice);
            return;
      }
      record.next_subrecord();
   }
   /*static*/ void package_data_target_base::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      switch (subrecord.signature()) {
         case package_target::subrecord_legacy:
         case package_target::subrecord_modern:
            break;
         default:
            return;
      }

      package_target::use_info_state state;
      state.generate_use_info(subrecord);
      state.commit_to(uib);

      record.next_subrecord();
   };
   /*virtual*/ void package_data_target_base::save_value(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(package_target::subrecord_modern);
      this->data.save(subrecord, intfc);
      subrecord.close();
   }
   /*virtual*/ void package_data_target_base::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->data.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void package_data_target_base::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->data.clear(my_owner);
   }
}