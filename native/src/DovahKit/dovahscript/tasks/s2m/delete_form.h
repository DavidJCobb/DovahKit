#pragma once
#include <vector>
#include "../_base.h"

struct lua_State;
namespace dovah {
   class form_stub;
}

namespace dovahscript::tasks::s2m {
   class delete_form : public _base {
      public:
         delete_form();

      protected:
         lua_State* lua_state = nullptr;

      public:
         dovah::form_stub* stub = nullptr;
         struct {
            bool        failed = false;
            const char* text   = nullptr;
         } results;
         
         virtual bool is_blocking() const noexcept override { return true; }
      protected:
         virtual void _exec_impl() override;
         virtual void run_lua_before(lua_State* L) override;
   };
}