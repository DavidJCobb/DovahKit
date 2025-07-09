#pragma once
#include <cstdint>
#include <set>
#include <vector>
#include "../../_common.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   struct road_marker_map {
      public:
         struct form_specific_use_info_data;

         // form_reference_t can't (sensibly) be used as a key in an STL (unordered) map,
         // so we instead need to implement our map as a vector of pairs, and enforce the 
         // uniqueness of those pairs ourselves. Unfortunate.
         struct entry {
            form_reference_t navmesh; // key
            uint32_t         index;   // value
         };

      protected:
         std::vector<entry> data;

         entry& _get_or_create_entry_during_load(form_stub*);

      public:
         constexpr bool empty() const noexcept { return this->data.empty(); }

         std::optional<uint32_t> get_index_for(form_stub&) const;
         void set_index_for(form_stub&, uint32_t, Form& my_owner);

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_subrecord_reader&, form_specific_use_info_data&);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc);
         void clone_from(const road_marker_map& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
         
         struct form_specific_use_info_data {
            std::set<form_id_t> navmeshes;
         };
   };
}