#include "form_identifiers_to_string.h"
#include "dovah/form_stub.h"
#include "helpers/qt/strings.h"

namespace editor_helpers {
   extern QString form_id_to_string(dovah::bare_form_id_t id) {
      return QString("%1").arg(id, 8, 16, QChar('0')).toUpper();
   }
   extern QString form_signature_to_string(const dovah::form_stub* stub) {
      if (!stub)
         return "NONE";
      return cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->formType).signature);
   }

   extern QString form_identifiers_to_string(const dovah::form_stub* stub) {
      if (!stub)
         return QString("[NONE:00000000]");
      return QString("[%1:%2]%3")
         .arg(form_signature_to_string(stub))
         .arg(form_id_to_string(stub->formID))
         .arg(stub->get_editor_id());
   }
}