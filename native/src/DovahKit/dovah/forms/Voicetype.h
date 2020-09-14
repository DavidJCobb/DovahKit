#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Voicetype : public Form {
      public:
         struct voicetype_flag {
            voicetype_flag() = delete;
            enum type : uint8_t {
               allow_default_dialogue = 0x01,
               female                 = 0x02,
            };
         };
         using voicetype_flags_t = std::underlying_type_t<voicetype_flag::type>;
         //
      public:
         static constexpr form_type_t form_type = form_type::voicetype;
         Voicetype() : Form(form_type) {};

         voicetype_flags_t voicetype_flags = 0;

         void load(tes_record_reader&);
         static void generateUseInfo(tes_record_reader&, form_stub*);
   };
}