#pragma once

namespace dovah::use_info {
   enum class management_mode {
      // When form data refers to another form, or to a localized string, 
      // it does so using bare pointers.
      unmanaged,

      // When form data refers to another form, or to a localized string, 
      // it does so via a "use" struct which automatically updates Use 
      // Info when modified.
      managed,
   };
}