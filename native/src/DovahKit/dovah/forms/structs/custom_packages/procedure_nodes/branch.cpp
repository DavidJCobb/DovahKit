#include "./branch.h"
#include "../../../_common_cpp.h"

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
         static_assert(false, "TODO: Save error");
      }
      auto& subrecord = record.open_next_subrecord(subrecord_branch);
      subrecord.write((uint32_t)this->children.size());
      subrecord.write(this->flags);
      subrecord.close();
   }
}