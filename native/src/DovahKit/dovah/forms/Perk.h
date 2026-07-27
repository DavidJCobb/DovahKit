#pragma once
#include <array>
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"
#include "structs/perk_effect.h"
#include "../data/perk_entry_points.h"

namespace dovah::loaded_forms {
   class Perk : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::perk;
         Perk(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               non_playable = 0x00000004,
            };
         };

      public:
         components::condition_list          conditions; // CTDA[]
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         //
         std::string icon; // ICON
         struct {
            bool    is_trait   = false;
            uint8_t level      = 0;
            uint8_t rank_count = 1;
            bool    playable   = false;
            bool    hidden     = false;
         } data; // DATA
         form_reference_t next_perk; // NNAM
         std::vector<structs::perk_effect> effects; // (PRKE...PRKF)[]

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