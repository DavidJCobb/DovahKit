#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/data/face_fx/phonemes.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   //
   // A race can specify a list of defined vertex-weight names for use in 
   // lip synching. It can then specify, for each FaceFX phoneme, a list 
   // of values to apply to each of those weights. The list of values for 
   // a phoneme must be of the same length as the list of defined weights.
   // 
   // We only check for this error on defined phonemes. If a race supplies 
   // data for more phonemes than actually exist in FaceFX, we don't check 
   // that the extra phonemes have the right number of weight values, and 
   // we don't emit this warning for them if they don't.
   //
   class wrong_weight_count_per_phoneme : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr wrong_weight_count_per_phoneme(
            form_stub& stub,
            dovah::face_fx::phoneme phoneme,
            size_t count,
            size_t desired_count
         )
         :
            base_form_load_warning(stub),
            phoneme(phoneme),
            count(count),
            desired_count(desired_count)
         {}

         dovah::face_fx::phoneme phoneme;
         size_t count;
         size_t desired_count;
   };
}
#include "../../../_util.undef.h"