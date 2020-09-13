#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "file_read_error.h"

namespace dovah {
   class file_header {
      public:
         struct flag {
            flag() = delete;
            enum type {
               master                 = 0x0001,
               localized_string_table = 0x0080,
               light                  = 0x0200, // SSE-only
            };
         };
         //
         std::string name;
         uint32_t    flags = 0;
         uint32_t    record_and_group_count = 0;
         std::string author;
         std::string description;
         std::vector<std::string> masters;
         //
         file_read_error load(const char* path) noexcept; // if result.defined() == false, then the load operation succeeded
         inline bool is_master() const noexcept { return (this->flags & flag::master) != 0; }
   };
}
