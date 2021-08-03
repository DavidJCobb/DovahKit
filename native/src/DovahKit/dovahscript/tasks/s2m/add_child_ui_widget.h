#pragma once
#include <QWidget>
#include "../_ui_base.h"
#include "../../task_reference.h"

namespace dovahscript::tasks::s2m {
   class add_child_ui_widget : public _ui_write_base {
      public:
         add_child_ui_widget() {}

         enum class error_code {
            none,
            would_be_cyclical,
            unknown_layout_type,
            child_has_a_forced_parent,
         };

         task_reference<QWidget> child  = nullptr;
         task_reference<QWidget> parent = nullptr;
         struct {
            int row     = -1; // zero-indexed; negative = unspecified
            int col     = -1; // zero-indexed; negative = unspecified
            int rowspan =  1;
            int colspan =  1;
         } layout;
         error_code error = error_code::none;
         
         virtual bool is_blocking() const noexcept override { return true; } // must be blocking in order to return an error
         virtual bool is_fire_and_forget() const noexcept override { return true; }
      protected:
         virtual void _exec_impl() override;
   };
}