#include "water_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result water_data::load(tes_subrecord_reader& subrecord) {
      //
      // NOTE: Skyrim Classic, upon encountering XWCN, will open the next subrecord 
      // during the extra-data handling and load (or not) data from it. This means 
      // that the next subrecord, *whatever signature it has*, is considered part 
      // of ExtraWaterData.
      //
      auto s = subrecord.signature();
      if (s == signature_base) {
         uint32_t data;
         subrecord.read(data);
         /*
         if (data) {
            open next record;
            read (data * 4) bytes;
         }
         */
         return load_result::succeeded;
      }
      return load_result::unrecognized;
   }
   void water_data::save(tes_record_writer& record) {
      //
      // NOTE: We should write our subrecords even if the values match the defaults, 
      // because we can't tell whether we're writing to a new form or to an override. 
      // If the user wants to overwrite a record that has non-default poison settings 
      // in order to return them to their defaults, then we need to make sure that 
      // that works.
      //
      record.write_formID_subrecord(signature_type, this->type);
      auto& XPSC = record.open_next_subrecord(signature_dose);
      XPSC.write(this->doses);
      XPSC.close();
   }
}