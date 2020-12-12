#pragma once
#include <QString>
#include "../../dovah/detailed_notice.h"

namespace editor_helpers {
   extern QString warning_or_error_to_string(const dovah::detailed_notice&);
}