#pragma once

namespace dovahscript {
   // Currently unused. May be redesigned and implemented in the future, or may be wholly removed.
   struct permission_set {
      struct {
         bool modify = false; // Can the script modify form data?
      } form_data;
      struct {
         bool basic = false; // Can the script create UI controls?
         bool html  = false; // Can the script use HTML-formatted text in UI controls?
      } ui;
   };
}
