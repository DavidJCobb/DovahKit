#pragma once
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
#include "helpers/vector3.h"
#include "../../_common.h"
#include "../navmesh_pathing_cell.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   struct navmesh_info {
      public:
         static constexpr const uint32_t subrecord = 'NVMI';

      public:
         struct door_link {
            uint32_t         crc = 0;
            form_reference_t door; // -> REFR
         };

         struct triangle {
            std::array<uint16_t, 3> vertices;
         };

         struct island_data {
            cobb::vector3<float> min;
            cobb::vector3<float> max;
            std::vector<triangle> triangles;
            std::vector<cobb::vector3<float>> vertices;
         };

      public:
         bool is_active_file_data = false;

         form_reference_t     navmesh;         // NVMI+0x00 -> NAVM
         uint32_t             category = 0;    // NVMI+0x04
         cobb::vector3<float> approx_location; // NVMI+0x08
         float                preference = 0;  // NVMI+0x14 // a percentage
         struct {
            std::vector<form_reference_t> edges; // NVMI+0x18
            std::vector<form_reference_t> preferred_edges;
            std::vector<door_link> doors;
         } links;
         std::optional<island_data> island;
         navmesh_pathing_cell pathing_cell;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&) = delete; // use (package_location::use_info_state)
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         
         void clone_from(const navmesh_info& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
         
         struct use_info_state {
            form_id_t navmesh;
            struct {
               std::vector<form_id_t> edges;
               std::vector<form_id_t> preferred_edges;
               std::vector<form_id_t> doors;
            } links;
            navmesh_pathing_cell::use_info_state pathing_cell;
            //
            void generate_use_info(tes_subrecord_reader&);
            void commit_to(form_stub_use_info_builder&);
            std::vector<form_id_t> as_combined_list() const;
         };
   };
}