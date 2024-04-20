#include "./form_creation_failed.h"
#include "../load_order_requests/form_creation_request.h"

namespace dovah::exceptions {
   form_creation_failed::form_creation_failed(error_code ec, const form_creation_request& request) : std::runtime_error("Failed to create a form.") {
      this->details.requested_form_type = request.requested_form_type();
      if (request.cell_grid_coordinates.has_value()) {
         auto& src = request.cell_grid_coordinates.value();
         this->details.requested_grid_coords = { .x = src.x, .y = src.y };
      }
      this->details.requested_parent = request.requested_parent_form();

      if (ec == error_code::no_form_id_available)
         this->details.form_ids_missing = 1;
   }
}