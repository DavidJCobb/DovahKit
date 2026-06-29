#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct legacy_script {
      public:
         static constexpr const uint32_t subrecord_signature_header        = 'SCHR'; // SCript HeadeR
         static constexpr const uint32_t subrecord_signature_compiled_data = 'SCDA'; // SCript DAta
         static constexpr const uint32_t subrecord_signature_source_code   = 'SCTX'; // SCript TeXt
         static constexpr const uint32_t subrecord_signature_quest         = 'QNAM';
         static constexpr const uint32_t subrecord_signature_ref_objects   = 'SCRO'; // SCript Referenced Object (global)
         static constexpr const uint32_t subrecord_signature_ref_variables = 'SCRV'; // SCript Referenced Variable
         static constexpr const uint32_t subrecord_signature_variable_decl = 'SLSD'; // Script Local Stored Data?
         static constexpr const uint32_t subrecord_signature_variable_name = 'SCVR'; // SCript VaRiable

         enum class script_type : uint16_t {
            object = 0,
            quest  = 1,
            effect = 0x0100,
         };

         using referenced_object = std::variant<
            form_reference_t, // SCRO -> FORM
            uint32_t          // SCRV
         >;
         struct variable {
            uint32_t index = 0; // SLSD+00
            uint8_t  unk04[4];  // SLSD+04
            uint32_t unk08;     // SLSD+08
            uint32_t unk0C;     // SLSD+0C
            uint8_t  unk10[6];  // SLSD+06
            bool     is_long_or_short = false; // SLSD+16
            uint8_t  unk17;                    // SLSD+17
            std::string name; // SCVR
         };

         struct use_info_state {
            form_id_t parent_quest;
            std::vector<form_id_t> referenced_objects;

            bool load(tes_subrecord_reader&);
         };

      public:
         struct {
            uint32_t    unk00          = 0; // SCHR+00
            uint32_t    ref_obj_count  = 0; // SCHR+04
            uint32_t    compiled_size  = 0; // SCHR+08
            uint32_t    variable_count = 0; // SCHR+0C
            script_type type           = script_type::object; // SCHR+10
            uint8_t     unk11          = 0;     // SCHR+11
            bool        enabled        = false; // SCHR+12
         } header;
         std::vector<uint8_t> compiled_data;
         std::string          source_code;
         form_reference_t     parent_quest; // QNAM -> QUST
         std::vector<referenced_object> referenced_objects; // (SCRO|SCRV)[]
         std::vector<variable>          variables;          // (SLSD+SCVR)[]

         bool empty() const noexcept;

      public:
         bool load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
         bool save(tes_record_writer&, load_order_interfaces::form_save&);
         void clone_from(const legacy_script& original, loaded_forms::Form& my_owner) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         void clear(loaded_forms::Form& my_owner);
   };
}
