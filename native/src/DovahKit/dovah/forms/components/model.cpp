#include "model.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void model::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            subrecord.to_string(this->model_path);
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            {
               auto s = subrecord.size();
               this->texture_hashes.data.resize(s);
               for (uint32_t i = 0; i < s; i++)
                  subrecord.read(this->texture_hashes.data[i]);
            }
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            {
               uint32_t count;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     auto& entry = this->texture_swaps.emplace_back();
                     subrecord.read_length_prefixed_string<4>(entry.nif_block_name);
                     subrecord.read(entry.texture_set);
                     subrecord.read(entry.nif_block_index);
                     //
                     intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::texture_set, intfc.target_stub, entry.texture_set)
                           .set_cause_form_index(i)
                     );
                     //
                     if (!subrecord.is_in_bounds())
                        break;
                  }
               }
            }
            break;
      }
   }
   /*static*/ void model::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            {
               uint32_t count;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     subrecord.skip_length_prefixed_string<4>();
                     if (subrecord.read(formID))
                        uib.add_outbound_reference(formID);
                     subrecord.skip_bytes(4);
                     if (!subrecord.is_in_bounds())
                        return;
                  }
               }
            }
            break;
      }
   }
   void model::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            subrecord.write(this->model_path);
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            for (auto& byte : this->texture_hashes.data) {
               subrecord.write(byte);
            }
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            subrecord.write(uint32_t(this->texture_swaps.size()));
            for (auto& entry : this->texture_swaps) {
               subrecord.write_length_prefixed_string<4>(entry.nif_block_name);
               subrecord.write(entry.texture_set);
               subrecord.write(entry.nif_block_index);
            }
            break;
      }
   }
   void model::save(tes_record_writer& record, load_order_interfaces::form_save& intfc, uint32_t signature_path, uint32_t signature_hash, uint32_t signature_swap) {
      if (!this->model_path.empty())
         this->save(record.open_next_subrecord(signature_path), intfc);
      if (this->has_texture_hashes())
         this->save(record.open_next_subrecord(signature_hash), intfc);
      if (!this->texture_swaps.empty())
         this->save(record.open_next_subrecord(signature_swap), intfc);
   }
   void model::clear(loaded_forms::Form& my_owner) {
      this->model_path.clear();
      this->texture_hashes.data.clear();
      for (auto& entry : this->texture_swaps)
         entry.texture_set.set(my_owner, nullptr);
   }
   void model::clone_from(const model& other, loaded_forms::Form& my_owner) noexcept {
      this->model_path     = other.model_path;
      this->texture_hashes = other.texture_hashes;
      //
      size_t size = other.texture_swaps.size();
      if (!this->texture_swaps.empty()) {
         for (auto& entry : this->texture_swaps)
            entry.texture_set.set(my_owner, nullptr);
         this->texture_swaps.clear();
      }
      this->texture_swaps.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& entry = this->texture_swaps[i];
         auto& from  = other.texture_swaps[i];
         entry.nif_block_name  = from.nif_block_name;
         entry.nif_block_index = from.nif_block_index;
         entry.texture_set.set(my_owner, from.texture_set);
      }
   }
   void model::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto& entry : this->texture_swaps)
         entry.texture_set.clear_if(my_owner, target);
   }
}