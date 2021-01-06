#pragma once
#include <functional>
#include "base.h"

namespace editor_script::tasks {
   class generic_task : public cross_thread_task {
      //
      // A cross-thread task that accepts a simple lambda.
      //
      using functor_t = std::function<void()>;
      public:
         const functor_t functor;
         const bool      blocking;
         //
         generic_task(functor_t f, bool b = false) : functor(f), blocking(b) {}
         //
         virtual bool is_blocking() const noexcept override { return this->blocking; }
      protected:
         virtual void _exec_impl() override {
            (this->functor)();
         }
   };
}