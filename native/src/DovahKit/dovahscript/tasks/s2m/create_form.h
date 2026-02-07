#pragma once
#include <cstdint>
#include <string>
#include "../_base.h"
#include "dovah/form_types.h"

namespace dovah {
   class form_stub;
}

namespace dovahscript::tasks::s2m {
   class create_form : public _base {
      public:
         dovah::form_type   form_type = dovah::form_type::none;
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
      protected:
         virtual void _exec_impl() override;

         bool parenthood_is_via_record_groups() const;
         bool check_non_group_parenthood() const;
         void set_up_non_group_parenthood();
   };
}