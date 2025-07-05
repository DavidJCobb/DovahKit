#include "./custom.h"
#include <memory>
#include "../../_common_cpp.h"

#include "dovah/notices/form_save_errors/by_form_type/package/too_many_package_data_values.h"

namespace {
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_type::package;
   }
}

#include "../custom_packages/procedure_node.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void custom::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      //
      // Load header.
      //
      uint32_t package_data_count = 0;
      subrecord.read(package_data_count);
      if (auto& form = this->template_package; subrecord.read(form)) {
         intfc.warn_if_ref_is_wrong_type(form, form_type::package, subrecord.signature());
      }
      subrecord.read(this->revision);
      //
      // NOTE: If the revision number can't be stored in a uint16_t without truncation, 
      // then the Creation Kit will warn about it.
      //
      record.next_subrecord();

      this->data.values.load(record, intfc, package_data_count);

      if (!this->template_package) {
         this->procedures.load(record, intfc);
         this->data.declarations.load(record, intfc);
      }
   }
   /*static*/ void custom::generate_header_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      form_id_t template_package;

      //
      // Load header.
      //
      uint32_t package_data_count = 0;
      subrecord.read(package_data_count);
      subrecord.read(template_package);
      record.next_subrecord();

      custom_packages::package_data_value_map::generate_use_info(record, package_data_count, uib);
      if (template_package) {
         custom_packages::procedure_tree::generate_use_info(record, uib);
         custom_packages::package_data_declaration_map::generate_use_info(record, uib);
      }
   }
   /*virtual*/ void custom::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      if (this->data.values.entries.size() > custom_packages::package_data_value_map::max_serializable_count) {
         auto notice = specific_save_errors::too_many_package_data_values(
            *intfc.target_stub,
            this->data.values.entries.size()
         );
         intfc.throw_save_error(notice);
      }

      auto& subrecord = record.open_next_subrecord(header_subrecord);
      subrecord.write((uint32_t)this->data.values.entries.size());
      subrecord.write(this->template_package);
      subrecord.write(this->revision);
      subrecord.close();

      this->data.values.save(record, intfc);
      if (this->template_package) {
         //
         // Packages that use a template cannot declare their own packdata nor store 
         // their own procedure trees. We want to explicitly clear them now, so that 
         // we don't end up with "phantom use info" (e.g. if we save the form, unload 
         // it, and then reload it).
         //
         auto& my_owner = *intfc.target_stub->form;
         this->procedures.clear(my_owner);
         this->data.declarations = {};
      } else {
         this->procedures.save(record, intfc);
         this->data.declarations.save(record, intfc);
      }
   }
   /*virtual*/ base* custom::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<custom>();
      auto* copy     = copy_ptr.get();

      copy->template_package.set(owner_of_clone, this->template_package);
      copy->revision = this->revision;

      copy->data.values.clone_from(this->data.values, owner_of_clone);
      copy->data.declarations = this->data.declarations;
      copy->procedures.clone_from(this->procedures, owner_of_clone);

      return copy_ptr.release();
   }
   /*virtual*/ void custom::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->template_package.clear_if(my_owner, other);
      this->data.values.sever_outbound_references_to(other, my_owner);
      this->procedures.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void custom::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->template_package.set(my_owner, nullptr);

      this->data.values.clear(my_owner);
      this->data.declarations = {};
      this->procedures.clear(my_owner);
   }
}