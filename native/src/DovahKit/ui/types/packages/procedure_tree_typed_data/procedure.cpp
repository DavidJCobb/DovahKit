#include "./procedure.h"
#include "dovah/forms/structs/custom_packages/procedure_nodes/procedure.h"

namespace ui::types::packages::procedure_tree_typed_data {
   void procedure::importData(const backend_type& src) {
      this->type = src.type;
      this->flags = src.flags;
      this->flag_overrides = src.flag_overrides;
      this->parameter_unique_ids = src.parameter_unique_ids;
   }
   void procedure::exportData(backend_type& dst, dovah::loaded_forms::Form& dst_owner) const {
      dst.type = this->type;
      dst.flags = this->flags;
      dst.flag_overrides = this->flag_overrides;
      dst.parameter_unique_ids = this->parameter_unique_ids;
   }
}