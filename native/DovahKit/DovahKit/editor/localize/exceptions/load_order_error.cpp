#include "./load_order_error.h"
#include <QCoreApplication>
#include "dovah/exceptions/load_order_error.h"
#include "dovah/exceptions/load_order_errors/cyclical_dependency_between_files.h"
#include "dovah/exceptions/load_order_errors/desired_active_file_is_a_dependency.h"
#include "dovah/exceptions/load_order_errors/load_order_would_have_too_many_files.h"
#include "dovah/facts/game/game_supports_light_plugins.h"
#include "dovah/game.h"

namespace {
   using base_ex_type = dovah::exceptions::load_order_error;
   namespace ex_subtypes {
      using namespace dovah::exceptions::load_order_errors;
   }
}

namespace dovahkit::localize::exceptions {
   extern QString load_order_error(const base_ex_type& ex) {
      if (auto* casted = dynamic_cast<const ex_subtypes::cyclical_dependency_between_files*>(&ex)) {
         #define EXCEPTION_NAME "dovah::exceptions::load_order_errors::cyclical_dependency_between_files"
         QString list = QCoreApplication::translate(EXCEPTION_NAME, "<ul>", "list start");
         for (auto& name : casted->files_seen) {
            list += QCoreApplication::translate(EXCEPTION_NAME, "<li>%1</li>", "list item").arg(QString::fromStdString(name));
         }
         list += QCoreApplication::translate(EXCEPTION_NAME, "</ul>", "list end");

         return QCoreApplication::translate(
            EXCEPTION_NAME,
            "<p>This load order is invalid: a cyclical dependency exists between the following files:</p>%1",
            "main text"
         )
            .arg(list)
         ;
         #undef EXCEPTION_NAME
      }
      if (auto* casted = dynamic_cast<const ex_subtypes::desired_active_file_is_a_dependency*>(&ex)) {
         return QCoreApplication::translate(
            "dovah::exceptions::load_order_errors::desired_active_file_is_a_dependency",
            "This load order is invalid: the desired active file, %1, is a dependency of file %2."
         )
            .arg(casted->active_file_name)
            .arg(casted->dependent_file_name)
         ;
      }
      if (auto* casted = dynamic_cast<const ex_subtypes::load_order_would_have_too_many_files*>(&ex)) {
         #define EXCEPTION_NAME "dovah::exceptions::load_order_errors::load_order_would_have_too_many_files"
         if (dovah::facts::game_supports_light_plugins(casted->target_game)) {
            return QCoreApplication::translate(
               /*context*/  EXCEPTION_NAME,
               /*text*/     "This load order is invalid: there are too many files.\n\n"
                            "There are %1 normal files and %2 light files. The maximum amount of each kind "
                            "of file is %3 and %4, respectively.",
               /*disambig*/ "text if light files supported"
            )
               .arg(casted->file_counts.heavy)
               .arg(casted->file_counts.light)
               .arg(253)
               .arg(4095)
            ;
         } else {
            return QCoreApplication::translate(
               /*context*/  EXCEPTION_NAME,
               /*text*/     "This load order is invalid: there are too many files.\n\n"
                            "There are %1 files, but there can be no more than %2 files.",
               /*disambig*/ "text if light files not supported"
            )
               .arg(casted->file_counts.heavy)
               .arg(254)
            ;
         }
         #undef EXCEPTION_NAME
      }
      return QCoreApplication::translate("dovah::exceptions::load_order_error", "This load order is invalid.");
   }
}