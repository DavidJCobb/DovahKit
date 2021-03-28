#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "common.h"

namespace dovah {
   struct tes_file_header {
      struct master_entry {
         std::string master; // MAST
         uint64_t    data;   // DATA
      };
      struct detail_flag {
         detail_flag() = delete;
         enum type {
            has_intv = 0x0001,
            has_incc = 0x0002,
            has_onam = 0x0004,
            is_hardcoded_dummy = 0x0008, // used by file_reader
            is_none_stub_dummy = 0x0010, // used by file_reader
            file_extension_forces_light  = 0x0020,
            file_extension_forces_master = 0x0040,
         };
      };
      using detail_flag_t = std::underlying_type_t<detail_flag::type>;
      //
      using flag = tes_file_flag;
      //
      uint32_t      flags   = 0;
      detail_flag_t details = 0; // application-/backend-specific flags; these are not part of a saved file
      uint16_t      record_version = 0;
      float         file_version   = 0.94F;
      uint32_t      record_and_group_count = 0;
      uint32_t      nextFormID     = 0x00000800;
      std::string author;
      std::string description;
      std::vector<master_entry> masters;
      // TODO: ONAM, a list of overridden records within temporary CELLs, of the following types: ACHR, LAND, NAVM, REFR, PGRE, PHZD, PMIS, PARW, PBAR, PBEA, PCON, PFLA
      // TODO: DELE
      uint32_t subINTV;
      uint32_t subINCC;
      // TODO: SCRN
      //
      inline bool is_light() const noexcept {
         if (this->flags & flag::light)
            return true;
         return this->details & detail_flag::file_extension_forces_light;
      }
      inline bool is_master() const noexcept {
         if (this->flags & flag::master)
            return true;
         return this->details & detail_flag::file_extension_forces_master;
      }
   };
}
