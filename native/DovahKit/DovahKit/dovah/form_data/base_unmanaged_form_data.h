#pragma once
#include <cstdint>
#include <string>

namespace dovah {
   class base_unmanaged_form_data {
      public:
         virtual ~base_unmanaged_form_data() {}

      public:
         struct {
            std::string editor_id;
            uint32_t    record_flags = 0;
         } stub;
   };
}
