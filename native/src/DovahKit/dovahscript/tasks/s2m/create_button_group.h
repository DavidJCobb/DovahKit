#pragma once
#include <QButtonGroup>
#include "../_ui_base.h"

namespace dovahscript::tasks::s2m {
   class create_button_group : public _ui_write_base {
      public:
         create_button_group() {}

         QButtonGroup* created = nullptr;
         
         virtual bool is_blocking() const noexcept override { return true; }
         virtual bool is_fire_and_forget() const noexcept override { return false; }
      protected:
         virtual void _exec_impl() override;
   };
}