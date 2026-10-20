#pragma once
#include "./threaded_slab_heap.h"
#include <algorithm> // std::find

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#define TEMPLATE_PARAMS template<typename T, size_t CountPerSlab>
#define CLASS_NAME threaded_slab_heap<T, CountPerSlab>

namespace lu::memory {
   #pragma region slab_info
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::slab_info::on_allocate(element_index_type index) noexcept {
         this->presence.set(index);
         --this->remaining;
         this->start_from = index + 1;
      }
      TEMPLATE_PARAMS
      constexpr void CLASS_NAME::slab_info::on_free(element_index_type index) noexcept {
         this->presence.reset(index);
         ++this->remaining;
         if (this->start_from > index)
            this->start_from = index;
      }
   #pragma endregion
   #pragma region slab_type
      TEMPLATE_PARAMS
      void* CLASS_NAME::slab_type::try_allocate() {
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
      bool CLASS_NAME::slab_type::try_free(void* mem) {
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
      constexpr CLASS_NAME::slab_type* CLASS_NAME::slab_type::get_end() noexcept {
         auto* slab = this;
         while (slab->info.next)
            slab = slab->info.next;
         return slab;
      }

      TEMPLATE_PARAMS
      void CLASS_NAME::slab_type::prune() noexcept { // removes all empty blocks after this one
         auto* n = this->info.next;
         for (auto* slab = n; slab; slab = n) {
            n = slab->info.next;
            if (!slab->has_any_slots_used())
               delete slab;
         }
      }

      #pragma region Helpers for State::force_destroy_all
         TEMPLATE_PARAMS
         void CLASS_NAME::slab_type::destroy_all_elements() noexcept {
            auto& presence = this->info.presence;
            for (element_index_type i = 0; i < count_per_slab; i++) {
               if (presence.test(i)) {
                  std::intptr_t start = (std::intptr_t)&this->buffer;
                  std::intptr_t addr = start + (stride * i);
                  //
                  auto element = (mapped_type*)addr;
                  element->~mapped_type();
               }
            }
            presence.clear();
            this->info.remaining = count_per_slab;
            //
            memset(this->buffer, 0, sizeof(this->buffer));
         }

         TEMPLATE_PARAMS
         void CLASS_NAME::slab_type::destroy_and_prune_list() noexcept {
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
         assert(this->first && "The subheap has no slab?");
         slab_type* slab = this->first;
         slab_type* last = slab;
         void*      out  = slab->try_allocate();
         while (!out) {
            slab = slab->info.next;
            if (slab)
               last = slab;
            else
               break;
            out = slab->try_allocate();
         }
         if (out)
            return out;
         if (!slab) {
            assert(last && "A lu::memory::threaded_slab_heap::subheap wants to create a new slab, but there's no preceding slab to append it to.");
            auto next = new slab_type;
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
         auto* slab = this->first;
         do {
            if (slab->try_free(mem)) {
               //
               // Ah, good: we do indeed own the slab that contains the to-be-freed 
               // memory, and that slab has now freed said memory. If the slab is 
               // now empty and isn't our head slab, then let's free it, too, before 
               // we signal success to our caller.
               //
               if (slab != this->first && !slab->has_any_slots_used()) {
                  delete slab;
               }
               return true;
            }
         } while (slab = slab->info.next);
         //
         // We don't own whatever slab contains the to-be-freed memory.
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
         // Now, see if we have any unowned slabs we can give to the subheap.
         //
         if (this->unowned_slabs) {
            assert(sub.first != nullptr);
            assert(this->unowned_slabs != sub.first);
            //
            // Subheaps preemptively spawn a slab when they're initialized. If we're 
            // giving this newly-initialized subheap our unowned slabs, then we should 
            // ditch the slab it spawned for itself. (That slab will be empty, because 
            // subheaps must necessarily initialize and register themselves before they 
            // can carry out an allocation.)
            //
            delete sub.first;
            //
            sub.first = this->unowned_slabs;
            this->unowned_slabs = nullptr;
         }
      }

      TEMPLATE_PARAMS
      void CLASS_NAME::State::kill_subheap(subheap& sub) noexcept {
         assert(sub.first && "This threaded_slab_heap is being asked to kill a subheap that's already dead.");
         //
         // If we're transferring data between these two places, then we have to lock BOTH of 
         // them at the start of the transfer and unlock BOTH of them after the transfer is 
         // complete.
         // 
         // When other code (i.e. `State::free`) checks these two locations, it locks them 
         // individually and only while it's checking them. If we do the same here, then that 
         // creates a window where...
         // 
         //    1. We remove the slab from the dying subheap.
         //    2. The other thread beats us to the unowned slabs lock.
         //    3. The other thread loops over all unowned slabs and doesn't see the slab, as 
         //       the slab hasn't yet made it to that list.
         //    4. The other thread fails to do what it's trying to do, because the specific 
         //       slab it was looking for was the one we were acting upon.
         //
         std::lock_guard guard(this->lock);
         //
         // Prune the subheap's slab list, and then take ownership of any slabs that remain.
         //
         auto* slab = sub.first;
         slab->prune();
         sub.first = nullptr;
         if (!slab->has_any_slots_used()) {
            //
            // We've pruned the subheap's tail slabs, but its head slab is empty. Delete the 
            // head slab and take its next sibling (if one remains).
            //
            auto* n = slab->info.next;
            delete slab;
            slab = n;
         }
         if (slab) {
            if (!this->unowned_slabs) {
               this->unowned_slabs = slab;
            } else {
               auto* append_to = this->unowned_slabs->get_end();
               append_to->info.next = slab;
               slab->info.prev = append_to;
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
            assert(found && "This threaded_slab_heap killed a subheap it seemingly did not own!");
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
         for (auto* slab = this->unowned_slabs; slab; slab = slab->info.next) {
            if (slab->try_free(mem)) {
               if (!slab->has_any_slots_used()) {
                  //
                  // Remove the block if we just deleted its last element.
                  //
                  if (slab == this->unowned_slabs) {
                     this->unowned_slabs = slab->info.next;
                  }
                  delete slab;
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
         if (auto*& ptr = this->unowned_slabs) {
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
         threaded_slab_heap::_get_state().kill_subheap(*this->data);
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
         threaded_slab_heap::_get_state().register_new_subheap(*this->data);
      }
   #pragma endregion
}

#undef CLASS_NAME
#undef TEMPLATE_PARAMS
#pragma pop_macro("CLASS_NAME")
#pragma pop_macro("TEMPLATE_PARAMS")