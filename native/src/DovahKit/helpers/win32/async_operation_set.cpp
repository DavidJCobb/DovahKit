#include "./async_operation_set.h"

namespace cobb::win32 {
   async_operation_set::operation::operation() {
      ZeroMemory(&overlapped, sizeof(overlapped));
      overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
   }
   async_operation_set::operation::~operation() {
      CloseHandle(overlapped.hEvent);
      overlapped.hEvent = NULL;
   }

   size_t async_operation_set::_index_of_operation(size_t id) {
      for (size_t i = 0; i < this->_operations.size(); ++i)
         if (this->_operations[i]->id == id)
            return i;
      return -1;
   }
   async_operation_set::operation* async_operation_set::_operation_by_id(size_t id) {
      for (auto& op : this->_operations)
         if (op->id == id)
            return op.get();
      return nullptr;
   }

   async_operation_set::~async_operation_set() {
      cancel_all_operations();
      this->_operations.clear();
   }

   void async_operation_set::cancel_operation(size_t id) {
      auto* op = _operation_by_id(id);
      if (!op || !op->pending)
         return;

      DWORD bytes_transferred;

      CancelIoEx(op->file, &op->overlapped);
      GetOverlappedResult(op->file, &op->overlapped, &bytes_transferred, true);
      op->pending = false;
   }
   void async_operation_set::cancel_all_operations() {
      //
      // You can cancel asynchronous I/O operations tied to a specific OVERLAPPED structure, 
      // but you still have to wait for the canceled operations to complete afterwards. Let's 
      // cancel them all as soon as possible, and *then* do our waits.
      // 
      // A useful code sample for cancelling a single operation can be found here:
      // https://learn.microsoft.com/en-us/windows/win32/fileio/canceling-pending-i-o-operations
      //
      for (auto& op : this->_operations) {
         if (op->pending)
            CancelIoEx(op->file, &op->overlapped);
      }

      DWORD bytes_transferred;
      for (auto& op : this->_operations) {
         if (op->pending) {
            GetOverlappedResult(op->file, &op->overlapped, &bytes_transferred, true);
            op->pending = false;
         }
      }
   }

   void async_operation_set::destroy_operation(size_t id) {
      auto i = _index_of_operation(id);
      if (i == -1)
         return;
      auto op = std::move(this->_operations[i]);
      if (op->pending) {
         DWORD bytes_transferred;

         CancelIoEx(op->file, &op->overlapped);
         GetOverlappedResult(op->file, &op->overlapped, &bytes_transferred, true);
         op->pending = false;
      }
      this->_operations.erase(this->_operations.begin() + i);
   }
   void async_operation_set::destroy_all_operations() {
      cancel_all_operations();
      this->_operations.clear();
      this->_all_waitables = this->_extra_waitables;
   }
}