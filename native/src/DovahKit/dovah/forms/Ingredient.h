#pragma once
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/magic_effect_list.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "dovah/data/magic_casting_type.h"
#include "dovah/data/magic_delivery_type.h"

namespace dovah::loaded_forms {
   class Ingredient : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::ingredient;
         Ingredient(const constructor_params& c) : Form(form_type, c), effects(4) {};

         struct flag {
            enum type : uint32_t {
               manual_cost_calc   = 0x00000001,
               food_item          = 0x00000002,
               references_persist = 0x00000100,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         components::object_bounds           bounds;      // OBND
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::magic_effect_list       effects;     // EFID, EFIT, CTDA[]
         components::model_ts                model;       // MODL, MODT, MODS
         components::keyword_list            keywords;    // KSIZ, KWDA
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name = localized_string(localized_string_type::common); // FULL
         //
         flags_t  flags = 0; // ENIT+0x04
         int32_t  effect_value = 0; // ENIT+0x00
         form_reference_t equip_type; // ETYP
         struct {
            std::string inventory; // ICON
            std::string message;   // MICO
         } icons;
         struct {
            form_reference_t take; // YNAM
            form_reference_t drop; // ZNAM
         } sounds;

         int32_t value  = 0; // DATA+0x00
         float   weight = 0; // DATA+0x04

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}