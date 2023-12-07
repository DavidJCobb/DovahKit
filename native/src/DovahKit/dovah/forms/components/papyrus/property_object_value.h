#pragma once
#include <cstdint>
#include "./_forward_declare_file_handling.h"
#include "../../../core.h"

namespace dovah::loaded_forms {
   namespace components::papyrus {
      class attachment_header;
   }
   class Form;
}

namespace dovah::loaded_forms::components::papyrus {
   struct property_object_value { // if object_format == 2, then the order of fields is reversed in the file
      public:
         static constexpr const uint16_t no_alias = -1;

      public:
         form_reference_t form;
         uint16_t alias_id = -1;
         uint16_t always_zero = 0;
      
         static constexpr int serialized_size = sizeof(bare_form_id_t) + sizeof(alias_id) + sizeof(always_zero);
      
         bool load(const attachment_header& header, tes_subrecord_reader&);
         bool save(const attachment_header& header, tes_subrecord_writer&) const noexcept;
         void clone_from(const property_object_value& source, loaded_forms::Form& owner_of_clone) noexcept;
         void clear(loaded_forms::Form& owner);
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
   };
}