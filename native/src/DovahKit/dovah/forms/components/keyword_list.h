#pragma once
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct keyword_list {
      std::vector<form_id_t> forms; // KSIZ, KWDA
      //
      void load(tes_subrecord_reader&);
      static void generateUseInfo(tes_subrecord_reader&, form_stub*);
      void save(tes_record_writer&);
      //
      void clear(form_stub& my_owner) noexcept;
      void clone_from(const keyword_list& original, form_stub& my_owner) noexcept;
   };
}