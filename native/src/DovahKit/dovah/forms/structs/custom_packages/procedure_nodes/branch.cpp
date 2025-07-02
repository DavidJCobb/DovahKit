#include "./branch.h"
#include "../../../_common_cpp.h"

namespace dovah::loaded_forms::structs::custom_packages::procedure_node_data {
   void branch::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == subrecord_branch) {
            uint32_t count = 0;
            subrecord.read(count);
            subrecord.read(this->flags);
            this->children.resize(count);
         }
      }
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