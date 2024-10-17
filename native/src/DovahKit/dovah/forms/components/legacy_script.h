#pragma once
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct legacy_script {
      public:
         static constexpr const uint32_t subrecord_signature_header        = 'SCHR';
         static constexpr const uint32_t subrecord_signature_compiled_data = 'SCDA'; // SCript DAta
         static constexpr const uint32_t subrecord_signature_source_code   = 'SCTX'; // SCript TeXt
         static constexpr const uint32_t subrecord_signature_quest         = 'QNAM';
         static constexpr const uint32_t subrecord_signature_ref_objects   = 'SCRO'; // SCript Ref Objects (globals)
         static constexpr const uint32_t subrecord_signature_ref_variables = 'SCRV'; // SCript Ref Variables

      public:
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

         bool empty() const noexcept;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         bool save(tes_record_writer&, load_order_interfaces::form_save&);
         void clone_from(const legacy_script& original, loaded_forms::Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         void clear(loaded_forms::Form& my_owner);
   };
}
