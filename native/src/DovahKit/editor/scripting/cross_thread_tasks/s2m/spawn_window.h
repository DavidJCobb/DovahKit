#pragma once
#include "../base.h"
#include <QDialog>

namespace editor_script::tasks::s2m {
   class spawn_window : public cross_thread_task {
      public:
         QDialog* result = nullptr;
         bool     error  = false;
         //
         virtual bool is_blocking() const noexcept override { return true; }
         virtual bool is_fire_and_forget() const noexcept override { return false; }
      protected:
         virtual void _exec_impl() override;
   };
}