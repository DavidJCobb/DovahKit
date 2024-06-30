#pragma once

namespace ui::types {
   enum class log_item_context {
      unspecified,
      file_load, // initial loading of files
      form_load, // on-demand loading of a form's full data
      file_save, // the overall process of saving the active file
      form_save, // saving of a form's full data, as part of the overall process of saving the active file
   };
}