#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::papyrus {
   //
   // VMAD fragment data for TopicInfos specifies an overall scriptname, and then 
   // each individual fragment (OnBegin and OnEnd) specifies a scriptname. If any 
   // fragment has an inconsistent scriptname from the overall one, that fragment 
   // will be cleared.
   // 
   // Other fragment types don't appear to have this same behavior.
   //
   class inconsistent_fragment_scriptname : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class fragment_type {
            on_begin,
            on_end,
         };

      public:
         constexpr inconsistent_fragment_scriptname(
            form_stub& subject,
            fragment_type ft,
            const std::string& overall,
            const std::string& fragment
         ) :
            base_form_load_warning(subject),
            fragment(ft),
            scriptnames({ overall, fragment })
         {}

         fragment_type fragment;
         struct {
            std::string overall;
            std::string fragment;
         } scriptnames;
   };
}
#include "../../../_util.undef.h"