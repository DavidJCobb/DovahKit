#pragma once
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct keyword_list {
      std::vector<form_id_t> forms; // KSIZ, KWDA
      //
      void load(tes_subrecord_reader&);
      static void generateUseInfo(tes_subrecord_reader&, form_stub*);
   };
}