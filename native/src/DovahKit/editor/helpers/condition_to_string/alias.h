#pragma once
#include <cstdint>
#include <QString>
namespace dovah::loaded_forms::components::conditions {
   struct context;
}

namespace editor_helpers::condition_to_string {
   // returns: string; and bool indicating that the string is the name of an alias (as opposed to "none" or fallback text)
   extern std::pair<QString, bool> alias(
      const dovah::loaded_forms::components::conditions::context&,
      uint32_t alias_id,
      bool trim_alias_name
   );
}
