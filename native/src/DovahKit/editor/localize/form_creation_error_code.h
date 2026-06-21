#pragma once
#include <QString>
#include "dovah/exceptions/enums/form_creation_error_code.h"

namespace editor::localize {
   extern QString form_creation_error_code(dovah::exceptions::form_creation_error_code);

   namespace terse {
      extern QString form_creation_error_code(dovah::exceptions::form_creation_error_code);
   }
}