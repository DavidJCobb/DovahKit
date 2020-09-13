#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Voicetype : public Form {
      public:
         struct flag {
            flag() = delete;
            enum type : uint8_t {
               allow_default_dialogue = 0x01,
               female                 = 0x02,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;
         //
      public:
         Voicetype() : Form(form_type::voicetype) {};

         flags_t flags = 0;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}