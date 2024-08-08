#pragma once
#include <array>
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"
#include "../data/skills.h"

namespace dovah::loaded_forms {
   class Class : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::combat_class;
         Class(const constructor_params& c) : Form(form_type, c) {};

         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name;        // FULL
         localized_string description; // DESC
         std::string      icon;        // ICON
         struct {
            dovah::skill skill     = dovah::skill::one_handed;
            uint8_t      max_level = 0; // 0 means this isn't a skill trainer
         } training;
         union _ {
            ~_() { list.~array(); }

            std::array<uint8_t, 4> list = { 0 };
            struct {
               uint8_t health;
               uint8_t magicka;
               uint8_t stamina;
               uint8_t unknown; // possibly padding; need to verify
            };
         } attribute_weights;

         std::array<uint8_t, dovah::skill_count> skill_weights = { 0 }; // indices are `dovah::skill` values
         float    bleedout_default = 0;
         int32_t  voice_points     = 0;
         uint32_t unk30 = 0; // offset 0x00 in DATA; offset 0x30 in TESClass

         constexpr const bool is_trainer() const noexcept {
            return this->training.max_level != 0;
         }

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