#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class AnimationProp : public Form { // TESObjectANIO
      public:
         static constexpr const enum form_type form_type = form_type::animation_prop;
         AnimationProp(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_unload_event_name_length = 259; // does not include null terminator, which must be present

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               unknown = 0x00000200,
            };
         };

         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         std::string unload_event; // BNAM

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