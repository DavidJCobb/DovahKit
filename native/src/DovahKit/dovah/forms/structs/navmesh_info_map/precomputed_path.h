#pragma once
#include <cstdint>
#include <vector>
#include "../../_common.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   struct precomputed_path {
      public:
         static constexpr const uint32_t subrecord = 'NVPP';

      public:
         std::vector<form_reference_t> navmeshes;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, size_t which_am_i);
         static void generate_use_info(tes_subrecord_reader&, std::vector<form_id_t>& append_to);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         void clone_from(const precomputed_path& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
   };
}