#pragma once

namespace dovah::form_data_helpers {
   template<typename T>
   concept is_visitable = requires(T& f, const T& cf) {
      { T::visit_fields(f,  [](auto& field) {}) };
      { T::visit_fields(cf, [](const auto& field) {}) };
   };
}