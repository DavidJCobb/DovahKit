#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/biped_object.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/detection_loudness.h"
#include "../utils/data_by_sex.h"

namespace dovah::loaded_forms {
   class ArmorAddon : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::armor_addon;
         ArmorAddon(const constructor_params& c) : Form(form_type, c) {};

         struct addon_flag {
            enum type : uint8_t {
               enable_weight_slider = 0x02,
            };
         };
         using addon_flags_t = std::underlying_type_t<addon_flag::type>;

         struct addon_data {
            uint8_t       priority = 0; // Male: DATA+0x00; Female: DATA+0x01
            addon_flags_t flags    = 0; // Male: Data+0x02; Female: Data+0x03
            struct {
               components::model_ts third_person; // MOD*, MO*T, MO*S (male: 2; female: 3)
               components::model_ts first_person; // MOD*, MO*T, MO*S (male: 4; female: 5)
            } models;
            struct {
               form_reference_t base;      // NAM0 (male) / NAM1 (female) -> TXST
               form_reference_t swap_list; // NAM2 (male) / NAM3 (female) -> FLST
            } skin_texture;
         };

      public:
         components::biped_object  biped_object; //  BODT, BOD2
         components::object_bounds bounds; // OBND
         components::papyrus_attachment_data script_data; // VMAD
         //
         form_reference_t art_object;     // ONAM -> ARTO
         form_reference_t footstep_sound; // SNDD -> FSTS
         data_by_sex<addon_data> graphics;
         uint8_t loudness = 0; // DATA+0x06
         struct {
            form_reference_t primary; // RNAM -> RACE
            std::vector<form_reference_t> additional; // MODL[] -> RACE
         } races;
         float weapon_adjust = 0; // DATA+0x08

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}