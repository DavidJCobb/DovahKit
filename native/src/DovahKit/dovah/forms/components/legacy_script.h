#pragma once
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct legacy_script {
      struct {
         uint32_t unk00 = 0;
         uint32_t refs  = 0; // number of REFRs used; should equal SCRO + SCRV subrecord count
         uint32_t size  = 0; // size of compiled data
         uint32_t vars  = 0;
         uint32_t type  = 0x00010000;
      } header;
      std::vector<uint8_t> compiled_data;
      std::string          source_code;
      form_reference_t     quest;
      std::vector<form_reference_t> refs;
      //
      bool empty() const noexcept;
      //
      void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
      bool save(tes_record_writer&, load_order_interfaces::form_save&);
      void clone_from(const legacy_script& original, loaded_forms::Form& my_owner) noexcept;
      void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
      void clear(loaded_forms::Form& my_owner);
   };
}
