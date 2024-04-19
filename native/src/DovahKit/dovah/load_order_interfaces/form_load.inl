#pragma once
#include "./form_load.h"
#include "../notices/form_load_warnings/form_reference_type_mismatch.h"
#include "../notices/form_load_warnings/unrecognized_subrecord.h"
#include "../form_stub.h"

namespace dovah::load_order_interfaces {
   template<size_t Size>
   void form_load::warn_if_ref_is_wrong_type(
      form_stub* target,
      const std::array<form_type, Size>& desired,
      uint32_t subrecord_signature
   ) {
      if (!target)
         return;
      for (auto ft : desired)
         if (target->form_type == ft)
            return;
      notices::form_load_warnings::form_reference_type_mismatch notice(
         const_cast<form_stub&>(this->target_stub), // TODO: clean things up so we don't need const cast lol
         *target,
         desired,
         subrecord_signature
      );
      this->log_load_warning(notice);
   }
}
