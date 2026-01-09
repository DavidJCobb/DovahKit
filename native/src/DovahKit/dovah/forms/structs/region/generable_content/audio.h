#pragma once
#include <cstdint>
#include <vector>
#include "../../../../form_reference_t.h"
namespace dovah::loaded_forms {
   class Form;
}

namespace dovah::loaded_forms::structs::region::generable_content {
   class audio {
      public:
         struct ambient_sound {
            public:
               struct flag {
                  flag() = delete;
                  enum : uint32_t {
                     weather_pleasant = 1 << 0,
                     weather_cloudy   = 1 << 1,
                     weather_rainy    = 1 << 2,
                     weather_snowy    = 1 << 3,
                  };
               };

            public:
               form_reference_t form; // -> SNDR|SOUN
               uint32_t         flags = 0;
               float            chance;
         };

      public:
         std::vector<ambient_sound> ambient_sounds; // RDSA
         form_reference_t music; // RDMO -> MUSC

      public:
         void clear(dovah::loaded_forms::Form& my_containing_form);
         void clone_from(dovah::loaded_forms::Form& my_containing_form, const audio&);
         void sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub&);

         void copy_insert_from(dovah::loaded_forms::Form& my_containing_form, const audio&);
   };
}