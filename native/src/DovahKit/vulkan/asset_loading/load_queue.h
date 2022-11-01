#pragma once
#include <array>
#include <vector>
#include "../config/asset_loading.h"

namespace vulkanDK::asset_loading {
   template<typename Info> struct load_queue {
      std::array<std::vector<Info>, config::asset_loading_thread_count> batches;
      size_t next_batch_index = 0;

      constexpr void clear() {
         for (auto& batch : batches)
            batch.clear();
         next_batch_index = 0;
      }
      constexpr bool empty() const noexcept {
         for (auto& batch : batches)
            if (!batch.empty())
               return false;
         return true;
      }

      template<typename Functor>
      void for_each_queue_item(Functor&& functor) {
         for (auto& batch : batches)
            for (auto& item : batch)
               (functor)(item);
      }

      void enqueue(Info info) requires (std::is_scalar_v<Info>) {
         auto& idx = this->next_batch_index;
         this->batches[this->next_batch_index].push_back(info);
         idx = (idx + 1) % this->batches.size();
      }
      void enqueue(const Info& info) requires (!std::is_scalar_v<Info>) {
         auto& idx = this->next_batch_index;
         this->batches[this->next_batch_index].push_back(info);
         idx = (idx + 1) % this->batches.size();
      }
      void enqueue(Info&& info) {
         auto& idx = this->next_batch_index;
         this->batches[this->next_batch_index].push_back(std::move(info));
         idx = (idx + 1) % this->batches.size();
      }
   };
}