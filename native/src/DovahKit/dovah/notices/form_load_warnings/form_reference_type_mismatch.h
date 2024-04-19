#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "../base_form_load_warning.h"

#include "../../form_types.h"

#include "../_util.define.h"
namespace dovah::notices::form_load_warnings {
   //
   // A form contains a property meant to refer to other forms of a given type, 
   // but the form ID loaded corresponds to a form of the wrong type. For example, 
   // an Activator listing a Quest as its water type.
   //
   class form_reference_type_mismatch final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         struct metadata_type {
            std::optional<size_t> nth_reference;
         };

      public:
         constexpr form_reference_type_mismatch(
            form_stub& subject,
            form_stub& target,
            form_type  desired,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(subject),
            target(target),
            desired{ desired },
            subrecord_signature(subrecord_signature)
         {}

         template<size_t Size>
         constexpr form_reference_type_mismatch(
            form_stub& subject,
            form_stub& target,
            const std::array<form_type, Size>& desired,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(subject),
            target(target),
            subrecord_signature(subrecord_signature)
         {
            this->desired.resize(Size);
            for (size_t i = 0; i < Size; ++i)
               this->desired[i] = desired[i];
         }

         form_stub&             target;
         std::vector<form_type> desired; // NOTE: If this member == { form_type::reference }, then any REFR subclass would be accepted as well.
         uint32_t               subrecord_signature = 0;

         metadata_type metadata;
   };
}
#include "../_util.undef.h"