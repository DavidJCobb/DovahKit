#include "lambda.h"

namespace editor_script::tasks::s2m {
   //
   // If this is not done, the "lambda" task type will leak its lambda -- and anything that 
   // the lambda captured by value!
   //
   static_assert(std::has_virtual_destructor_v<cross_thread_task>, "The base class needs to have a virtual destructor so that subclasses destroy their members properly.");
}