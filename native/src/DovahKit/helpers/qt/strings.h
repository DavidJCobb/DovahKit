#pragma once
#include <cstdint>
#include <QString>

namespace cobb::qt {
   extern QString four_cc_to_string(uint32_t);
   extern QString winapi_code_to_string(uint32_t);
}