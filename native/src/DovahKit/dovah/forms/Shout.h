#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../../helpers/bitwise.h"
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Shout : public Form {
      public:
         static constexpr form_type_t form_type = form_type::shout;
         Shout() : Form(form_type) {
            this->words.reserve(3);
         };

         struct Word {
            form_id_t wordOfPowerID;
            form_id_t spellID;
            float     recoveryTime = 0.0F;
         };

         localized_string name; // FULL
         localized_string description; // DESC
         form_id_t menuDisplayObjectID; // MDOB
         std::vector<Word> words; // SNAM // a shout should always have exactly 3 of these, but we want to account for cases where they do not

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);

         inline bool treat_as_power() const noexcept { return (this->flags & 0x00000080) != 0; }
         inline void treat_as_power(bool v) noexcept { cobb::modify_bit(this->flags, 0x00000080, v); }
   };
}