#pragma once
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct container_entry {
      // CNTO:
      form_id_t item;
      int32_t   count = 0;
      // COED:
      form_id_t owner;
      union {
         form_id_t global;
         int32_t   factionRank = 0;
      };
      float condition;
      //
      container_entry() : factionRank(0) {};
   };
   struct container_data {
      std::vector<container_entry> entries;
      //
      void load(tes_subrecord_reader&);
      static void generateUseInfo(tes_subrecord_reader&, form_stub*);
   };
}
