#pragma once
#include <QString>

namespace dovah::notices {
   class base_error;
}

namespace editor_helpers {
   extern QString backend_error_to_string(const dovah::notices::base_error&);
}