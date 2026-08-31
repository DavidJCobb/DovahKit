#pragma once
#include <cstdint>
#include <variant>
#include "helpers/vector3.h"
#include "../_common.h"

namespace dovah::loaded_forms::structs {
   struct navmesh_pathing_cell {
      public:
         struct pathing_cell_exterior {
            form_reference_t parent_world;
            int16_t grid_x = 0;
            int16_t grid_y = 0;
         };
         struct pathing_cell_interior { // NOTE: sometimes used in vanilla for some exterior cells; not sure when or why
            form_reference_t cell;
         };

      public:
         uint32_t crc = 0;
         std::variant<
            pathing_cell_exterior,
            pathing_cell_interior
         > data;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, form_stub* owning_navmesh);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&) = delete; // use (navmesh_pathing_cell::use_info_state)
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         
         void clone_from(const navmesh_pathing_cell& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
         
         struct use_info_state {
            form_id_t parent_world;
            form_id_t interior_cell;
            //
            void generate_use_info(tes_subrecord_reader&);
            void commit_to(form_stub_use_info_builder&);
         };
   };
}