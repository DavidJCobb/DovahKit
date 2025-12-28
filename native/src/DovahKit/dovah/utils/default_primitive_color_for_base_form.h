#pragma once
#include "../forms/structs/color_floats.h"
namespace dovah {
   class form_stub;
}

namespace dovah::utils {
   extern loaded_forms::color_floats default_primitive_color_for_base_form(const dovah::form_stub&);
}