#pragma once
#include <QString>
namespace dovah::conditions {
   enum class comparison_operator;
}

namespace editor_helpers::condition_to_string {
   extern QString comparison_operator(dovah::conditions::comparison_operator);
}
