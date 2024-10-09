#pragma once
#include <string>
#include <vector>
#include "../_common.h"
#include "../structs/precached_nif_info.h"

namespace dovah::loaded_forms::components {
   class model { // MODL
      protected:
         model(bool sts) : supports_texture_swaps(sts) {}
      public:
         model() : supports_texture_swaps(false) {}

         struct facegen_flag {
            facegen_flag() = delete;
            enum type : uint8_t {
               head       = 0x01,
               torso      = 0x02,
               hand_right = 0x04,
               hand_left  = 0x08,
            };
         };
         using facegen_flags_t = std::underlying_type_t<facegen_flag::type>;

         const bool         supports_texture_swaps;
         std::string        model_path;
         precached_nif_info precached_info; // MODT (used to be just texture hashes, in past games)
         facegen_flags_t    facegen_flags = 0; // MODD
         
         // Special-case load functions for when alternate subrecord signatures are used (e.g. RACE/ANAM).
         void load_model_path(tes_subrecord_reader&, load_order_interfaces::form_load&);

         bool load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc); // returns (true) if the subrecord is recognized and handled
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);

         void save_model_path(tes_subrecord_writer&, load_order_interfaces::form_save&);
         void save_precached_info(tes_subrecord_writer&, load_order_interfaces::form_save&);
         void save_facegen_flags(tes_subrecord_writer&, load_order_interfaces::form_save&); // typically in MODD or MOSD
         //
         void save(tes_record_writer&, load_order_interfaces::form_save&, uint32_t signature_path, uint32_t signature_hash); // opens the subrecords, etc., for you
         
         void clear();
         void clone_from(const model& original) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         
         constexpr bool has_precached_info() const noexcept { return !this->precached_info.texture_hashes.empty() || !this->precached_info.addon_node_ids.empty(); }
   };
   
   class model_ts : public model {
      public:
         model_ts() : model(true) {}
         
         struct texture_swap { // MODS
            std::string      nif_block_name;
            form_reference_t texture_set;
            uint32_t         nif_leaf_index;
         };
         std::vector<texture_swap> texture_swaps;
         
         bool load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc); // returns (true) if the subrecord is recognized and handled
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);

         void save_texture_swaps(tes_subrecord_writer&, load_order_interfaces::form_save&);
         //
         void save(tes_record_writer&, load_order_interfaces::form_save&, uint32_t signature_path, uint32_t signature_hash) = delete;
         void save(tes_record_writer&, load_order_interfaces::form_save&, uint32_t signature_path, uint32_t signature_hash, uint32_t signature_swap); // opens the subrecords, etc., for you
         
         void clear(loaded_forms::Form& my_owner);
         void clone_from(const model_ts& original, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
   };
}