#include "MagicEffect.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   /*static*/ void MagicEffect::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      uint32_t  keywordCount = 0;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'MDOB': // menu display object
            case 'ESCE': // counter effects
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               subrecord.skip_bytes(8); // flags, base cost
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(12); // skill, resistance, unknown
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(4); // taper weight
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(32); // skill level, area, casting time, taper curve, taper duration, second AV weight, effect type, primary AV
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(12);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(4); // skill usage mult
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(4); // dual cast scale
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(8);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               subrecord.skip_bytes(12);
               break;
            case 'SNDD':
               subrecord.skip_bytes(4);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'DNAM': // description
               break;
         }
      }
   }
}