#pragma once

class  QObject;
struct ObservableStandardItemModelObserver;

namespace dovahscript {
   namespace impl::task_reference {
      extern void inc(QObject*);
      extern void inc(ObservableStandardItemModelObserver*);

      extern void dec(QObject*);
      extern void dec(ObservableStandardItemModelObserver*);
   }

   template<typename T> class task_reference {
      protected:
         T* ptr = nullptr;

         inline void _inc() const noexcept { impl::task_reference::inc(this->ptr); }
         inline void _dec() const noexcept { impl::task_reference::dec(this->ptr); }

      public:
         task_reference() {}
         task_reference(T* p) : ptr(p) { _inc(); }
         task_reference(const task_reference& other) : ptr(other.ptr) { _inc(); }
         task_reference(task_reference&& other) { *this = other; }

         task_reference& operator=(const task_reference& other) {
            _dec();
            this->ptr = other.ptr;
            _inc();
         }
         task_reference& operator=(task_reference&& other) {
            _dec();
            this->ptr = other.ptr;
            other.ptr = nullptr;
         }

         operator bool() { return this->ptr != nullptr; };
         operator T* () const noexcept { return this->ptr; };
         T* operator->() const noexcept { return this->ptr; };
   };
}