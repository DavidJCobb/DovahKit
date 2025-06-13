#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Ragdoll : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::ragdoll;
         Ragdoll(const constructor_params& c) : Form(form_type, c) {};

      public:
         components::model model;
         components::papyrus_attachment_data script_data; // VMAD
         //
         struct { // sizeof == 0xE
            uint16_t unk00 = 0; // number of values in RAFB. DATA must precede RAFB, because the array is init'd via [the equivalent of] std::vector::resize
            uint16_t unk02 = 0; // size of some sort of array in the form's run-time data
            uint16_t unk04 = 0; // size of some sort of array in the form's run-time data
            uint16_t unk06 = 0; // size of some sort of array in the form's run-time data
            uint8_t  unk08 = 0;
            uint8_t  unk09 = 0;
            uint8_t  unk0A = 0;
            uint8_t  unk0B = 0;
            uint8_t  unk0C = 0;
            uint8_t  unk0D = 0;
         } data; // DATA
         struct { // sizeof == 0x3C
            float    unk00 = 0.9;   // 00
            float    unk04 = 0.8;   // 04
            float    unk08 = 0.4;   // 08
            float    unk0C = 0.8;   // 0C
            float    unk10 = 0.1;   // 10
            float    unk14 = 0.3;   // 14
            float    unk18 = 0;     // 18
            float    unk1C = 50;    // 1C
            float    unk20 = 50;    // 20
            float    unk24 = 25;    // 24
            float    unk28 = 25;    // 28
            float    unk2C = 50;    // 2C
            float    unk30 = 50;    // 30
            uint32_t unk34 = 10000; // 34
            uint32_t unk38 = 30000; // 38
         } rafd; // RAFD
         struct { // sizeof == 0x18
            uint16_t unk00 = 0xFFFF; // 00
            uint16_t unk02 = 0;      // 02
            uint16_t unk04 = 0;      // 04
            uint8_t  unk06 = 0;      // 06
            // padding
            float    unk08 = 0;      // 08
            float    unk0C = 0;      // 0C
            float    unk10 = 0.1;    // 10
            float    unk14 = 0;      // 14
         } raps;
         std::vector<uint16_t> rafb; // RAFB
         //
         std::string      anam;           // ANAM
         form_reference_t body_part_data; // TNAM
         form_reference_t preview_actor;  // XNAM
         uint32_t         version = 1;    // NVER

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