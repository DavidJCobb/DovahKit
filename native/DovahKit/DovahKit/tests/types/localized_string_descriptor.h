#pragma once
#include <string>
#include <tuple>
#include "dovah/localization/category.h"
#include "dovah/types/bare_localized_string_id.h"

namespace dovahkit::tests::types {
   // filename, category, ID
   using localized_string_descriptor = std::tuple<std::string, dovah::localization::category, dovah::bare_localized_string_id>;
}
