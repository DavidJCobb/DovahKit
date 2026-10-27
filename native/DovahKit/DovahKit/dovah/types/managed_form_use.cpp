#include "./managed_form_use.h"
#include "dovah/active_load_order.h"
#include "dovah/form_stub_collection.h"
#include "dovah/tes/file.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "./form_id.h"

namespace dovah {
   void managed_form_use::set(form_stub& containing_form, form_stub* value) {
      if (this->_used == value)
         return;
      containing_form._replace_outbound_use({}, this->_used, value, 0);
   }
   void managed_form_use::_set_with_flags(form_stub& containing_form, form_stub* value, use_info::entry_flag_underlying_type mask) {
      if (this->_used == value)
         return;
      containing_form._replace_outbound_use({}, this->_used, value, mask);
   }

   bool managed_form_use::stream(tes::subrecord_view& subrecord) {
      if (!subrecord.is_in_bounds(sizeof(bare_form_id)))
         return false;
      this->unchecked_stream(subrecord);
      return true;
   }
   void managed_form_use::unchecked_stream(tes::subrecord_view& subrecord) {
      form_id id;
      subrecord.unchecked_read(id);
      auto* alo_ptr = subrecord.get_source_file().get_active_load_order();
      assert(!!alo_ptr);
      auto& fsc = alo_ptr->get_form_stub_collection();
      this->_used = fsc.get_form_by_id((bare_form_id)id);
   }
}