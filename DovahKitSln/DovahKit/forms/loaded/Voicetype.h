#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"

class TESPluginRecord;

SCOPE_ENUM(voicetype_flags, enum voicetype_flags : uint8_t {
   allow_default_dialogue = 0x01,
   female                 = 0x02,
});
namespace LoadedForms {
   class Voicetype : public Form {
      public:
         using Flags = voicetype_flags;
      public:
         Voicetype() : Form(FormType::Voicetype) {};

         uint8_t flags = 0;

         void load(TESPluginRecord&);
         static void generateUseInfo(TESPluginRecord&, FormStub*);
   };
}