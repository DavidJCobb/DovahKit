#pragma once
#include <memory>
#include <type_traits>
#include <vector>
#include "../windows.h"

// untested

namespace cobb::win32 {
   class async_operation_set {
      protected:
         struct operation {
            operation();
            ~operation();

            size_t     id      = 0;
            HANDLE     file    = NULL;
            OVERLAPPED overlapped;
            bool       pending = false;
         };

      protected:
         std::vector<HANDLE> _all_waitables;
         std::vector<std::unique_ptr<operation>> _operations;

         std::vector<HANDLE> _extra_waitables;

         size_t _index_of_operation(size_t id);
         operation* _operation_by_id(size_t id);

      public:
         ~async_operation_set();

         template<typename Functor> requires std::is_invocable_r_v<bool, Functor, HANDLE&, OVERLAPPED&>
         void queue_operation(size_t id, Functor&&);

         template<typename... Types> requires (std::is_same_v<Types, HANDLE> && ...)
         void add_extra_waitables(Types... args);

         void cancel_operation(size_t id);
         void cancel_all_operations();

         void destroy_operation(size_t id);
         void destroy_all_operations();

         template<typename OpFunctor, typename ExtraWaitableFunctor> requires (
            std::is_invocable_v<ExtraWaitableFunctor, HANDLE>
         )
         void wait(OpFunctor&& of, ExtraWaitableFunctor&& ewf);
   };
}

#include "./async_operation_set.inl"