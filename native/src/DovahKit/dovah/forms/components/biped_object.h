#pragma once
#include <type_traits>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct biped_object {
      public:
         static constexpr const uint32_t subrecord_signature_deprecated = 'BODT';
         static constexpr const uint32_t subrecord_signature_modern     = 'BOD2';

      public:
         enum class armor_type : uint32_t {
            light_armor,
            heavy_armor,
            clothing,
         };
         
         // These flags were apparently moved out of BGSBipedObjectForm and into 
         // specific form types. They're vestigial, present only in BODT.
         struct flag {
            flag() = delete;
            enum type : uint8_t {
               modulates_voice = 0x01, // from ARMA
               non_playable    = 0x10, // from ARMO
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         uint32_t        first_person_slots = 0; // mask
         flags_t         flags = 0;
         enum armor_type armor_type = armor_type::clothing;
         
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         //
         void clear(loaded_forms::Form& my_owner) noexcept;
         void clone_from(const biped_object& original, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
   };
}