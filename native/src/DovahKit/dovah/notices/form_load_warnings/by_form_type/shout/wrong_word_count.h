#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::shout {
   //
   // A shout had the wrong number of "word" subrecords.
   //
   class wrong_word_count : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr wrong_word_count(
            form_stub& subject,
            size_t     word_count
         )
         :
            base_form_load_warning(subject),
            word_count(word_count)
         {}

         size_t word_count;
   };
}
#include "../../../_util.undef.h"