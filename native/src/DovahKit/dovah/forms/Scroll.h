#pragma once
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/common_spell_data.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/magic_effect_list.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Scroll : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::scroll;
         Scroll(const constructor_params& c) : Form(form_type, c) {};

         components::object_bounds           bounds;      // OBND
         components::common_spell_data       common_data; // SPIT
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::magic_effect_list       effects;     // EFID, EFIT, CTDA[]
         components::model_ts                model;       // MODL, MODT, MODS
         components::keyword_list            keywords;    // KSIZ, KWDA
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         //
         int32_t value  = 0;
         float   weight = 0.0F;
         form_reference_t equip_type;          // ETYP
         form_reference_t menu_display_object; // MDOB
         struct {
            form_reference_t take; // YNAM
            form_reference_t drop; // ZNAM
         } sounds;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}