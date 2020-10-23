#include "ActorBase.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ActorBase::load(tes_record_reader& record) {
      Form::load(record);
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'FULL':
               subrecord.to_string(this->name);
               break;
         }
      }
   }
   /*static*/ void ActorBase::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
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
            case 'RNAM': // race
            case 'TPLT': // template actor base
            case 'VTCK': // voicetype
            case 'INAM': // death item
            case 'SNAM': // faction (has four more bytes, but we don't need them)
            case 'SPLO': // spell
            case 'WNAM': // skin
            case 'ANAM': // far-away skin
            case 'ATKR': // attack race
            case 'SPOR': // spectator package override
            case 'OCOR': // observe corpse package override
            case 'GWOR': // guard worn package override
            case 'ECOR': // combat package override
            case 'PRKR': // perk
            case 'PKID': // package
            case 'CNAM': // class
            case 'PNAM': // head part
            case 'HCLF': // hair color form
            case 'ZNAM': // combat style
            case 'GNAM': // gift filter list
            case 'CSDI': // sound
            case 'CSCR': // audio template
            case 'DOFT': // default outfit
            case 'SOFT': // sleep outfit
            case 'DPLT': // default package list
            case 'CRIF': // crime faction
            case 'FTST': // face textureset
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'CNTO':
            case 'COED':
               components::container_data::generate_use_info(subrecord, uib);
               break;
            case 'ATKD': // attack data
               subrecord.skip_bytes(8);
               if (subrecord.read(formID)) // attack spell
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(16);
               if (subrecord.read(formID)) // attack type
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(12);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, uib);
               break;
            //
            // End of destruction stage fields.
            //
            case 'ACBS': // character base stats
            case 'ATKE': // attack event
            case 'SPCT': // spell count
            case 'PRKZ': // perk count
            case 'COCT': // item count ("count of container")
            case 'AIDT': // AI data
            case 'FULL': // full name
            case 'SHRT': // short name
            case 'DATA': // marker for DNAM position
            case 'DNAM': // skill/stat data
            case 'NAM5': // unknown two-byte int
            case 'NAM6': // height
            case 'NAM7': // weight
            case 'NAM8': // sound level
            case 'CSDT': // sound type
            case 'CSDC': // sound chance
            case 'QNAM': // skin tone
            case 'NAM9': // face morphs values
            case 'NAMA': // face part integers
            case 'TINI': // tint item
            case 'TINC': // tint color
            case 'TINV': // tint value
            case 'TIAS': // unknown two-byte int
               break;
         }
      }
   }
}