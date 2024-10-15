#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/enchantable.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/detection_loudness.h"

namespace dovah::loaded_forms {
   class Explosion : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::explosion;
         Explosion(const constructor_params& c) : Form(form_type, c) {};

         enum class knockdown_type {
            never,      // 0b00
            always,     // 0b01
            by_formula, // 0b10
            only_npcs,  // 0b11
         };

         struct explosion_flag {
            explosion_flag() = delete;
            enum type : uint32_t {
               //
               use_world_orientation   = 0x002,
               knockdown_always        = 0x004, // file format detail. do not use; prefer the `knockdown` field
               knockdown_formula       = 0x008, // file format detail. do not use; prefer the `knockdown` field
               ignore_los_check        = 0x010,
               push_source_ref_only    = 0x020,
               ignore_imagespace_swap  = 0x040,
               chain                   = 0x080,
               no_controller_vibration = 0x100,
            };
         };

         components::object_bounds bounds; // OBND
         components::enchantable   enchantable; // EITM, EAMT // denotes spell to apply to targets
         components::model_ts      model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name; // FULL
         form_reference_t imagespace_modifier; // MNAM
         form_reference_t impact_data_set;     // DATA+0x0C
         form_reference_t light;               // DATA+0x00
         form_reference_t placed_object;       // DATA+0x10
         form_reference_t projectile;          // DATA+0x14
         std::array<form_reference_t, 2> sounds; // DATA+0x04, DATA+0x08
         //
         float damage = 0; // DATA+0x1C
         float force  = 0; // DATA+0x18
         float radius = 0; // DATA+0x20
         float imagespace_radius = 0; // DATA+0x24
         float vertical_offset = 0; // DATA+0x28
         uint32_t explosion_flags = 0; // DATA+0x2C
         knockdown_type knockdown = knockdown_type::never; // embedded in explosion flags
         detection_loudness loudness = detection_loudness::normal; // DATA+0x30

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