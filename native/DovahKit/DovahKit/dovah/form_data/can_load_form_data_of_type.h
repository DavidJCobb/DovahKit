#pragma once
#include <cstdint>
namespace dovah {
   enum class form_type : uint8_t;
}

namespace dovah {
   extern bool can_load_form_data_of_type(form_type);
}