#pragma once
#include <QObject>

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

         inline void _inc() const noexcept {
            if constexpr (std::is_base_of_v<QObject, T>) { // false C2665 errors without this; MSVC doesn't understand overloads so we have to do this garbage
               impl::task_reference::inc((QObject*)this->ptr);
            } else {
               impl::task_reference::inc(this->ptr);
            }
         }
         inline void _dec() const noexcept {
            if constexpr (std::is_base_of_v<QObject, T>) { // false C2665 errors without this; MSVC doesn't understand overloads so we have to do this garbage
               impl::task_reference::dec((QObject*)this->ptr);
            } else {
               impl::task_reference::dec(this->ptr);
            }
         }

      public:
         task_reference() {}
         task_reference(T* p) : ptr(p) { _inc(); }
         task_reference(const task_reference& other) : ptr(other.ptr) { _inc(); }
         task_reference(task_reference&& other) : ptr(other.ptr) { other.ptr = nullptr; }
         ~task_reference() {
            _dec();
            this->ptr = nullptr;
         }

         task_reference& operator=(const task_reference& other) {
            _dec();
            this->ptr = other.ptr;
            _inc();
            return *this;
         }
         task_reference& operator=(task_reference&& other) {
            _dec();
            this->ptr = other.ptr;
            other.ptr = nullptr;
            return *this;
         }

         operator bool() { return this->ptr != nullptr; };
         operator T* () const noexcept { return this->ptr; };
         T* operator->() const noexcept { return this->ptr; };
   };
}