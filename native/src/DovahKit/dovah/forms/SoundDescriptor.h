#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/conditions.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class SoundDescriptor : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::sound_descriptor;
         SoundDescriptor(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_sound_file_path_length = 0x104;

         enum class descriptor_type {
            standard = 0x1EEF540A, // CRC32 hash of "BGSStandardSoundDef"
         };
         enum class loop_type {
            none,
            loop          = 1 << 3, // 0x08
            envelope_fast = 1 << 4, // 0x10
            envelope_slow = 1 << 5, // 0x20
         };

         struct LengthCharacteristics {
            //
            // LNAM: 0xAABBCCDD
            //  - DD: unk00
            //  - CC: loop_type
            //  - BB: unk02
            //  - AA: rumble send
            // 
            // FNAM: 0xAABBCCDD
            //  - DD & 0x10 -> LNAM CC | 0x08
            //  - AA & 0x02 -> LNAM CC | 0x10
            //  - AA & 0x04 -> LNAM CC | 0x20
            //
            loop_type type = loop_type::none; // LNAM+0x01
            struct {
               uint8_t large = 0; // upper four bits of (LNAM+0x03), times 7
               uint8_t small = 0; // == lower four bits of (LNAM+0x03), times 7
            } rumble_send;
         };

         components::papyrus_attachment_data script_data;
         components::condition_list conditions;
         //
         descriptor_type  type = descriptor_type::standard; // CNAM
         form_reference_t category;      // GNAM // value is SNCT
         form_reference_t alternate_for; // SNAM // value is SNDR
         form_reference_t output_model;  // ONAM // value is SOPM
         std::vector<std::string> sound_files; // ANAM[]
         LengthCharacteristics length_characteristics; // LNAM/FNAM
         struct {
            int8_t shift    = 0; // BNAM+0x00 // BNAM == BGSStandardSoundDef::SoundPlaybackCharacteristics
            int8_t variance = 0; // BNAM+0x01
         } frequency;
         uint8_t priority = 0x80; // BNAM+0x02
         uint8_t db_variance = 0; // BNAM+0x03
         uint16_t static_attenuation = 0; // BNAM+0x04 // value times 100

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