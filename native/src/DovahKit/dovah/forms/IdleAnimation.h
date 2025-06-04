#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class IdleAnimation : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::idle;
         IdleAnimation(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const uint8_t loop_forever = 0xFF;

         static constexpr const size_t max_event_name_length = 0x104;
         static constexpr const size_t max_filename_length   = 0x104;

         struct flag {
            enum type : uint8_t {
               parent       = 0x01,
               sequence     = 0x02,
               no_attacking = 0x04,
               blocking     = 0x08,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         components::condition_list conditions; // CTDA[]
         components::papyrus_attachment_data script_data; // VMAD
         //
         std::string      animation_event;  // ENAM
         std::string      filename;         // DNAM      // HKX file
         form_reference_t parent;           // ANAM+0x00 // IDLE or AACT
         form_reference_t previous_sibling; // ANAM+0x04 // IDLE
         struct {
            struct {
               uint8_t min = 0; // DATA+0x00
               uint8_t max = 0; // DATA+0x01
            } loop_time_range;
            flags_t  flags              = 0; // DATA+0x02
            uint8_t  anim_group_section = 0; // DATA+0x03
            uint16_t replay_delay       = 0; // DATA+0x04
         } data; // DATA

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