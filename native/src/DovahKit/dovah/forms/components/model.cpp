#include "model.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   bool model::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for model path
            subrecord.read(this->model_path);
            return true;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for texture hashes
            {
               auto& record = subrecord.get_containing_record();
               if (record.version() < 0x26)
                  return true;
               if (record.version() >= 0x28) {
                  uint32_t number_of_counts = 0;
                  uint32_t edi = 0;
                  uint32_t ebx = 0;
                  subrecord.read(number_of_counts);
                  subrecord.read(edi);
                  subrecord.read(ebx);
                  if (number_of_counts <= 0)
                     edi = 0;
                  if (number_of_counts <= 1)
                     ebx = 0;
                  for (uint32_t i = 0; i < edi; ++i) {
                     auto& entry = this->texture_hash_data.hashes.emplace_back();
                     subrecord.read(entry.flags);
                     subrecord.read_signature(entry.extension);
                     subrecord.read(entry.hash);
                  }
                  for (uint32_t i = 0; i < ebx; ++i) {
                     uint32_t value;
                     if (subrecord.read(value))
                        this->texture_hash_data.addenda.push_back(value);
                  }
                  return true;
               }
               if (record.version() >= 0x26) {
                  //
                  // The game reads (subrecord.size() / 1.5 / 8 * 0xC) bytes. It's not the most intuitive 
                  // approach but it amounts to reading the nearest multiple of 0xC bytes available in the 
                  // subrecord, stuffing it all into a buffer, and then going over it 0xC bytes at a time.
                  //
                  while (subrecord.is_in_bounds(12)) {
                     auto& entry = this->texture_hash_data.hashes.emplace_back();
                     subrecord.read(entry.flags);
                     subrecord.read_signature(entry.extension);
                     subrecord.read(entry.hash);
                  }
                  return true;
               }
            }
            return true;
         case 'MODD':
            subrecord.read(this->facegen_flags);
            return true;
      }
      return false;
   }
   bool model_ts::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (model::load(subrecord, intfc))
         return true;
      switch (subrecord.signature()) {
         case 'MODS':
         case 'MO2S':
         case 'DMDS':
            break;
         default:
            return false;
      }
      uint32_t count;
      if (!subrecord.read(count))
         return true;
      for (uint32_t i = 0; i < count; i++) {
         bool has_block_index = subrecord.get_containing_record().version() >= 0xF;
         //
         texture_swap entry;
         subrecord.read_length_prefixed_string<4>(entry.nif_block_name);
         subrecord.read(entry.texture_set);
         if (!has_block_index) {
            //
            // If the record version is less than 0xF, the game uses the value -1 instead of loading a 
            // value. However, -1 is not treated as "ignore the block index," so in practice, these 
            // texture swaps would never actually apply.
            //
            entry.nif_block_index = -1;
         } else {
            subrecord.read(entry.nif_block_index);
         }
         //
         if (!entry.texture_set) // the game doesn't retain entries with no TextureSet form
            continue;
         if (entry.nif_block_name.empty() || entry.nif_block_name[0] == '\0') // the game doesn't retain entries with no NIF block name.
            continue;
         //
         this->texture_swaps.push_back(entry);
         //
         intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
            detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::texture_set, intfc.target_stub, entry.texture_set)
               .set_cause_form_index(i)
         );
         //
         if (!subrecord.is_in_bounds())
            break;
      }
      return true;
   }

   /*static*/ void model::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for the model path
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for texture hashes
            break;
      }
   }
   /*static*/ void model_ts::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      model::generate_use_info(subrecord, uib);
      //
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for texture swaps
            {
               uint32_t count;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     std::string nif_block_name;
                     subrecord.read_length_prefixed_string<4>(nif_block_name);
                     if (subrecord.read(formID) && nif_block_name.size() && nif_block_name[0] != '\0') // the game doesn't retain entries with no name, and neither do we, so don't build use info for them
                        uib.add_outbound_reference(formID);
                     subrecord.skip_bytes(4); // NIF block index
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
            {
               auto& record = subrecord.get_containing_record();
               if (record.version() < 0x26)
                  break;
               auto& data = this->texture_hash_data;
               if (record.version() >= 0x28) {
                  if (data.addenda.empty()) {
                     if (data.hashes.empty()) {
                        subrecord.write(uint32_t(0));
                        break;
                     }
                     subrecord.write(uint32_t(1));
                     subrecord.write(uint32_t(data.hashes.size()));
                  } else {
                     subrecord.write(uint32_t(2));
                     subrecord.write(uint32_t(data.hashes.size()));
                     subrecord.write(uint32_t(data.addenda.size()));
                  }
                  for (auto& h : data.hashes) {
                     subrecord.write(h.flags);
                     subrecord.write_signature(h.extension);
                     subrecord.write(h.hash);
                  }
                  for (auto& v : data.addenda)
                     subrecord.write(v);
               }
               if (record.version() >= 0x26) {
                  for (auto& h : data.hashes) {
                     subrecord.write(h.flags);
                     subrecord.write_signature(h.extension);
                     subrecord.write(h.hash);
                  }
               }
            }
            break;
         case 'MODD':
            subrecord.write(this->facegen_flags);
            break;
      }
   }
   void model_ts::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      switch (subrecord.signature()) {
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

   void model::save(tes_record_writer& record, load_order_interfaces::form_save& intfc, uint32_t signature_path, uint32_t signature_hash) {
      if (!this->model_path.empty())
         this->save(record.open_next_subrecord(signature_path), intfc);
      if (this->has_texture_hashes())
         this->save(record.open_next_subrecord(signature_hash), intfc);
   }
   void model_ts::save(tes_record_writer& record, load_order_interfaces::form_save& intfc, uint32_t signature_path, uint32_t signature_hash, uint32_t signature_swap) {
      model::save(record, intfc, signature_path, signature_hash);
      if (!this->texture_swaps.empty())
         this->save(record.open_next_subrecord(signature_swap), intfc);
   }

   void model::clear() {
      this->model_path.clear();
      this->texture_hash_data.hashes.clear();
      this->texture_hash_data.addenda.clear();
   }
   void model_ts::clear(loaded_forms::Form& my_owner) {
      model::clear();
      for (auto& entry : this->texture_swaps)
         entry.texture_set.set(my_owner, nullptr);
   }

   void model::clone_from(const model& other) noexcept {
      this->model_path        = other.model_path;
      this->texture_hash_data = other.texture_hash_data;
   }
   void model_ts::clone_from(const model_ts& other, loaded_forms::Form& my_owner) noexcept {
      model::clone_from(other);
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
         auto& from = other.texture_swaps[i];
         entry.nif_block_name = from.nif_block_name;
         entry.nif_block_index = from.nif_block_index;
         entry.texture_set.set(my_owner, from.texture_set);
      }
   }

   void model::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
   }
   void model_ts::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      model::sever_outbound_references_to(target, my_owner);
      for (auto& entry : this->texture_swaps)
         entry.texture_set.clear_if(my_owner, target);
   }
}