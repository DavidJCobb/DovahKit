#pragma once
#include "../base.h"
#include <QDialog>

namespace editor_script::tasks::s2m {
   class ui_dialog_operation : public cross_thread_task {
      public:
         enum class operation_type {
            none,
            get_size_grip,
            set_size_grip,
            get_title,
            set_title,
            get_visibility,
            set_visibility,
         };
         
         QDialog*       target    = nullptr;
         operation_type operation = operation_type::none;
         struct {
            bool    boolean = false;
            QString text;
         } params;
         struct {
            bool    boolean = false;
            QString text;
         } results;
         
         virtual bool is_blocking() const noexcept override { return true; }
      protected:
         virtual void _exec_impl() override;
   };
}