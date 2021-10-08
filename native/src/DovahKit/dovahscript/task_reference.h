#pragma once
#include <QObject>

struct ObservableStandardItemModelObserver;

namespace dovahscript {
   class DovahscriptResource;
   class DovahscriptResourceHandle;

   namespace impl::task_reference {
      extern void inc(DovahscriptResource*);
      extern void inc(QObject*);
      extern void inc(ObservableStandardItemModelObserver*);

      extern void dec(DovahscriptResource*);
      extern void dec(QObject*);
      extern void dec(ObservableStandardItemModelObserver*);

      extern DovahscriptResource* strip(const DovahscriptResourceHandle&);
   }

   template<typename T> class task_reference {
      protected:
         T* ptr = nullptr;

         static constexpr bool is_resource  = std::is_same_v<DovahscriptResource, T>;
         static constexpr bool is_qt_object = ([]() { if constexpr (is_resource) { return false; } else { return std::is_base_of_v<QObject, T>; } })(); // WORKAROUND: can't pass a forward-declared class to is_base_of

         inline void _inc() const noexcept {
            if constexpr (is_resource) {
               impl::task_reference::inc((DovahscriptResource*)this->ptr);
            } else if constexpr (is_qt_object) {
               impl::task_reference::inc((QObject*)this->ptr);
            } else {
               impl::task_reference::inc(this->ptr);
            }
         }
         inline void _dec() const noexcept {
            if constexpr (is_resource) {
               impl::task_reference::dec((DovahscriptResource*)this->ptr);
            } else if constexpr (is_qt_object) {
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
         task_reference(const DovahscriptResourceHandle& other) : ptr(impl::task_reference::strip(other)) { _inc(); }
         ~task_reference() {
            _dec();
            this->ptr = nullptr;
         }

         task_reference& operator=(T* other) {
            _dec();
            this->ptr = other;
            _inc();
            return *this;
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