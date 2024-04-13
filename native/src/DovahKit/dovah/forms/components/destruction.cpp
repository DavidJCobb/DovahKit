#include "destruction.h"
#include "../_common_cpp.h"
#include "../Form.h" // for LOAD_NAIVELY_WHEN_THE_GAME_DOES directive
#include "../../notice_code_list.h"

namespace dovah::loaded_forms::components {
   void destruction_stage_data::use_info_builder::done() {
      for (auto& stage : this->per_stage) {
         if (stage.debris)
            this->owner.add_outbound_reference(stage.debris);
         if (stage.explosion)
            this->owner.add_outbound_reference(stage.explosion);
      }
   }

   void destruction_stage_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      constexpr const auto no_stage = std::numeric_limits<size_t>::max();

      size_t  last_loaded_stage  = no_stage;
      size_t  nth_dstd_subrecord = 0; // for error reporting

      auto _handle_dest = [this, &subrecord, &intfc]() {
         if (subrecord.size() != 8) {
            //
            // Game doesn't read anything unless the subrecord is exactly 8 bytes.
            //
            this->stages.clear();
            return;
         }
         uint8_t stage_count = 0;
         subrecord.unchecked_read(this->health);
         subrecord.unchecked_read(stage_count);
         subrecord.unchecked_read(this->flags);
         subrecord.skip_bytes(2);
         this->stages.resize(stage_count);
      };
      auto _handle_dstd = [this, &last_loaded_stage, &nth_dstd_subrecord, &subrecord, &intfc]() {
         uint8_t stage_index = 0;

         decltype(Stage::healthPercent) health_percent = 0;
         subrecord.read(health_percent);
         subrecord.read(stage_index);
         last_loaded_stage = stage_index;
         //
         if (stage_index >= this->stages.size()) {
            detailed_notice warning;
            warning.code = notice_code::destruction_stage_serialized_index_out_of_bounds;
            warning.set_cause_subrecord(subrecord.signature());
            warning.set_subrecord_index(nth_dstd_subrecord);
            warning.extra_integers[0] = stage_index;
            warning.extra_integers[1] = this->stages.size();
            intfc.log_load_warning(warning);
            //
            ++nth_dstd_subrecord;
            return;
         }
         auto& stage = this->stages[stage_index];
         stage.healthPercent = health_percent;
         //
         subrecord.read(stage.damageStage);
         subrecord.read(stage.flags);
         subrecord.read(stage.selfDamageRate);
         subrecord.read(stage.explosion);
         subrecord.read(stage.debris);
         subrecord.read(stage.debrisCount);
                  
         const auto& stub = intfc.target_stub;
         intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
            detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::explosion, stub, stage.explosion)
               .set_subrecord_index(nth_dstd_subrecord)
         );
         intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
            detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::debris, stub, stage.debris)
               .set_subrecord_index(nth_dstd_subrecord)
         );

         ++nth_dstd_subrecord;
      };

      #if LOAD_NAIVELY_WHEN_THE_GAME_DOES == 1
         auto& record = subrecord.get_containing_record();
         if (subrecord.signature() == 'DEST') {
            _handle_dest();
            return;
         }
         if (subrecord.signature() == 'DSTD') {
            _handle_dstd();
            for (; subrecord.exists() && subrecord.signature() != 'DSTF'; record.next_subrecord()) {
               switch (subrecord.signature()) {
                  case 'DMDL':
                  case 'DMDT':
                  case 'DMDS':
                     if (last_loaded_stage < this->stages.size()) {
                        auto& stage = this->stages[last_loaded_stage];
                        stage.replacementModel.load(subrecord, intfc);
                     }
                     break;
               }
            }
            last_loaded_stage = no_stage;
         }
      #else
         switch (subrecord.signature()) {
            case 'DEST':
               _handle_dest();
               break;
            case 'DSTD':
               _handle_dstd();
               break;
            case 'DMDL':
            case 'DMDT':
            case 'DMDS':
               if (last_loaded_stage < this->stages.size()) {
                  auto& stage = this->stages[last_loaded_stage];
                  stage.replacementModel.load(subrecord, intfc);
               }
               break;
            case 'DSTF': // end marker
               last_loaded_stage = no_stage;
               break;
         }
      #endif
   }
   /*static*/ void destruction_stage_data::generate_use_info(tes_subrecord_reader& subrecord, use_info_builder& uib) {
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            if (subrecord.size() != 8) {
               uib.per_stage.clear();
               break;
            }
            subrecord.skip_bytes(4);
            {
               uint8_t stage_count;
               subrecord.unchecked_read(stage_count);
               if (stage_count) {
                  uib.per_stage.clear();
                  uib.per_stage.resize(stage_count);
               }
            }
            // remaining bytes don't matter
            break;
         case 'DSTD': // destruction stage data
            {
               uint8_t stage_index;
               subrecord.skip_bytes(1);
               if (subrecord.read(stage_index)) {
                  if (stage_index >= uib.per_stage.size())
                     break;
                  subrecord.skip_bytes(6);

                  auto& dst = uib.per_stage[stage_index];
                  //
                  if (subrecord.read(formID)) // explosion
                     dst.explosion = formID;
                  if (subrecord.read(formID)) // debris
                     dst.debris = formID;
               }
            }
            // remaining bytes don't matter
            break;
         case 'DMDL': // destruction stage model
         case 'DMDT': // 
         case 'DMDS': // 
            model::generate_use_info(subrecord, uib.owner);
            break;
         case 'DSTF': // destruction stage end marker
            break;
      }
   }
   void destruction_stage_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto stage_count = this->stages.size();
      if (stage_count > std::numeric_limits<uint8_t>::max()) {
         detailed_notice error;
         error.code = notice_code::too_many_destruction_stages_to_save;
         error.extra_integers[0] = stage_count;
         error.extra_integers[1] = std::numeric_limits<uint8_t>::max();
         intfc.set_save_error(error);
         return;
      }
      auto& DEST = record.open_next_subrecord('DEST');
      DEST.write(this->health);
      DEST.write(uint8_t(stage_count));
      DEST.write(this->flags);
      DEST.skip_bytes(2);
      DEST.close();
      //
      for (size_t i = 0; i < this->stages.size(); ++i) {
         auto& stage = this->stages[i];

         auto& DSTD = record.open_next_subrecord('DSTD');
         DSTD.write(stage.healthPercent);
         DSTD.write((uint8_t)i);
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
      auto& DSTF = record.open_next_subrecord('DSTF');
      DSTF.close();
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