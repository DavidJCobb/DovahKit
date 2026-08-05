#pragma once
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/keyword_list.h"
#include "components/magic_effect_list.h"
#include "components/papyrus.h"
#include "dovah/data/magic_casting_type.h"
#include "dovah/data/magic_delivery_type.h"
#include "dovah/data/magic_spell_type.h"

namespace dovah::loaded_forms {
   class Enchantment : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::enchantment;
         Enchantment(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const auto legal_spell_types = std::array{
            magic_spell_type::enchantment_normal,
            magic_spell_type::enchantment_staves,
         };

         struct flag {
            enum type : uint32_t {
               manual_cost_calc          = 0x00000001,
               extend_duration_on_recast = 0x00000004,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         components::object_bounds           bounds;      // OBND
         components::magic_effect_list       effects;     // EFID, EFIT, CTDA[]
         components::keyword_list            keywords;    // KSIZ, KWDA
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name = localized_string(localized_string_type::common); // FULL
         //
         uint32_t cost  = 0;
         flags_t  flags = 0;
         magic_casting_type  casting_type     = magic_casting_type::constant_effect;
         int32_t  charge_amount = 0; // unused
         magic_delivery_type delivery_type    = magic_delivery_type::self;
         magic_spell_type    enchantment_type = magic_spell_type::enchantment_normal;
         float charge_time = 0;
         form_reference_t base_enchantment;  // ENCH
         form_reference_t worn_restrictions; // FLST

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}