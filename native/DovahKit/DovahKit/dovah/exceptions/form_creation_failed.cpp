#include "./form_creation_failed.h"

namespace dovah::exceptions {
   form_creation_failed::form_creation_failed(error_code ec, const requests::create_form& request) : std::runtime_error("Failed to create a form."), code(ec) {
      this->details.requested_form_type   = request.form_type;
      this->details.requested_grid_coords = request.grid_position;
      this->details.requested_parent      = request.child_of;

      if (ec == error_code::no_form_id_available)
         this->details.form_ids_missing = 1;
   }
}