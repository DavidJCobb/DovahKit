#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/forms/SoundDescriptor.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::sound_descriptor {
   //
   // Unrecognized sound typename hash in SNDR/CNAM.
   //
   class unrecognized_cnam : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         static constexpr const uint32_t correct_hash = (uint32_t)loaded_forms::SoundDescriptor::descriptor_type::standard;

      public:
         constexpr unrecognized_cnam(
            form_stub& stub,
            uint32_t   hash
         )
         :
            base_form_load_warning(stub),
            hash(hash)
         {}

         uint32_t hash;
   };
}
#include "../../../_util.undef.h"