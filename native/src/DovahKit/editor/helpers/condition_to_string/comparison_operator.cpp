#include "./comparison_operator.h"
#include <utility> // std::unreachable
#include <QCoreApplication>
#include "dovah/data/conditions/comparison_operator.h"

#define STRING(t) QCoreApplication::translate("dovah::conditions::comparison_operator", t)

namespace editor_helpers::condition_to_string {
   extern QString comparison_operator(dovah::conditions::comparison_operator op) {
      switch (op) {
         using enum dovah::conditions::comparison_operator;
         case equal:
            return STRING("==");
         case not_equal:
            return STRING("!=");
         case greater:
            return STRING(">");
         case greater_or_equal:
            return STRING(">=");
         case less:
            return STRING("<");
         case less_or_equal:
            return STRING("<=");
      }
      std::unreachable();
      return STRING("??");
   }
}