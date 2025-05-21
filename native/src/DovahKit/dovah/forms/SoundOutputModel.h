#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class SoundOutputModel : public Form { // BGSSoundOutput (SOPM)
      public:
         static constexpr const enum form_type form_type = form_type::sound_output_model;
         SoundOutputModel(const constructor_params& c) : Form(form_type, c) {};

         enum class sound_output_type : uint32_t {
            use_hrtf,
            defined_speaker_output,
         };

         struct sound_output_flag {
            enum type : uint8_t {
               attenuates_with_distance = 0x01,
               allow_rumble = 0x02,
            };
         };
         using sound_output_flags_t = std::underlying_type_t<sound_output_flag::type>;

         struct channel_output {
            uint8_t left = 0;
            uint8_t right = 0;
            uint8_t center = 0;
            uint8_t low_frequency_effects = 0;
            uint8_t surround_left  = 0;
            uint8_t surround_right = 0;
            uint8_t rear_surround_left  = 0;
            uint8_t rear_surround_right = 0;

            void load(tes_subrecord_reader&);
            void save(tes_subrecord_writer&);
         };

      public:
         components::papyrus_attachment_data script_data;
         //
         sound_output_type    type  = sound_output_type::use_hrtf; // MNAM
         sound_output_flags_t flags = 0; // NAM1+0x00
         uint8_t reverb_send = 50; // NAM1+0x03 // percentage
         union {
            std::array<channel_output, 3> all = {};
            struct {
               channel_output mono;
               channel_output stereo_l;
               channel_output stereo_r;
            };
         } channels; // ONAM
         struct {
            struct {
               float minimum = 0.0F; // ANAM+0x04
               float maximum = 0.0F; // ANAM+0x08
            } distance;
            std::array<uint8_t, 5> curve = {}; // ANAM+0x0C through ANAM+0x10
         } attenuation; // BGSSoundOutput::DynamicAttenuationCharacteristics

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}