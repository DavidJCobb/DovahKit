#include "MagicEffect.h"
#include "_common_cpp.h"

#include "../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_forms, "The backend for MagicEffect is incomplete (missing everything except the loader).");

namespace dovah::loaded_forms {
   /*static*/ void MagicEffect::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      form_id_t related;        // DATA+0x08
      form_id_t light;          // DATA+0x18
      form_id_t hit_shader;     // DATA+0x20
      form_id_t enchant_shader; // DATA+0x24
      form_id_t projectile;     // DATA+0x48
      form_id_t explosion;      // DATA+0x4C
      form_id_t casting_art;    // DATA+0x5C
      form_id_t hit_effect;     // DATA+0x60
      form_id_t impact_data;    // DATA+0x64
      form_id_t dual_cast;      // DATA+0x6C
      form_id_t enchant_art;    // DATA+0x74
      form_id_t equip_ability;  // DATA+0x80
      form_id_t imagespace_mod; // DATA+0x84
      form_id_t perk;           // DATA+0x88
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
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               subrecord.skip_bytes(8); // flags, base cost
               subrecord.read(related);
               subrecord.skip_bytes(12); // skill, resistance, unknown
               subrecord.read(light);
               subrecord.skip_bytes(4); // taper weight
               subrecord.read(hit_shader);
               subrecord.read(enchant_shader);
               subrecord.skip_bytes(32); // skill level, area, casting time, taper curve, taper duration, second AV weight, effect type, primary AV
               subrecord.read(projectile);
               subrecord.read(explosion);
               subrecord.skip_bytes(12);
               subrecord.read(casting_art);
               subrecord.read(hit_effect);
               subrecord.read(impact_data);
               subrecord.skip_bytes(4); // skill usage mult
               subrecord.read(dual_cast);
               subrecord.skip_bytes(4); // dual cast scale
               subrecord.read(enchant_art);
               subrecord.skip_bytes(8);
               subrecord.read(equip_ability);
               subrecord.read(imagespace_mod);
               subrecord.read(perk);
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
      uib.add_outbound_reference(related);        // DATA+0x08
      uib.add_outbound_reference(light);          // DATA+0x18
      uib.add_outbound_reference(hit_shader);     // DATA+0x20
      uib.add_outbound_reference(enchant_shader); // DATA+0x24
      uib.add_outbound_reference(projectile);     // DATA+0x48
      uib.add_outbound_reference(explosion);      // DATA+0x4C
      uib.add_outbound_reference(casting_art);    // DATA+0x5C
      uib.add_outbound_reference(hit_effect);     // DATA+0x60
      uib.add_outbound_reference(impact_data);    // DATA+0x64
      uib.add_outbound_reference(dual_cast);      // DATA+0x6C
      uib.add_outbound_reference(enchant_art);    // DATA+0x74
      uib.add_outbound_reference(equip_ability);  // DATA+0x80
      uib.add_outbound_reference(imagespace_mod); // DATA+0x84
      uib.add_outbound_reference(perk);           // DATA+0x88
   }
}