#include "_use_info.h"
#include "../../../form_stub_use_info_builder.h"

namespace dovah {
   void extra_data_use_info_state::clear() {
      for (auto& id : this->list)
         id = 0;
   }
   void extra_data_use_info_state::commit_to(form_stub_use_info_builder& uib) {
      for (auto id : this->list)
         uib.add_outbound_reference(id);
   }
}