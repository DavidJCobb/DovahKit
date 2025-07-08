#pragma once
#include <cstdint>
#include <vector>
#include "../../_common.h"
#include "./precomputed_path.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   struct precomputed_path_collection {
      public:
         struct form_specific_use_info_data;

      public:
         std::vector<precomputed_path> paths;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, Form& my_owner);
         static void generate_use_info(tes_subrecord_reader&, form_specific_use_info_data&);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         void clone_from(const precomputed_path_collection& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
         
         struct form_specific_use_info_data {
            std::vector<form_id_t> navmeshes;
         };
   };
}