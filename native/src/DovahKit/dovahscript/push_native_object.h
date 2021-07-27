#pragma once
#include "../lua.h"

class LuaManagedResource;
class QObject;
namespace dovah {
   class form_stub;
}

namespace dovahscript {
   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   int push_native_object(dovah::form_stub*);

   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   int push_native_object(LuaManagedResource*);

   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   int push_native_object(QObject*);
}