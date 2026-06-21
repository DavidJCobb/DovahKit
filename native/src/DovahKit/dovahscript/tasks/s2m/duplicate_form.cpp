#include "./duplicate_form.h"
#include "editor/core.h"
#include "editor/helpers/make_editor_id_for_duplicate.h"
#include "editor/localize/form_creation_error_code.h"
#include "dovah/exceptions/form_creation_failed.h"

namespace {
   using exception  = dovah::exceptions::form_creation_failed;
   using error_code = exception::error_code;

   std::string _explain_error_code(error_code code) {
      constexpr const std::string_view prefix = "failed to duplicate this form or one of its child forms: ";

      std::string out;
      {
         auto text = editor::localize::terse::form_creation_error_code(code).toUtf8();
         if (text.isEmpty()) {
            text = "unknown error";
         }
         out.reserve(prefix.size() + text.size());
         out = prefix;
         out += std::string_view(text.data(), text.size());
      }
      return out;
   }
}

namespace dovahscript::tasks::s2m {
   /*virtual*/ void duplicate_form::_exec_impl() /*override*/ {
      assert(this->source);
      auto request = DovahKitCore::get().request_form_duplication();
      request.set_target(this->source);
      request.set_parent_form(this->parent);
      if (this->editorID.empty()) {
         request.editorID = editor_helpers::make_editor_id_for_duplicate(this->source->get_editor_id()).toStdString();
      } else {
         request.editorID = this->editorID;
      }
      request.cell_grid_coordinates = {
         .x = this->cell_grid_coordinates.x,
         .y = this->cell_grid_coordinates.y,
      };

      try {
         this->result = request.commit();
      } catch (const exception& ex) {
         this->error      = true;
         this->error_text = _explain_error_code(ex.code);
         return;
      }
      if (!this->result) {
         this->error      = true;
         this->error_text = "cannot duplicate the form for an unknown reason";
      }
   }
}