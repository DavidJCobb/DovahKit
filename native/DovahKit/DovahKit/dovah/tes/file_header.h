#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "./file_header_flags.h"

namespace dovah::tes {
   struct file_header {
      public:
         struct master_entry {
            std::string master; // MAST
            uint64_t    data;   // DATA
         };

      public:
         file_header_flags flags;
         uint16_t          record_version = 0;
         float             file_version   = 0.94F;
         uint32_t          record_and_group_count = 0;
         uint32_t          nextFormID     = 0x00000800;
         std::string       author;
         std::string       description;
         std::vector<master_entry> masters;
         // TODO: ONAM, a list of overridden records within temporary CELLs, of the following types: ACHR, LAND, NAVM, REFR, PGRE, PHZD, PMIS, PARW, PBAR, PBEA, PCON, PFLA
         // TODO: DELE
         uint32_t subINTV             = 0; // INTV
         uint32_t interior_cell_count = 0; // INCC
         // TODO: SCRN

         // True if the "TES4" signature is in the same endianness as the system. False otherwise. 
         // All non-signature content in the file has the opposite endianness of the signatures.
         bool needs_endianness_swap = false;

      public:
         constexpr bool is_light() const noexcept {
            return this->flags.has_any_of<file_header_flag::light>();
         }
         constexpr bool is_master() const noexcept {
            return this->flags.has_any_of<file_header_flag::master>();
         }
   };
}