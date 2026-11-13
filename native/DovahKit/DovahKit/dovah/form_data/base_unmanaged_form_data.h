#pragma once
#include <cstdint>
#include <string>
#include "dovah/form_stub.h"

namespace dovah {
   class base_unmanaged_form_data {
      protected:
         constexpr base_unmanaged_form_data(form_type ft) : stub({ .form_type = ft }) {}
         constexpr base_unmanaged_form_data(const form_stub& stub) :
            stub{
               .editor_id    = std::string(stub.editor_id.data(), stub.editor_id.size()),
               .form_type    = stub.form_type,
               .record_flags = stub.get_record_flags(),
            }
         {}
      public:
         virtual ~base_unmanaged_form_data() {}

      public:
         struct {
            std::string     editor_id;
            const form_type form_type    = {};
            uint32_t        record_flags = 0;
         } stub;
   };
}
