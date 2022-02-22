#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Door : public Form {
      public:
         static constexpr form_type_t form_type = form_type::door;
         Door(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               has_distant_lod   = 0x00008000,
               random_anim_start = 0x00010000,
               is_marker         = 0x00800000,
            };
         };

         struct door_flag {
            door_flag() = delete;
            enum type : uint8_t {
               automatic   = 0x01,
               hidden      = 0x02,
               minimal_use = 0x04,
               sliding     = 0x08,
               do_not_open_in_combat_search = 0x10,
            };
         };
         using door_flags_t = std::underlying_type_t<door_flag::type>;

         components::papyrus_attachment_data script_data;
         components::object_bounds bounds;
         components::model_ts model;
         components::destruction_stage_data destruction_data; // DEST
         localized_string name; // FULL
         door_flags_t     door_flags = 0; // FNAM
         form_reference_t open_sound;  // SNAM // form type is SNDR
         form_reference_t close_sound; // ANAM // form type is SNDR
         form_reference_t loop_sound;  // BNAM // form type is SNDR
         std::vector<form_reference_t> random_destinations; // TNAM[] // Creation Kit v1.5.3.0 corrupts this when used to edit it

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}