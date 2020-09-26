#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class ignored_by_sandbox : public empty_extra_data<'XIS2', extra_data_type::ignored_by_sandbox> {
      // The game has an XIBS subrecord, but that subrecord is skipped.
   };
}