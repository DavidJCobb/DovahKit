#pragma once
#include "dovah/use_info/entry_flag_to_mask.h"
#include "dovah/use_info/entry_flag_underlying_type.h"
namespace dovah {
   namespace tes {
      class subrecord_view;
   }
   class form_stub;
   enum class form_type : uint8_t;
}

namespace dovah {
   class managed_form_use {
      protected:
         form_stub* _used = nullptr;

      public:
         constexpr form_stub* get() const noexcept {
            return this->_used;
         }
         void set(form_stub& containing_form, form_stub* value);

      protected:
         void _set_with_flags(form_stub& containing_form, form_stub* value, use_info::entry_flag_underlying_type mask);

      public:
         bool stream(tes::subrecord_view& subrecord);
         void unchecked_stream(tes::subrecord_view& subrecord);
   };
   
   template<auto UseInfoEntryFlag>
   class unique_managed_form_use : protected managed_form_use {
      public:
         static constexpr const auto use_info_flag = UseInfoEntryFlag;

         using managed_form_use::get;
         using managed_form_use::stream;
         using managed_form_use::unchecked_stream;

      public:
         void set(form_stub& containing_form, form_stub* value) {
            this->_set_with_flags(containing_form, value, use_info::entry_flag_to_mask(use_info_flag));
         }
   };
}