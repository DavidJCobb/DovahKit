#include "file_read_error.h"

namespace {
   const char* _load_error_code_names[] = {
      "no error",
      "active file is a dependency of another file",
      "file is malformed",
      "ESL contains a form ID that is too high",
      "a dependency is missing",
      "the file is missing",
      "the file is locked and cannot be opened",
      "a form had a bad form ID",
      "there are too many files in the load order",
      "the file is part of a cyclical dependency",
      "unknown/unhandled error",
      "filesystem or file I/O error",
      "insufficient memory available for this data",
      "active file is a master but there are plug-ins",
   };
}
namespace dovah {
   const char* file_read_error::code_string() const noexcept {
      if ((int)this->code < std::extent<decltype(_load_error_code_names)>::value)
         return _load_error_code_names[(int)this->code];
      return "<no string>";
   }
}