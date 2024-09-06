#pragma once
#include <cstdint>
#include <limits>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"
#include "structs/movement_type_speeds.h"

namespace dovah::loaded_forms {
   class MovementType : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::movement_type;
         MovementType(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const float anim_change_threshold_disabled = std::numeric_limits<float>::max();
         
         components::papyrus_attachment_data script_data;
         //
         std::string name;
         structs::movement_type_speeds speeds; // SPED
         struct {
            float directional    = anim_change_threshold_disabled;
            float movement_speed = anim_change_threshold_disabled;
            float rotation_speed = anim_change_threshold_disabled;
         } anim_change_thresholds; // INAM

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