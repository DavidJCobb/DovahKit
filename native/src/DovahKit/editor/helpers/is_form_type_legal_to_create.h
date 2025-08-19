#pragma once
#include "dovah/form_types.h"

namespace editor_helpers {
   // Enforces some hardcoded limitations, as well as limitations that the backend 
   // intentionally does not enforce (e.g. refusing to let you create new AVIFs).
   extern bool is_form_type_legal_to_create(dovah::form_type);
}
