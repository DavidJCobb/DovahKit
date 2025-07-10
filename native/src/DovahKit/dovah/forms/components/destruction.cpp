#include "destruction.h"
#include "../_common_cpp.h"
#include "../Form.h" // for LOAD_NAIVELY_WHEN_THE_GAME_DOES directive

#include "../../notices/form_load_warnings/by_form_component/destruction/stage_serialized_index_out_of_bounds.h"
#include "../../notices/form_save_errors/by_form_component/destruction/too_many_stages.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::destruction;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_component::destruction;
   }
}

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
      if (!intfc.is_winning_record)
         return;

      auto& last_loaded_stage  = this->_load_state.in_stage;
      auto& nth_dstd_subrecord = this->_load_state.nth_dstd_subrecord; // for error reporting

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
            specific_load_warnings::stage_serialized_index_out_of_bounds notice(
               const_cast<form_stub&>(intfc.target_stub),
               nth_dstd_subrecord,
               stage_index,
               this->stages.size()
            );
            intfc.log_load_warning(notice);
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
         intfc.warn_if_ref_is_wrong_type(stage.explosion, form_type::explosion, subrecord, { .nth_reference = stage_index });
         intfc.warn_if_ref_is_wrong_type(stage.debris,    form_type::debris,    subrecord, { .nth_reference = stage_index });

         ++nth_dstd_subrecord;
      };

      #if LOAD_NAIVELY_WHEN_THE_GAME_DOES == 1
         auto& record = subrecord.get_containing_record();
         if (subrecord.signature() == subrecord_header) {
            _handle_dest();
            return;
         }
         if (subrecord.signature() == subrecord_stage_data) {
            _handle_dstd();
            for (; subrecord.exists() && subrecord.signature() != subrecord_terminator; record.next_subrecord()) {
               switch (subrecord.signature()) {
                  case subrecord_model_path:
                  case subrecord_model_hashes:
                  case subrecord_model_swaps:
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
            case subrecord_header:
               _handle_dest();
               break;
            case subrecord_stage_data:
               _handle_dstd();
               break;
            case subrecord_model_path:
            case subrecord_model_hashes:
            case subrecord_model_swaps:
               if (last_loaded_stage < this->stages.size()) {
                  auto& stage = this->stages[last_loaded_stage];
                  stage.replacementModel.load(subrecord, intfc);
               }
               break;
            case subrecord_terminator: // end marker
               last_loaded_stage = _not_in_a_stage;
               break;
         }
      #endif
   }
   /*static*/ void destruction_stage_data::generate_use_info(tes_subrecord_reader& subrecord, use_info_builder& uib) {
      form_id_t formID;
      switch (subrecord.signature()) {
         case subrecord_header: // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
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
         case subrecord_stage_data: // destruction stage data
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
         case subrecord_model_path: // destruction stage model
         case subrecord_model_hashes: // 
         case subrecord_model_swaps: // 
            model::generate_use_info(subrecord, uib.owner);
            break;
         case subrecord_terminator: // destruction stage end marker
            break;
      }
   }
   void destruction_stage_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto stage_count = this->stages.size();
      if (stage_count > std::numeric_limits<uint8_t>::max()) {
         auto notice = specific_save_errors::too_many_stages(
            *intfc.target_stub,
            stage_count
         );
         intfc.throw_save_error(notice);
         return;
      }
      auto& DEST = record.open_next_subrecord(subrecord_header);
      DEST.write(this->health);
      DEST.write(uint8_t(stage_count));
      DEST.write(this->flags);
      DEST.skip_bytes(2);
      DEST.close();
      //
      for (size_t i = 0; i < this->stages.size(); ++i) {
         auto& stage = this->stages[i];

         auto& DSTD = record.open_next_subrecord(subrecord_stage_data);
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
         model.save(record, intfc, subrecord_model_path, subrecord_model_hashes, subrecord_model_swaps);

         record.open_next_subrecord(subrecord_terminator).close();
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