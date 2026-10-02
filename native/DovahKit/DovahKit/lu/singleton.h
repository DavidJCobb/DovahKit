#pragma once
#include <type_traits>

namespace lu {
   template<typename T, bool AllowFastGet = true>
   class singleton {
      private:
         static T* instance;

      protected:
         constexpr singleton() {
            static_assert(std::is_base_of_v<singleton<T>, T>);
            T::instance = (T*) this;
         }

      private:
         // hack to work around the inability to access T::T if it's protected, without 
         // requiring T to friend `singleton`. will not work if T::T is private.
         struct _unprotect : public T {
            _unprotect() : T() {}
         };

         static T& _get_or_create() {
            static _unprotect instance;
            return instance;
         }

      public:
         static T& get_or_create() requires AllowFastGet {
            return _get_or_create();
         }
         static T& get() {
            if constexpr (AllowFastGet) {
               return *T::instance;
            } else {
               return _get_or_create();
            }
         }

         singleton(singleton&&) = delete;
         singleton(const singleton&) = delete;

         singleton& operator=(singleton&&) = delete;
         singleton& operator=(const singleton&) = delete;
   };

   template<typename T, bool AllowFastGet> T* singleton<T, AllowFastGet>::instance = nullptr;
}