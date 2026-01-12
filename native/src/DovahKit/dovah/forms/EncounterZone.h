#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"
#include "../use_info/entry_flags/encounter_zone.h"

namespace dovah::loaded_forms {
   class EncounterZone : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::encounter_zone;
         EncounterZone(const constructor_params& c) : Form(form_type, c) {};

         struct flag {
            enum type : uint8_t {
               never_resets             = 0x01,
               match_pc_below_min_level = 0x02,
               disable_combat_boundary  = 0x04,
            };
         };

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         form_reference_t owner;    // DATA+0x00 -> NPC_ or FACT
         unique_form_reference_t<use_info::entry_flags::encounter_zone::location> location; // DATA+0x04 -> LCTN
         struct {
            int8_t  rank      = 0; // DATA+0x08
            int8_t  min_level = 0; // DATA+0x09
            uint8_t flags     = 0; // DATA+0x0A
            int8_t  max_level = 0; // DATA+0x0B
         } data;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}