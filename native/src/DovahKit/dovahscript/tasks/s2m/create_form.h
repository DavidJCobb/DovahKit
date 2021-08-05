#pragma once
#include <string>
#include "../_base.h"
#include "../../../dovah/core.h"

namespace dovah {
   class form_stub;
}

namespace dovahscript::tasks::s2m {
   class create_form : public _base {
      public:
         dovah::form_type_t form_type = dovah::form_type::none;
         dovah::form_stub*  parent    = nullptr;
         dovah::form_stub*  result    = nullptr;
         std::string editorID;
         struct {
            int32_t x = 0;
            int32_t y = 0;
            bool    present = false;
         } cell_grid_coordinates; // grid coordinates to use when creating an exterior cell
         //
         bool        error      = false;
         const char* error_text = nullptr;
         //
         virtual bool is_blocking() const noexcept override { return true; }
         virtual bool is_fire_and_forget() const noexcept override { return false; }
      protected:
         virtual void _exec_impl() override;
   };
}