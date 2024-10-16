#include "model.h"
#include "../_common_cpp.h"

#include <limits>
namespace {
   //
   // For the texture hash and add-on node ID lists on TESModel, the game can safely handle 
   // any list size, but can only make use of a limited number of list items. This begs the 
   // question: should we truncate the lists on save? It's okay to do this without even 
   // bothering to notify the user because:
   // 
   //  - These lists don't contain form IDs, so we won't be causing Use Info problems.
   // 
   //  - These lists are used entirely under the hood for optimization purposes, so there 
   //    won't be any loss of user-facing data.
   // 
   //  - No sane NIF will have 65536+ different textures nor 256+ different add-on node IDs.
   // 
   // The decision I've made is to refrain from truncating the lists, in favor of faithfully 
   // representing whatever data is inside even if the game can't make full use of it.
   //
   constexpr const bool clip_the_precached_lists = false;
   //
   constexpr const size_t max_usable_addon_node_ids = std::numeric_limits<uint8_t>::max();
   constexpr const size_t max_usable_texture_hashes = std::numeric_limits<uint16_t>::max();
}

namespace dovah::loaded_forms::components {
   void model::load_model_path(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      subrecord.read(this->model_path);
   }

   bool model::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'MOD3':
         case 'DMDL': // for model path
            this->load_model_path(subrecord, intfc);
            return true;
         case 'MODT':
         case 'MO2T':
         case 'MO3T':
         case 'DMDT': // for texture hashes
            {
               auto& record = subrecord.get_containing_record();
               if (record.version() < 0x26)
                  return true;
               if (record.version() >= 0x28) {
                  uint32_t number_of_counts = 0;
                  uint32_t edi = 0;
                  uint32_t ebx = 0;
                  uint32_t count_materials = 0;
                  subrecord.read(number_of_counts);
                  if (number_of_counts > 0) {
                     subrecord.read(edi);
                     if (number_of_counts > 1) {
                        subrecord.read(ebx);
                        if (subrecord.is_skyrim_special() && number_of_counts > 2) {
                           subrecord.read(count_materials);
                        }
                     }
                  }
                  for (uint32_t i = 0; i < edi; ++i) {
                     auto& entry = this->precached_info.texture_hashes.emplace_back();
                     subrecord.read(entry.file_hash);
                     subrecord.read_signature(entry.extension);
                     subrecord.read(entry.folder_hash);
                  }
                  for (uint32_t i = 0; i < ebx; ++i) {
                     uint32_t value;
                     if (subrecord.read(value))
                        this->precached_info.addon_node_ids.push_back(value);
                  }
                  for (uint32_t i = 0; i < count_materials; ++i) {
                     auto& entry = this->precached_info.material_hashes.emplace_back();
                     subrecord.read(entry.file_hash);
                     subrecord.read_signature(entry.extension);
                     subrecord.read(entry.folder_hash);
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
                     auto& entry = this->precached_info.texture_hashes.emplace_back();
                     subrecord.read(entry.file_hash);
                     subrecord.read_signature(entry.extension);
                     subrecord.read(entry.folder_hash);
                  }
                  return true;
               }
            }
            return true;
         case 'MODD':
         case 'MOSD':
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
         case 'MO3S':
         case 'DMDS':
            break;
         case 'MOSD':
         case 'MODD':
            return true;
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
            entry.nif_leaf_index = -1;
         } else {
            subrecord.read(entry.nif_leaf_index);
         }
         //
         if (!entry.texture_set) // the game doesn't retain entries with no TextureSet form
            continue;
         if (entry.nif_block_name.empty() || entry.nif_block_name[0] == '\0') // the game doesn't retain entries with no NIF block name.
            continue;
         //
         this->texture_swaps.push_back(entry);
         //
         intfc.warn_if_ref_is_wrong_type(entry.texture_set, form_type::texture_set, subrecord.signature());
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

   void model::save_model_path(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->model_path);
   }
   void model::save_precached_info(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      auto& record = subrecord.get_containing_record();
      if (record.version() < 0x26)
         return;
      auto& data = this->precached_info;
      if (record.version() >= 0x28) {
         uint32_t texture_hashes_count = data.texture_hashes.size();
         uint32_t addon_node_ids_count = data.addon_node_ids.size();
         //
         if constexpr (clip_the_precached_lists) {
            if (texture_hashes_count > max_usable_texture_hashes) {
               texture_hashes_count = max_usable_texture_hashes;
            }
            if (addon_node_ids_count > max_usable_addon_node_ids) {
               addon_node_ids_count = max_usable_addon_node_ids;
            }
         }

         bool write_addon_ids = !data.addon_node_ids.empty();
         bool write_materials = subrecord.is_skyrim_special() && !data.material_hashes.empty();

         if (!write_addon_ids && !write_materials) {
            //
            // NOTE: We're inconsistent with Bethesda here. They seem to always write both 
            //       counts even if the lists are empty (i.e. 00000002 00000000 00000000).
            //
            if (data.texture_hashes.empty()) {
               subrecord.write(uint32_t(0));
               return;
            }
            subrecord.write(uint32_t(1));
            subrecord.write(uint32_t(texture_hashes_count));
         } else {
            if (!write_materials) {
               subrecord.write(uint32_t(2));
               subrecord.write(uint32_t(texture_hashes_count));
               subrecord.write(uint32_t(addon_node_ids_count));
            } else {
               subrecord.write(uint32_t(3));
               subrecord.write(uint32_t(texture_hashes_count));
               subrecord.write(uint32_t(addon_node_ids_count));
               subrecord.write(uint32_t(data.material_hashes.size()));
            }
         }
         for (size_t i = 0; i < texture_hashes_count; ++i) {
            const auto& item = data.texture_hashes[i];
            subrecord.write(item.file_hash);
            subrecord.write_signature(item.extension);
            subrecord.write(item.folder_hash);
         }
         if (write_addon_ids) {
            for (size_t i = 0; i < addon_node_ids_count; ++i) {
               subrecord.write(data.addon_node_ids[i]);
            }
         }
         if (write_materials) {
            for (auto& item : data.material_hashes) {
               subrecord.write(item.file_hash);
               subrecord.write_signature(item.extension);
               subrecord.write(item.folder_hash);
            }
         }
      }
      if (record.version() >= 0x26) {
         uint32_t texture_hashes_count = data.texture_hashes.size();
         //
         if constexpr (clip_the_precached_lists) {
            if (texture_hashes_count > max_usable_texture_hashes) {
               texture_hashes_count = max_usable_texture_hashes;
            }
         }
                  
         for (size_t i = 0; i < texture_hashes_count; ++i) {
            const auto& item = data.texture_hashes[i];
            subrecord.write(item.file_hash);
            subrecord.write_signature(item.extension);
            subrecord.write(item.folder_hash);
         }
      }
   }

   void model_ts::save_texture_swaps(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(uint32_t(this->texture_swaps.size()));
      for (auto& entry : this->texture_swaps) {
         subrecord.write_length_prefixed_string<4>(entry.nif_block_name);
         subrecord.write(entry.texture_set);
         subrecord.write(entry.nif_leaf_index);
      }
   }

   void model::save_facegen_flags(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->facegen_flags);
   }

   void model::save(tes_record_writer& record, load_order_interfaces::form_save& intfc, uint32_t signature_path, uint32_t signature_hash) {
      if (!this->model_path.empty()) {
         auto& subrecord = record.open_next_subrecord(signature_path);
         this->save_model_path(subrecord, intfc);
         subrecord.close();
      }
      if (this->has_precached_info()) {
         auto& subrecord = record.open_next_subrecord(signature_hash);
         this->save_precached_info(subrecord, intfc);
         subrecord.close();
      }
   }
   void model_ts::save(tes_record_writer& record, load_order_interfaces::form_save& intfc, uint32_t signature_path, uint32_t signature_hash, uint32_t signature_swap) {
      model::save(record, intfc, signature_path, signature_hash);
      if (!this->texture_swaps.empty()) {
         auto& subrecord = record.open_next_subrecord(signature_swap);
         this->save_texture_swaps(subrecord, intfc);
         subrecord.close();
      }
   }

   void model::clear() {
      this->model_path.clear();
      this->precached_info.texture_hashes.clear();
      this->precached_info.addon_node_ids.clear();
   }
   void model_ts::clear(loaded_forms::Form& my_owner) {
      model::clear();
      for (auto& entry : this->texture_swaps)
         entry.texture_set.set(my_owner, nullptr);
   }

   void model::clone_from(const model& other) noexcept {
      this->model_path     = other.model_path;
      this->precached_info = other.precached_info;
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
         entry.nif_leaf_index = from.nif_leaf_index;
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