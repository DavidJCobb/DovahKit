#pragma once
#include "../base.h"
#include <functional>

namespace editor_script::tasks::s2m {
   class lambda : public cross_thread_task {
      protected:
         const bool _blocking;
      public:
         lambda(bool b = true) : _blocking(b) {}

         std::function<void()> handler;
         
         virtual bool is_blocking() const noexcept override { return this->_blocking; }
         virtual bool is_fire_and_forget() const noexcept override { return !this->_blocking; }
      protected:
         virtual void _exec_impl() override {
            if (this->handler)
               (this->handler)();
         }
   };

   class ui_read_lambda : public ui_read_task {
      public:
         std::function<void()> handler;
      protected:
         virtual void _exec_impl() override {
            if (this->handler)
               (this->handler)();
         }
   };
}