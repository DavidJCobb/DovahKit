#include "unknown.h"

namespace {
   using namespace dovahscript;
   using cls = wrappers::resource::unknown;
}

namespace dovahscript::wrappers::resource {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = no_functions;
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;
}