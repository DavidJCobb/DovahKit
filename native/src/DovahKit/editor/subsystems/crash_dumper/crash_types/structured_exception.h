#pragma once
#include <optional>
#include <vector>
#include "helpers/windows.h"

namespace dovahkit::subsystems::crash_dumper::crash_types {
   struct structured_exception {
      std::optional<CONTEXT>        context;
      std::vector<EXCEPTION_RECORD> records;

      static void send(HANDLE stream, const EXCEPTION_POINTERS&);
      bool read(HANDLE stream);
   };
}