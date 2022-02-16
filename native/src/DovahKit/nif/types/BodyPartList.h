#pragma once
#include <cstdint>
#include <type_traits>
#include "BodyPartIndex.h"

namespace nifDK {
   class file_reader;

   struct BodyPartList {
      static constexpr size_t serialized_size = 4;

      struct flag {
         enum type : uint16_t {
            visible_in_editor        = (1 << 0),
            start_new_shared_boneset = (1 << 8),
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      flags_t       flags = 0;
      BodyPartIndex parts = 0;

      void read(file_reader&);
      void unchecked_read(file_reader&);
   };
}