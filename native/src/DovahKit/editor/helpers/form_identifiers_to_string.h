#pragma once
#include <QString>
#include "dovah/core.h"

namespace dovah {
   class form_stub;
}

namespace editor_helpers {
   extern QString form_id_to_string(dovah::bare_form_id_t);
   extern QString form_signature_to_string(const dovah::form_stub*);

   extern QString form_identifiers_to_string(const dovah::form_stub*); // of the form "[TYPE:01234567]EditorID"
}