#pragma once
#include <QString>

namespace dovah::notices {
   class base_warning;
}

namespace editor_helpers {
   extern QString backend_warning_to_string(const dovah::notices::base_warning&);
}