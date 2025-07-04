#include "./branch.h"
#include "../../../_common_cpp.h"

#include "../../../../notices/form_save_errors/by_form_type/package/too_many_procedure_tree_branch_children.h"

namespace {
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_type::package;
   }
}

namespace dovah::loaded_forms::structs::custom_packages::procedure_node_data {
   size_t branch::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      size_t child_count = 0;
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == subrecord_branch) {
            uint32_t count = 0;
            subrecord.read(count);
            subrecord.read(this->flags);
            child_count = count;
            record.next_subrecord();
         }
      }
      return child_count;
   }
   /*static*/ void branch::generate_use_info(tes_record_reader& record, form_stub_use_info_builder&) {
      //
      // We don't actually contain any uses, but we need to skip the right number 
      // of subrecords.
      //
      if (record.get_current_subrecord().signature() == subrecord_branch)
         record.next_subrecord();
   }
   void branch::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (this->children.size() > max_children) {
         auto notice = specific_save_errors::too_many_procedure_tree_branch_children(
            *intfc.target_stub,
            this->children.size()
         );
         intfc.throw_save_error(notice);
      }
      auto& subrecord = record.open_next_subrecord(subrecord_branch);
      subrecord.write((uint32_t)this->children.size());
      subrecord.write(this->flags);
      subrecord.close();
   }
}