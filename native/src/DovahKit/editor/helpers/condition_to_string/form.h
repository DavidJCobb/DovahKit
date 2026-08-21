#pragma once
#include <QString>
#include "./options/form_format.h"
namespace dovah {
   class form_stub;
}

namespace editor_helpers::condition_to_string {
   extern QString form(const dovah::form_stub*, const options::form_format& = {});
}
