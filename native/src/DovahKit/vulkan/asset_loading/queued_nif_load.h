#pragma once
#include <glm/glm.hpp>
#include "dovah/forms/components/model.h"
#include "dovah/forms/Form.h"
#include "dovah/form_stub.h" // for loaded_form_ptr

namespace vulkanDK {
   class rendered_nif;
}

namespace vulkanDK::asset_loading {
   using form_model_data    = dovah::loaded_forms::components::model;
   using form_keepalive_ptr = dovah::loaded_form_ptr<dovah::loaded_forms::Form>;

   struct queued_nif_load {
      struct {
         form_keepalive_ptr loaded_form; // keep the form loaded until we're done with it
         form_model_data*   model = nullptr;
      } form_data;
      rendered_nif* nif = nullptr;
      glm::mat4     transform;
   };
}
