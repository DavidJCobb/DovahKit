#include "./subrecord_data_position.h"
#include "../files/tes_file_reading/elements.h"
#include "../load_order_interfaces/form_load.h"

namespace dovah::utils {
   /*static*/ subrecord_data_position subrecord_data_position::from_loader(tes_file_reading::subrecord& subrecord, load_order_interfaces::form_load& intfc) {
      auto& record    = subrecord.get_containing_record();
      auto  pos_prior = subrecord.offset();

      subrecord_data_position out;

      out.source_file = intfc.current_file;
      subrecord.back_to_start(); // HACK: no accessor for subrecord start pos, so rewind to start of subrecord body and use record.current_offset()
      out.offsets = {
         .of_record    = record.header_pos(),
         .of_subrecord = record.current_offset(),
      };
      subrecord.skip_bytes(pos_prior); // HACK: undo the effects of the above HACK

      return out;
   }
}