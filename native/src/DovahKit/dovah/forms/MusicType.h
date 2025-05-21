#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class MusicType : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::music_type;
         MusicType(const constructor_params& c) : Form(form_type, c) {};
         
         struct music_type_flag {
            music_type_flag() = delete;
            enum type : uint32_t {
               plays_one_selection  = 0x01,
               abrupt_transition    = 0x02,
               cycle_tracks         = 0x04,
               maintain_track_order = 0x08,
               //
               ducks_current_track  = 0x20,
               does_not_queue       = 0x40, // SSE only
            };
         };
         using music_type_flags_t = std::underlying_type_t<music_type_flag::type>;

         components::papyrus_attachment_data script_data; // VMAD
         //
         music_type_flags_t flags = 0; // FNAM
         uint16_t priority      = 0; // DATA+0x00
         uint16_t ducking_db    = 0; // DATA+0x01
         float    fade_duration = 0; // WNAM
         std::vector<form_reference_t> tracks; // TNAM, one per

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