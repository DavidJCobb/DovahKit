#include "./bulk_string_substitution.h"
#include <QMessageBox>
#include "qt/utils/bulk_string_substitution.h"

namespace DovahKitDebug::features::qt {
   /*static*/ void bulk_string_substitution::execute(QWidget* from) {
      using bss = ::dovahkit::qt::utils::bulk_string_substitution;

      auto run_test = [from](QString name, QString format, auto&&... args) {
         bss  subber(format);
         auto result = subber.exec(args...);
         QMessageBox::information(from, name, result);
      };

      run_test(
         QString("Test 1: tokens at start and end; integer values"),
         QString("%1 Test %2"),
         1,
         2
      );
      run_test(
         QString("Test 2: token #1 at start; integer values"),
         QString("%1 Test"),
         1,
         2
      );
      run_test(
         QString("Test 3: token #1 at end; integer values"),
         QString("Test %1"),
         1,
         2
      );
      run_test(
         QString("Test 4: token #2 at end; integer values"),
         QString("Test %2"),
         1,
         2
      );
      run_test(
         QString("Test 5: token #3 (out of bounds) at end"),
         QString("Test %3"),
         1,
         2
      );
      run_test(
         QString("Test 6: token #0 (out of bounds) at end"),
         QString("Test %0"),
         1,
         2
      );
      run_test(
         QString("Test 7: percentage sign not indicating a token"),
         QString("Test % Foo"),
         1,
         2
      );
      run_test(
         QString("Test 8: percentage sign not indicating a token (followed by L)"),
         QString("Test %L Foo"),
         1,
         2
      );
      run_test(
         QString("Test 9: tokens at start and end; string values"),
         QString("%1 Test %2"),
         QString("A"),
         QString("B")
      );
      run_test(
         QString("Test 10: token A at start; string values"),
         QString("%1 Test"),
         QString("A"),
         QString("B")
      );
      run_test(
         QString("Test 11: mixed values"),
         QString("The values are %1 and %2."),
         QString("A"),
         -5
      );
      run_test(
         QString("Test 11: mixed values, use 1 and 3, skip 2"),
         QString("The values are %1 and %3."),
         QString("A"),
         -5,
         10
      );
   }
}
