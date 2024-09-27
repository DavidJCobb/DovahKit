#pragma once
#include "../_common.h"
#include <cstdint>
#include <unordered_map>

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}

namespace dovah::loaded_forms::structs {
   namespace impl::world_large_ref_data {
      //
      // Have to define this outside of `world_large_ref_data`, because we must define the specialization 
      // of `std::hash` before we declare any `std::unordered_map` that uses `cell_id` as its key type.
      //
      struct cell_id {
         constexpr bool operator==(const cell_id&) const noexcept = default;

         int16_t y = 0;
         int16_t x = 0;
      };
   }
}

template<>
struct std::hash<dovah::loaded_forms::structs::impl::world_large_ref_data::cell_id> {
   size_t operator()(const dovah::loaded_forms::structs::impl::world_large_ref_data::cell_id& id) const noexcept {
      uint32_t merged = (id.y << 0x10) | id.x;
      return std::hash<uint32_t>{}(merged);
   }
};

namespace dovah::loaded_forms::structs {
   //
   // WRLD/RNAM
   //
   class world_large_ref_data {
      public:
         using cell_id = impl::world_large_ref_data::cell_id;

         struct form_specific_use_info_data {
            std::unordered_map<cell_id, std::vector<form_id_t>> cells_to_refs;
         };

      public:
         std::unordered_map<cell_id, std::vector<form_reference_t>> cells_to_refs;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save&) const;
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         void clone_from(const world_large_ref_data& original, loaded_forms::Form& my_containing_form) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);

         bool empty() const noexcept;
   };
}