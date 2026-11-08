#pragma once
#include <string>
#include <unordered_map>
#include "dovah/types/bare_localized_string_id.h"
#include "dovah/utils/localization/data_by_string_category.h"

namespace dovah::load_order_processes::active_file_strings_to_localized {
   using string_to_id_maps = utils::localization::data_by_string_category<std::unordered_map<std::string, bare_localized_string_id>>;
}
