#include "destruction.h"
#include "../_common_cpp.h"
#include "../Form.h" // for LOAD_NAIVELY_WHEN_THE_GAME_DOES directive

namespace dovah::loaded_forms::components {
   void destruction_stage_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      uint32_t stageCount = 0;
      #if LOAD_NAIVELY_WHEN_THE_GAME_DOES == 1
         auto& record = subrecord.get_containing_record();
         for (; subrecord.exists() && subrecord.signature() != 'DSTF'; record.next_subrecord()) {
            switch (subrecord.signature()) {
               case 'DEST':
                  subrecord.read(this->health);
                  subrecord.read(stageCount);
                  subrecord.read(this->flags);
                  subrecord.skip_bytes(2);
                  if (stageCount)
                     this->stages.reserve(stageCount);
                  break;
               case 'DSTD':
                  {
                     auto& stage = this->stages.emplace_back();
                     subrecord.read(stage.healthPercent);
                     subrecord.read(stage.damageStage);
                     subrecord.read(stage.flags);
                     subrecord.read(stage.selfDamageRate);
                     subrecord.read(stage.explosion);
                     subrecord.read(stage.debris);
                     subrecord.read(stage.debrisCount);
                     //
                     const auto& stub = intfc.target_stub;
                     auto index = this->stages.size() - 1;
                     intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::explosion, stub, stage.explosion)
                           .set_subrecord_index(index)
                     );
                     intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::debris, stub, stage.debris)
                           .set_subrecord_index(index)
                     );
                  }
                  break;
               case 'DMDL':
               case 'DMDT':
               case 'DMDS':
                  if (this->stages.size()) {
                     auto& stage = *this->stages.rbegin();
                     stage.replacementModel.load(subrecord);
                  }
                  break;
            }
         }
      #else
         switch (subrecord.signature()) {
            case 'DEST':
               subrecord.read(this->health);
               subrecord.read(stageCount);
               subrecord.read(this->flags);
               subrecord.skip_bytes(2);
               if (stageCount)
                  this->stages.reserve(stageCount);
               break;
            case 'DSTD':
               {
                  auto& stage = this->stages.emplace_back();
                  subrecord.read(stage.healthPercent);
                  subrecord.read(stage.damageStage);
                  subrecord.read(stage.flags);
                  subrecord.read(stage.selfDamageRate);
                  subrecord.read(stage.explosion);
                  subrecord.read(stage.debris);
                  subrecord.read(stage.debrisCount);
                  //
                  const auto& stub = intfc.target_stub;
                  auto index = this->stages.size() - 1;
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::explosion, stub, stage.explosion)
                        .set_subrecord_index(index)
                  );
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::debris, stub, stage.debris)
                        .set_subrecord_index(index)
                  );
               }
               break;
            case 'DMDL':
            case 'DMDT':
            case 'DMDS':
               if (this->stages.size()) {
                  auto& stage = *this->stages.rbegin();
                  stage.replacementModel.load(subrecord, intfc);
               }
               break;
            case 'DSTF': // end marker
               break;
         }
      #endif
   }
   /*static*/ void destruction_stage_data::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            break;
         case 'DSTD': // destruction stage data
            subrecord.skip_bytes(8);
            if (subrecord.read(formID)) // explosion
               uib.add_outbound_reference(formID);
            if (subrecord.read(formID)) // debris
               uib.add_outbound_reference(formID);
            // remaining four bytes don't matter
            break;
         case 'DMDL': // destruction stage model
         case 'DMDT': // 
         case 'DMDS': // 
            model::generate_use_info(subrecord, uib);
            break;
         case 'DSTF': // destruction stage end marker
            break;
      }
   }
   void destruction_stage_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& DEST = record.open_next_subrecord('DEST');
      DEST.write(this->health);
      DEST.write(uint32_t(this->stages.size()));
      DEST.write(this->flags);
      DEST.skip_bytes(2);
      DEST.close();
      //
      for (auto& stage : this->stages) {
         auto& DSTD = record.open_next_subrecord('DSTD');
         DSTD.write(stage.healthPercent);
         DSTD.write(stage.damageStage);
         DSTD.write(stage.flags);
         DSTD.write(stage.selfDamageRate);
         DSTD.write(stage.explosion);
         DSTD.write(stage.debris);
         DSTD.write(stage.debrisCount);
         DSTD.close();
         //
         auto& model = stage.replacementModel;
         model.save(record, intfc, 'DMDL', 'DMDT', 'DMDS');
      }
      if (!this->stages.empty()) {
         auto& DSTF = record.open_next_subrecord('DSTF');
         DSTF.close();
      }
   }
   void destruction_stage_data::clone_from(const destruction_stage_data& other, loaded_forms::Form& my_owner) noexcept {
      this->health = other.health;
      this->flags  = other.flags;
      //
      size_t size = other.stages.size();
      if (!this->stages.empty()) {
         for (auto& stage : this->stages) {
            stage.explosion.set(my_owner, nullptr);
            stage.debris.set(my_owner, nullptr);
            stage.replacementModel.clear(my_owner);
         }
         this->stages.clear();
      }
      this->stages.resize(size);
      //
      for (size_t i = 0; i < size; ++i) {
         auto& stage = this->stages[i];
         auto& from  = other.stages[i];
         stage.healthPercent  = from.healthPercent;
         stage.damageStage    = from.damageStage;
         stage.flags          = from.flags;
         stage.selfDamageRate = from.selfDamageRate;
         stage.explosion.set(my_owner, from.explosion);
         stage.debris.set(my_owner, from.debris);
         stage.debrisCount    = from.debrisCount;
         stage.replacementModel.clone_from(from.replacementModel, my_owner);
      }
   }
   void destruction_stage_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto& stage : this->stages) {
         stage.debris.clear_if(my_owner, target);
         stage.explosion.clear_if(my_owner, target);
      }
   }
   void destruction_stage_data::clear(loaded_forms::Form& my_owner) {
      for (auto& stage : this->stages) {
         stage.debris.set(my_owner, nullptr);
         stage.explosion.set(my_owner, nullptr);
         stage.replacementModel.clear(my_owner);
      }
      this->stages.clear();
   }
}