#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "../components.h"

class TESPluginRecord;

namespace LoadedForms {
   class Shout : public Form {
      public:
         Shout() : Form(FormType::Shout) {};

         struct Word {
            form_id_t wordOfPowerID;
            form_id_t spellID;
            float     recoveryTime = 0.0F;
         };

         LStringRef name; // FULL
         LStringRef description; // DESC
         form_id_t menuDisplayObjectID; // MDOB
         std::vector<Word> words; // SNAM // a shout should always have exactly 3 of these, but we want to account for cases where they do not
         bool treatAsPower = false; // form flag 0x00000080

         void load(TESPluginRecord&); // TODO: FINISH ME
         static void generateUseInfo(TESPluginRecord&, FormStub*);
   };
}