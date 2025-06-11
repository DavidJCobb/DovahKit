#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::impact_data_set {
   class mapping_missing_data : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr mapping_missing_data(
            form_stub& stub,
            size_t     which,
            form_stub* material_type,
            form_stub* impact_data
         )
         :
            base_form_load_warning(stub),
            which(which),
            material_type(material_type),
            impact_data(impact_data)
         {}

         size_t which;
         form_stub* material_type = nullptr;
         form_stub* impact_data   = nullptr;
   };
}
#include "../../../_util.undef.h"