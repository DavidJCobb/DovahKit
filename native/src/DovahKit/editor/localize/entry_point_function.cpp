#include "./entry_point_function.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString entry_point_function(dovah::entry_point_function v) {
      switch (v) {
         case dovah::entry_point_function::none:
            return QCoreApplication::translate("dovah::entry_point_function", "None");
         case dovah::entry_point_function::set_value:
            return QCoreApplication::translate("dovah::entry_point_function", "Set Value");
         case dovah::entry_point_function::add_value:
            return QCoreApplication::translate("dovah::entry_point_function", "Add Value");
         case dovah::entry_point_function::multiply_value:
            return QCoreApplication::translate("dovah::entry_point_function", "Multiply Value");
         case dovah::entry_point_function::add_range_to_value:
            return QCoreApplication::translate("dovah::entry_point_function", "Add Range to Value");
         case dovah::entry_point_function::add_actor_value_mult:
            return QCoreApplication::translate("dovah::entry_point_function", "Add Actor Value Mult");
         case dovah::entry_point_function::absolute_value:
            return QCoreApplication::translate("dovah::entry_point_function", "Absolute Value");
         case dovah::entry_point_function::negative_absolute_value:
            return QCoreApplication::translate("dovah::entry_point_function", "Negative Absolute Value");
         case dovah::entry_point_function::add_leveled_list:
            return QCoreApplication::translate("dovah::entry_point_function", "Add Leveled List");
         case dovah::entry_point_function::add_activate_choice:
            return QCoreApplication::translate("dovah::entry_point_function", "Add Activate Choice");
         case dovah::entry_point_function::select_spell:
            return QCoreApplication::translate("dovah::entry_point_function", "Select Spell");
         case dovah::entry_point_function::select_text:
            return QCoreApplication::translate("dovah::entry_point_function", "Select Text");
         case dovah::entry_point_function::set_to_actor_value_mult:
            return QCoreApplication::translate("dovah::entry_point_function", "Set to Actor Value Mult");
         case dovah::entry_point_function::multiply_actor_value_mult:
            return QCoreApplication::translate("dovah::entry_point_function", "Multiply Actor Value Mult");
         case dovah::entry_point_function::multiply_one_plus_av_mult:
            return QCoreApplication::translate("dovah::entry_point_function", "Multiply 1 + Actor Value Mult");
         case dovah::entry_point_function::set_text:
            return QCoreApplication::translate("dovah::entry_point_function", "Set Text");
      }
      return "";
   }
}