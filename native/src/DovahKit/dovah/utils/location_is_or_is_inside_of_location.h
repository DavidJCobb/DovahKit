#pragma once
namespace dovah {
   class form_stub;
}

namespace dovah::utils {
   extern bool location_is_or_is_inside_of_location(form_stub& subject, const form_stub& desired);
}