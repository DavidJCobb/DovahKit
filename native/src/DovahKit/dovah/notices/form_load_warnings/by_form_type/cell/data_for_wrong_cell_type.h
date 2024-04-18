#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::cell {
   //
   // While loading an interior cell, we encountered subrecords that are only appropriate for 
   // exterior cells; or vice versa.
   //
   class data_for_wrong_cell_type : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class cell_type {
            interior,
            exterior,
         };

      public:
         constexpr data_for_wrong_cell_type(
            form_stub& subject,
            cell_type  type,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(subject),
            current_cell_type(type),
            subrecord_signature(subrecord_signature)
         {}

         cell_type current_cell_type;
         uint32_t  subrecord_signature = 0;
   };
}
#include "../../../_util.undef.h"