#pragma once
#include "./base_record_error.h"

namespace dovah::exceptions::file_parse_errors {
   //
   // A record was misplaced in the wrong GRUP, in a way that DovahKit can't tolerate. 
   // DovahKit counts on some specific form types (e.g. worldspaces, cells) always 
   // being located in the correct places within the file structure, in order to be 
   // able to load content quickly.
   //
   class record_fatally_misplaced : public base_record_error {
      public:
         using base_record_error::base_record_error;
   };
}