#pragma once
#include "./multiheap.h"
#include <algorithm> // std::find

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#define TEMPLATE_PARAMS template<typename T, size_t CountPerBlock>
#define CLASS_NAME multiheap<T, CountPerBlock>

namespace cobb {
   #pragma region block_info
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::block_info::on_allocate(element_index_type index) noexcept {
         this->presence.set(index);
         --this->remaining;
         this->start_from = index + 1;
      }
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::block_info::on_free(element_index_type index) noexcept {
         this->presence.reset(index);
         ++this->remaining;
         if (this->start_from > index)
            this->start_from = index;
      }
   #pragma endregion
   #pragma region block_t
      TEMPLATE_PARAMS
      void* CLASS_NAME::block_t::try_allocate() {
         if (!this->has_free_slots())
            return nullptr;
         auto i = this->info.presence.find_first_clear_from(this->info.start_from);
         if (i < 0)
            return nullptr;
         std::intptr_t start = (std::intptr_t) &this->buffer;
         std::intptr_t addr  = start + (stride * i);
         this->info.on_allocate(i);
         return (void*)addr;
      }

      TEMPLATE_PARAMS
      bool CLASS_NAME::block_t::try_free(void* mem) {
         std::intptr_t m_addr  = (std::intptr_t)mem;
         std::intptr_t b_start = (std::intptr_t)&this->buffer;
         std::intptr_t b_end   = b_start + sizeof(this->buffer);
         if (m_addr >= b_start && m_addr < b_end) {
            m_addr -= b_start;
            element_index_type index = m_addr / stride;
            assert(m_addr % stride == 0            && "Cannot free; element is not aligned.");
            assert(this->info.presence.test(index) && "You're freeing something that was already free!");
            this->info.on_free(index);
            return true;
         }
         return false;
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::block_t* CLASS_NAME::block_t::get_end() noexcept {
         auto* block = this;
         while (block->info.next)
            block = block->info.next;
         return block;
      }

      TEMPLATE_PARAMS
      void CLASS_NAME::block_t::prune() noexcept { // removes all empty blocks after this one
         auto* n = this->info.next;
         for (auto* block = n; block; block = n) {
            n = block->info.next;
            if (!block->has_any_slots_used())
               delete block;
         }
      }

      #pragma region Helpers for State::force_destroy_all
         TEMPLATE_PARAMS
         void CLASS_NAME::block_t::destroy_all_elements() noexcept {
            auto& presence = this->info.presence;
            for (element_index_type i = 0; i < count_per_block; i++) {
               if (presence.test(i)) {
                  std::intptr_t start = (std::intptr_t)&this->buffer;
                  std::intptr_t addr = start + (stride * i);
                  //
                  auto element = (mapped_type*)addr;
                  element->~mapped_type();
               }
            }
            presence.clear();
            this->info.remaining = count_per_block;
            //
            memset(this->buffer, 0, sizeof(this->buffer));
         }

         TEMPLATE_PARAMS
         void CLASS_NAME::block_t::destroy_and_prune_list() noexcept {
            auto last = this;
            while (last->info.next)
               last = last->info.next;
            auto prev = last->info.prev;
            do {
               last->destroy_all_elements();
               if (last != this) // never delete the first block in a list
                  delete last;
               //
               last = prev;
               if (prev)
                  prev = prev->info.prev;
            } while (last);
         }
      #pragma endregion
   #pragma endregion
   #pragma region subheap
      TEMPLATE_PARAMS
      void* CLASS_NAME::subheap::allocate() {
         std::lock_guard guard(this->alloc_free_lock);
         assert(this->first && "The subheap has no block?");
         block_t* block = this->first;
         block_t* last  = block;
         void*    out   = block->try_allocate();
         while (!out) {
            block = block->info.next;
            if (block)
               last = block;
            else
               break;
            out = block->try_allocate();
         }
         if (out)
            return out;
         if (!block) {
            assert(last && "A cobb::multiheap::subheap wants to create a new block, but there's no preceding block to append it to.");
            auto next = new block_t;
            last->info.next = next;
            next->info.prev = last;
            #if _DEBUG
               next->info.original_thread_id = this->thread_id;
            #endif
            out = next->try_allocate();
            assert(out && "Allocation failed!");
         }
         return out;
      }
      
      TEMPLATE_PARAMS
      bool CLASS_NAME::subheap::try_free(void* mem) {
         std::lock_guard guard(this->alloc_free_lock);
         auto* block = this->first;
         do {
            if (block->try_free(mem)) {
               //
               // Ah, good: we do indeed own the block that contains the to-be-freed 
               // memory, and that block has now freed said memory. If the block is 
               // now empty and isn't our head block, then let's free it, too, before 
               // we signal success to our caller.
               //
               if (block != this->first && !block->has_any_slots_used()) {
                  delete block;
               }
               return true;
            }
         } while (block = block->info.next);
         //
         // We don't own whatever block contains the to-be-freed memory.
         //
         return false;
      }
   #pragma endregion
   #pragma region State
      TEMPLATE_PARAMS
      void CLASS_NAME::State::register_new_subheap(subheap& sub) noexcept {
         std::lock_guard guard(this->lock);
         //
         // First, add the subheap to our list of subheaps.
         //
         {
            auto& list = this->subheaps;
            auto  it   = std::find(list.begin(), list.end(), nullptr);
            if (it != list.end())
               *it = &sub;
            else
               list.push_back(&sub);
         }
         //
         // Now, see if we have any unowned blocks we can give to the subheap.
         //
         if (this->unowned_blocks) {
            assert(sub.first != nullptr);
            assert(this->unowned_blocks != sub.first);
            //
            // Subheaps preemptively spawn a block when they're initialized. If we're 
            // giving this newly-initialized subheap our unowned blocks, then we should 
            // ditch the block it spawned for itself. (That block will be empty, because 
            // subheaps must necessarily initialize and register themselves before they 
            // can carry out an allocation.)
            //
            delete sub.first;
            //
            sub.first = this->unowned_blocks;
            this->unowned_blocks = nullptr;
         }
      }

      TEMPLATE_PARAMS
      void CLASS_NAME::State::kill_subheap(subheap& sub) noexcept {
         assert(sub.first && "This multiheap is being asked to kill a subheap that's already dead.");
         //
         // If we're transferring data between these two places, then we have to lock BOTH of 
         // them at the start of the transfer and unlock BOTH of them after the transfer is 
         // complete.
         // 
         // When other code (i.e. `State::free`) checks these two locations, it locks them 
         // individually and only while it's checking them. If we do the same here, then that 
         // creates a window where...
         // 
         //    1. We remove the block from the dying subheap.
         //    2. The other thread beats us to the unowned blocks lock.
         //    3. The other thread loops over all unowned blocks and doesn't see the block, as 
         //       the block hasn't yet made it to that list.
         //    4. The other thread fails to do what it's trying to do, because the specific 
         //       block it was looking for was the one we were acting upon.
         //
         std::lock_guard guard(this->lock);
         //
         // Prune the subheap's block list, and then take ownership of any blocks that remain.
         //
         auto* block = sub.first;
         block->prune();
         sub.first = nullptr;
         if (!block->has_any_slots_used()) {
            //
            // We've pruned the subheap's tail blocks, but its head block is empty. Delete the 
            // head block and take its next sibling (if one remains).
            //
            auto* n = block->info.next;
            delete block;
            block = n;
         }
         if (block) {
            if (!this->unowned_blocks) {
               this->unowned_blocks = block;
            } else {
               auto* append_to = this->unowned_blocks->get_end();
               append_to->info.next = block;
               block->info.prev = append_to;
            }
         }
         //
         // Null out the subheap's entry in our list of subheaps, and then delete the subheap. 
         // (We null the entry rather than removing it so that we can recycle the entry later 
         // should a new subheap be created. Avoids having to shuffle the list around so much.)
         //
         #if _DEBUG
            bool found = false;
         #endif
         for (size_t i = 0; i < this->subheaps.size(); ++i) {
            auto*& item = this->subheaps[i];
            if (item == &sub) {
               item = nullptr;
               #if _DEBUG
                  found = true;
               #endif
               break;
            }
         }
         #if _DEBUG
            assert(found && "This multiheap killed a subheap it seemingly did not own!");
         #endif
         delete &sub;
      }

      TEMPLATE_PARAMS
      void CLASS_NAME::State::free(void* mem) noexcept {
         std::lock_guard guard(this->lock);

         for (auto* s : this->subheaps) {
            if (!s) {
               //
               // This empty list entry was once occupied by a subheap, but that subheap 
               // has since been killed (`State::kill_subheap`).
               //
               continue;
            }
            if (s->try_free(mem)) {
               return;
            }
         }
         for (auto* block = this->unowned_blocks; block; block = block->info.next) {
            if (block->try_free(mem)) {
               if (!block->has_any_slots_used()) {
                  //
                  // Remove the block if we just deleted its last element.
                  //
                  if (block == this->unowned_blocks) {
                     this->unowned_blocks = block->info.next;
                  }
                  delete block;
               }
               return;
            }
         }
         assert(false && "This heap cannot free memory that it isn't responsible for.");
      }

      TEMPLATE_PARAMS
      void CLASS_NAME::State::force_destroy_all() noexcept {
         std::lock_guard guard(this->lock);

         for (auto* s : this->subheaps) {
            std::lock_guard guard(s->alloc_free_lock);
            if (s->first)
               s->first->destroy_and_prune_list();
         }
         if (auto*& ptr = this->unowned_blocks) {
            ptr->destroy_and_prune_list();
            delete ptr;
            ptr = nullptr;
         }
      }
   #pragma endregion
   #pragma region subheap_handle
      TEMPLATE_PARAMS
      CLASS_NAME::subheap_handle::subheap_handle() {
         //
         // We don't initialize the subheap in the `subheap_handle` constructor. This is 
         // intentional. Every thread has a subheap handle, but we only want to create a 
         // subheap for a thread if that thread is actually going to allocate something. 
         // Ergo we delay initialization until the thread actually retrieves the subheap.
         //
      }

      TEMPLATE_PARAMS
      CLASS_NAME::subheap_handle::~subheap_handle() {
         if (!this->data)
            return;
         multiheap::_get_state().kill_subheap(*this->data);
         this->data = nullptr;
      }

      TEMPLATE_PARAMS
      void CLASS_NAME::subheap_handle::initialize() {
         if (this->data)
            return;
         this->data = new subheap;
         #if _DEBUG
            this->data->thread_id = std::this_thread::get_id();
         #endif
         multiheap::_get_state().register_new_subheap(*this->data);
      }
   #pragma endregion
}

#undef CLASS_NAME
#undef TEMPLATE_PARAMS
#pragma pop_macro("CLASS_NAME")
#pragma pop_macro("TEMPLATE_PARAMS")