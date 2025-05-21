#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class ReverbParameters : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::reverb_parameters;
         ReverbParameters(const constructor_params& c) : Form(form_type, c) {};

         components::papyrus_attachment_data script_data; // VMAD
         //
         uint16_t decay_time     = 0; // DATA+0x00
         uint16_t hf_reference   = 0; // DATA+0x02
         int8_t   room_filter    = 0; // DATA+0x04
         int8_t   room_hf_filter = 0; // DATA+0x05
         int8_t   reflections    = 0; // DATA+0x06
         int8_t   reverb_amp     = 0; // DATA+0x07
         uint8_t  decay_hf_ratio = 0; // DATA+0x08 // v*100
         uint8_t  reflect_delay  = 0; // DATA+0x09
         uint8_t  reverb_delay   = 0; // DATA+0x0A
         uint8_t  diffusion      = 0; // DATA+0x0B
         uint8_t  density        = 0; // DATA+0x0C
         uint8_t  unk0D          = 0; // DATA+0x0D // CommonLibSSE assumes this is padding, but the game loads 0x0E bytes specifically, so I'm inclined to believe otherwise.

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