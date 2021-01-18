#pragma once
#include <string>
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct model_texture_hash { // MODT
      std::vector<uint8_t> data; // bytes
      //
      // Skyrim still loads this data, but we don't necessarily know how. We'd need 
      // to reverse-engineer {void LoadMODTSubrecord(TESModel*, BGSLoadFormBuffer*) 
      // at 0x00454AF0 in Skyrim Classic to learn more. I do know for certain, how-
      // ever, that the loading behavior changes depending on the form version.
      //
      // xEdit has this decoded.
      //
   };
   struct model_texture_swap { // MODS
      std::string      nif_block_name;
      form_reference_t texture_set;
      uint32_t         nif_block_index;
   };
   struct model { // MODL
      std::string model_path;
      model_texture_hash texture_hashes;
      std::vector<model_texture_swap> texture_swaps;
      //
      void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
      static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
      void save(tes_subrecord_writer&, load_order_interfaces::form_save&); // open the subrecord before calling
      void save(tes_record_writer&, load_order_interfaces::form_save&, uint32_t signature_path, uint32_t signature_hash, uint32_t signature_swap); // opens the subrecords, etc., for you
      //
      void clear(loaded_forms::Form& my_owner);
      void clone_from(const model& original, loaded_forms::Form& owner_of_clone) noexcept;
      void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
      //
      inline bool has_texture_hashes() const noexcept { return !this->texture_hashes.data.empty(); }
   };
}