#pragma once
#include "./async_operation_set.h"
#include <cassert>
#include <stdexcept>

namespace cobb::win32 {
   template<typename Functor> requires std::is_invocable_r_v<bool, Functor, HANDLE&, OVERLAPPED&>
   void async_operation_set::queue_operation(size_t id, Functor&& functor) {
      std::unique_ptr<operation> created;

      auto* subject = this->_operation_by_id(id);
      if (subject) {
         assert(!subject->pending);
      } else {
         created = std::make_unique<operation>();
         subject = created.get();
         created->id = id;
      }

      if (created) {
         size_t size = this->_operations.size();

         this->_operations.push_back(std::move(created));
         this->_all_waitables.push_back(NULL);

         auto& list = this->_all_waitables;
         list[size] = subject->overlapped.hEvent;
         for (size_t i = 0; i < this->_extra_waitables.size(); ++i)
            _all_waitables[size + i] = _extra_waitables[i];
      }

      bool success = functor(subject->file, subject->overlapped);
      subject->pending = success;
   }

   template<typename... Types> requires (std::is_same_v<Types, HANDLE> && ...)
   void async_operation_set::add_extra_waitables(Types... args) {
      (_extra_waitables.push_back(args), ...);

      size_t size = this->_operations.size();
      _all_waitables.resize(size + _extra_waitables.size());
      for (size_t i = 0; i < _all_waitables.size(); ++i)
         _all_waitables[size + i] = _extra_waitables[i];
   }
   
   template<typename OpFunctor, typename ExtraWaitableFunctor> requires (
      std::is_invocable_v<ExtraWaitableFunctor, HANDLE>
   )
   void async_operation_set::wait(OpFunctor&& of, ExtraWaitableFunctor&& ewf) {
      auto wait_result = WaitForMultipleObjects(_all_waitables.size(), _all_waitables.data(), false, INFINITE);
      if (wait_result == WAIT_FAILED) {
         throw std::exception("");
      }

      size_t i = wait_result - WAIT_OBJECT_0;
      if (i < MAXIMUM_WAIT_OBJECTS) {
         if (i >= this->_operations.size()) {
            ewf(this->_all_waitables[i]);
            return;
         }

      }
   }
}
