#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "../../helpers/bitwise.h"
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Shout : public Form {
      public:
         static constexpr form_type_t form_type = form_type::shout;
         Shout() : Form(form_type) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               treat_as_power = 0x00000080,
            };
         };

         struct Word {
            form_reference_t word_of_power; // WOOP
            form_reference_t spell;         // SPEL
            float recoveryTime = 0.0F;
         };

         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         form_reference_t equip_type;          // ETYP; should be an EQUP form
         form_reference_t menu_display_object; // MDOB; should be a  STAT form
         std::array<Word, 3> words; // SNAM (one per word)

         void load(tes_record_reader&);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual void setup(const file_load_order&) noexcept override;

         inline bool treat_as_power() const noexcept { return (this->flags & form_flag::treat_as_power) != 0; }
         inline void treat_as_power(bool v) noexcept { cobb::modify_bit(this->flags, form_flag::treat_as_power, v); }
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}