#pragma once
#include <cstdint>
#include "./Form.h"
#include "./_common.h"
#include "./components/papyrus.h"
#include "./structs/color_dword.h"
#include "./structs/cell_lighting.h"
#include "./structs/directional_ambient_lighting_colors.h"

namespace dovah::loaded_forms {
   class LightingTemplate : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::lighting_template;
         LightingTemplate(const constructor_params& c) : Form(form_type, c) {};

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         // The directional ambient parameters in `data` are not used. For whatever 
         // reason, Bethesda preferred an alternate structure placed after it: the 
         // `directional_ambient` field.
         //
         structs::cell_lighting data;
         structs::directional_ambient_lighting_colors directional_ambient;

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