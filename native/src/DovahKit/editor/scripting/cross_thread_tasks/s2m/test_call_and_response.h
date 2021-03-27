#pragma once
#include "../base.h"
#include <QString>

namespace editor_script::tasks::s2m {
   class test_call_and_response : public cross_thread_task {
      //
      // Test message intended for use by DovahKit's developer only. It's a dummy message with 
      // no data, set up to be blocking; the main thread must handle it in order for script 
      // execution to continue.
      //
      public:
         virtual bool is_blocking() const noexcept override { return true; }
         virtual bool is_fire_and_forget() const noexcept override { return false; }
      protected:
         virtual void _exec_impl() override;
   };
}