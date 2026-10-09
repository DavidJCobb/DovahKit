#pragma once

namespace ui::object_window {
   enum class file_source_requirement {
      any_files,

      // Only show forms that were originally defined in the active file.
      active_file_definitions,

      // Only show forms that were defined in, or are overridden by, the active file.
      active_file_records,
   };
}