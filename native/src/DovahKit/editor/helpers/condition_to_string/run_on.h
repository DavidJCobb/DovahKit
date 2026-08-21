#pragma once
#include <cstdint>
#include <QString>
#include "./options/form_format.h"
namespace dovah {
   namespace conditions {
      enum class run_on_type : uint32_t;
   }
   namespace loaded_forms::components::conditions {
      struct context;
   }
   class form_stub;
}

namespace editor_helpers::condition_to_string {
   // returns: string; and bool indicating that the string is the name of an alias/packdata/ref (as opposed to "none," fallback text, or something else)
   extern std::pair<QString, bool> run_on(
      const dovah::loaded_forms::components::conditions::context&,
      dovah::conditions::run_on_type,
      uint32_t = -1,
      const dovah::form_stub* = nullptr,
      const options::form_format& = options::form_format{
         .form_type         = options::form_type_format::none,
         .include_editor_id = options::editor_id_presence::form_or_base,
         .include_form_id   = options::form_id_presence::if_no_editor_id,
      }
   );
}
