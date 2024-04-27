#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace dovah::tes_file_reading {
   class file_header_reader {
      public:
         struct flag {
            flag() = delete;
            enum type {
               master                 = 0x0001,
               localized_string_table = 0x0080,
               light                  = 0x0200, // SSE-only
            };
         };
         
      public:
         std::string name;
         uint32_t    flags = 0;
         uint32_t    record_and_group_count = 0;
         uint16_t    header_record_version  = 0;
         std::string author;
         std::string description;
         std::vector<std::string> masters;
         
      public:
         void clear();
         void load(const char* path); // throws on failure
         constexpr bool is_light() const noexcept { return (this->flags & flag::light) != 0; }
         constexpr bool is_master() const noexcept { return (this->flags & flag::master) != 0; }
   };
}
