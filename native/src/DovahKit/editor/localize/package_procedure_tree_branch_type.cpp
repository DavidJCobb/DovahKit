#include "./package_procedure_tree_branch_type.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString package_procedure_tree_branch_type(dovah::packages::procedure_tree_branch_type v) {
      switch (v) {
         case dovah::packages::procedure_tree_branch_type::random:
            return QCoreApplication::translate("dovah::packages::procedure_tree_branch_type", "Random");
         case dovah::packages::procedure_tree_branch_type::sequence:
            return QCoreApplication::translate("dovah::packages::procedure_tree_branch_type", "Sequential");
         case dovah::packages::procedure_tree_branch_type::simultaneous:
            return QCoreApplication::translate("dovah::packages::procedure_tree_branch_type", "Simultaneous");
         case dovah::packages::procedure_tree_branch_type::stacked:
            return QCoreApplication::translate("dovah::packages::procedure_tree_branch_type", "Stacked");
      }
      return QCoreApplication::translate("dovah::packages::procedure_tree_branch_type", "Unknown #%1").arg((size_t)v);
   }
}