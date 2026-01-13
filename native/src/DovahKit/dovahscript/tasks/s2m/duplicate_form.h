#pragma once
#include <cstdint>
#include <string>
#include "../_base.h"

namespace dovah {
   class form_stub;
}

namespace dovahscript::tasks::s2m {
   class duplicate_form : public _base {
      public:
         dovah::form_stub* source = nullptr;
         dovah::form_stub* parent = nullptr;
         dovah::form_stub* result = nullptr;
         std::string editorID;
         struct {
            int32_t x = 0;
            int32_t y = 0;
         } cell_grid_coordinates; // grid coordinates to use when duplicating an exterior cell
         //
         bool        error      = false;
         const char* error_text = nullptr;
         //
         virtual bool is_blocking() const noexcept override { return true; }
      protected:
         virtual void _exec_impl() override;
   };
}