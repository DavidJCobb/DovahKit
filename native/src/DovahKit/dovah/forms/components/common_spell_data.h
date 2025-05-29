#pragma once
#include "../_common.h"
#include "dovah/data/magic_casting_type.h"
#include "dovah/data/magic_delivery_type.h"

namespace dovah::loaded_forms::components {
   class common_spell_data { // shared by SPEL and SCRL
      public:
         static constexpr const uint32_t subrecord_signature = 'SPIT'; // SpellItem

         enum class type : uint32_t {
            spell     =  0,
            disease,
            power,
            lesser_power,
            ability,
            poison    =  5,
            addiction = 10,
            voice     = 11,
         };

         struct flag {
            enum type : uint32_t {
               manual_cost_calc  = 0x00000001,
               pc_start_spell    = 0x00020000,
               aoe_ignores_los   = 0x00080000,
               ignore_resistance = 0x00100000,
               no_absorb_reflect = 0x00200000,
               no_dual_cast_mod  = 0x00800000,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         flags_t   flags = 0;
         enum type type  = type::spell;
         uint32_t  base_cost = 0;
         float     charge_time = 0;
         magic_casting_type  casting_type =  magic_casting_type::constant_effect;
         magic_delivery_type delivery_type = magic_delivery_type::self;
         float casting_duration = 0;
         float range = 0;
         form_reference_t half_cost_perk; // PERK
      
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         //
         void clear(loaded_forms::Form& my_containing_form) noexcept;
         void clone_from(const common_spell_data& original, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;

         struct use_info_state {
            form_id_t half_cost_perk;
            //
            void read(tes_subrecord_reader&);
            void commit(form_stub_use_info_builder&);
         };
   };
}