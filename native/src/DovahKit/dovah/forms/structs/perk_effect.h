#pragma once
#include <cstdint>
#include <vector>
#include "../_common.h"
#include "../components/conditions.h"
#include "../../data/perk_entry_points.h"
#include "./perk_entry_point_data.h"

namespace dovah::loaded_forms::structs {
   struct perk_effect {
      public:
         static constexpr const uint32_t subrecord_start      = 'PRKE';
         static constexpr const uint32_t subrecord_conditions = 'PRKC';
         static constexpr const uint32_t subrecord_end        = 'PRKF';

      public:
         enum class type : uint8_t {
            quest_and_stage,
            ability,
            entry_point,
         };

         struct condition_group {
            int8_t which = 0; // PRKC
            components::condition_list conditions; // CTDA[]
         };

         struct data_types { // poor man's namespace
            data_types() = delete;

            struct quest {
               form_reference_t quest;
               uint8_t          stage = 0;
            };
            struct ability {
               form_reference_t spell;
            };
            struct entry_point {
               perk_entry_point      entry_point = (perk_entry_point)0;
               perk_entry_point_data function;
               std::vector<condition_group> condition_groups;
            };
         };

      public:
         uint8_t rank     = 0; // PRKE+0x01
         uint8_t priority = 0; // PRKE+0x02
         std::variant<
            data_types::quest,
            data_types::ability,
            data_types::entry_point
         > data;

      public:
         type get_type() const;
         void set_type(Form& my_owner, type);

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc, size_t which); // call when you see PRKE
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         
         void clone_from(const perk_effect& src, Form& my_owner) noexcept;
         void clear(Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept;
   };
}