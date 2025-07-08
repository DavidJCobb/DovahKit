#pragma once
#include <cstdint>
#include <unordered_map>
#include "../../_common.h"
#include "./navmesh_info.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   struct navmesh_info_collection {
      public:
         struct form_specific_use_info_data;

      public:
         std::unordered_map<form_stub*, navmesh_info> infos;

      public:
         void load_one(tes_subrecord_reader&, load_order_interfaces::form_load& intfc); // load a single NVMI
         static void generate_use_info(tes_subrecord_reader&, form_specific_use_info_data&);
         void save_all(tes_record_writer&, load_order_interfaces::form_save& intfc); // save all NVMI that are part of the active file
         void clone_from(const navmesh_info_collection& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
         
         struct form_specific_use_info_data {
            std::unordered_map<form_id_t, std::vector<form_id_t>> infos;
         };
   };
}