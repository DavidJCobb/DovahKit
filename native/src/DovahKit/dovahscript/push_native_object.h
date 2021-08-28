#pragma once
#include "../lua.h"

class QObject;
namespace dovah {
   class form_reference_t;
   class form_stub;
}
namespace dovahscript {
   class DovahscriptResource;
}

namespace dovahscript {
   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   extern int push_native_object(dovah::form_stub*);

   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   extern int push_native_object(const dovah::form_reference_t&);

   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   extern int push_native_object(DovahscriptResource*);

   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   extern int push_native_object(QObject*);
}