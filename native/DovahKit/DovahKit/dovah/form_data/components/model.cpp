#include "./model.h"
#include "dovah/load_order_interfaces/form_load.h"
#include "dovah/load_order_processes/use_info_build/use_info_builder.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/tes/reading/subrecord_view.h"

namespace dovah::form_data::components {
   #pragma region model
      void model::load_model_path(tes::subrecord_view& subrecord, load_order_interfaces::form_load&) {
         subrecord.read_all_to_string(this->model_path);
      }
      void model::load_precached_info(tes::subrecord_view& subrecord, load_order_interfaces::form_load& intfc) {
         auto& record = subrecord.get_containing_record();
         if (record.version() < 0x26)
            return;
         if (record.version() >= 0x28) {
            uint32_t number_of_counts = 0;
            uint32_t count_texture_hashes = 0;
            uint32_t count_addon_node_ids = 0;
            uint32_t count_materials      = 0;
            subrecord.read(number_of_counts);
            if (number_of_counts > 0) {
               subrecord.read(count_texture_hashes);
               if (number_of_counts > 1) {
                  subrecord.read(count_addon_node_ids);
                  if (subrecord.is_skyrim_special() && number_of_counts > 2) {
                     subrecord.read(count_materials);
                  }
               }
            }
            for (uint32_t i = 0; i < count_texture_hashes; ++i) {
               auto& entry = this->precached_info.texture_hashes.emplace_back();
               subrecord.read(entry);
            }
            for (uint32_t i = 0; i < count_addon_node_ids; ++i) {
               uint32_t value;
               if (subrecord.read(value))
                  this->precached_info.addon_node_ids.push_back(value);
            }
            for (uint32_t i = 0; i < count_materials; ++i) {
               auto& entry = this->precached_info.material_hashes.emplace_back();
               subrecord.read(entry);
            }
            return;
         }
         if (record.version() >= 0x26) {
            //
            // The game reads (subrecord.size() / 1.5 / 8 * 0xC) bytes. It's not the most intuitive 
            // approach but it amounts to reading the nearest multiple of 0xC bytes available in the 
            // subrecord, stuffing it all into a buffer, and then going over it 0xC bytes at a time.
            //
            while (subrecord.is_in_bounds(12)) {
               auto& entry = this->precached_info.texture_hashes.emplace_back();
               subrecord.unchecked_read(entry);
            }
            return;
         }
      }
      void model::load_facegen_flags(tes::subrecord_view& subrecord, load_order_interfaces::form_load&) {
         subrecord.read(this->facegen_flags);
      }
   #pragma endregion
}

namespace dovah::form_data::serialization {
   extern bool load(components::model& self, tes::subrecord_view& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'MOD3':
         case 'MOD4':
         case 'MOD5':
         case 'DMDL': // for model path
            self.load_model_path(subrecord, intfc);
            return true;
         case 'MODT':
         case 'MO2T':
         case 'MO3T':
         case 'MO4T':
         case 'MO5T':
         case 'DMDT': // for texture hashes
            self.load_precached_info(subrecord, intfc);
            return true;
         case 'MODD':
         case 'MOSD':
            self.load_facegen_flags(subrecord, intfc);
            return true;
      }
      return false;
   }

   template<>
   extern void build_outbound_use_info<components::model_ts>(tes::subrecord_view& subrecord, load_order_processes::use_info_build::use_info_builder& uib) {
      switch (subrecord.signature()) {
         case 'MODS':
         case 'MO2S':
         case 'MO3S':
         case 'MO4S':
         case 'MO5S':
         case 'DMDS':
            {
               uint32_t count = 0;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     std::string nif_block_name;
                     form_id     texture_set;
                     subrecord.read_length_prefixed_string<uint32_t>(nif_block_name);
                     if (subrecord.read(texture_set) && nif_block_name.size() && nif_block_name[0] != '\0') // the game doesn't retain entries with no name, and neither do we, so don't build use info for them
                        uib.add_outbound_form_use(texture_set);
                     subrecord.skip_bytes(4); // NIF block index
                     if (!subrecord.is_in_bounds())
                        return;
                  }
               }
            }
            break;
      }
   }

   extern void load(components::model_ts<managed_form_params>& self, tes::subrecord_view& subrecord, load_order_interfaces::form_load& intfc) {
      using component_type = std::decay_t<decltype(self)>;

      if (load(static_cast<components::model&>(self), subrecord, intfc))
         return;
      switch (subrecord.signature()) {
         case 'MODS':
         case 'MO2S':
         case 'MO3S':
         case 'MO4S':
         case 'MO5S':
         case 'DMDS':
            break;
         default:
            return;
      }
      uint32_t count;
      if (!subrecord.read(count))
         return;
      for (uint32_t i = 0; i < count; i++) {
         bool has_block_index = subrecord.get_containing_record().version() >= 0xF;
         //
         component_type::texture_swap entry;
         subrecord.read_length_prefixed_string<uint32_t>(entry.nif_block_name);
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
         if (entry.texture_set == nullptr) // the game doesn't retain entries with no TextureSet form
            continue;
         intfc.warn_if_use_is_wrong_type(entry.texture_set, form_type::texture_set, subrecord);
         if (entry.nif_block_name.empty() || entry.nif_block_name[0] == '\0') // the game doesn't retain entries with no NIF block name.
            continue;
         //
         self.texture_swaps.emplace_back() = std::move(entry);
         if (!subrecord.is_in_bounds())
            break;
      }
   }
}