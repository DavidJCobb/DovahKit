#include "./integer_divide_by_zero.h"
#include <QMessageBox>

/*
   Strictly speaking, integer division by zero is just undefined behavior in 
   C++. Commonly, x64/x64 CPUs will generate a hardware exception when it's 
   attempted, which for purposes is a crash.
*/

namespace DovahKitDebug::features::crash_tests {
   // MSVC detects expressions like `1 / 0` and throws a compiler error on them. 
   // Normally that's what you want, but in this case I am actively trying to 
   // write code that will break the program, so... Let's trick MSVC.
   static int trick_the_compiler(int divisor) {
      return 1 / divisor;
   }

   /*static*/ void integer_divide_by_zero::execute(QWidget* from) {
      volatile int v = trick_the_compiler(0);

      // ensure the variable is used
      QMessageBox::information(from, "This should never appear", QString("If you see this, then your processor didn't generate a hardware exception on divide-by-zero, OR something caught it for us.").arg(v));
   }
}