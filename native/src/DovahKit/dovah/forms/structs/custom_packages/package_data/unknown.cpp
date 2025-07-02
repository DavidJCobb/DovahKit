#include "./unknown.h"
#include <memory>
#include "../../../_common_cpp.h"

namespace dovah::loaded_forms::structs::custom_packages {
   /*virtual*/ void package_data_unknown::load_value(tes_record_reader& record, load_order_interfaces::form_load& intfc, const load_context& context) /*override*/ {
   }
   /*static*/ void package_data_unknown::generate_use_info(tes_record_reader& record, std::vector<form_id_t>& out) {
   };
   /*virtual*/ void package_data_unknown::save_value(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
   }
   /*virtual*/ package_data* package_data_unknown::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<package_data_unknown>();
      auto* copy = copy_ptr.get();

      return copy_ptr.release();
   }
   /*virtual*/ void package_data_unknown::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
   }
   /*virtual*/ void package_data_unknown::clear(loaded_forms::Form& my_owner) /*override*/ {
   }
}