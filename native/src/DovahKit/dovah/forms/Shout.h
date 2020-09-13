#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Shout : public Form {
      public:
         Shout() : Form(form_type::shout) {};

         struct Word {
            form_id_t wordOfPowerID;
            form_id_t spellID;
            float     recoveryTime = 0.0F;
         };

         localized_string name; // FULL
         localized_string description; // DESC
         form_id_t menuDisplayObjectID; // MDOB
         std::vector<Word> words; // SNAM // a shout should always have exactly 3 of these, but we want to account for cases where they do not
         bool treatAsPower = false; // form flag 0x00000080

         void load(tes_record_reader&); // TODO: FINISH ME
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}