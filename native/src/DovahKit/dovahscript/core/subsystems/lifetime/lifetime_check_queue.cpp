#include "lifetime_check_queue.h"
#include "../lifetime.h"

namespace {
   struct hierarchy_flag {
      enum type : uint8_t {
         referenced_in_lua  = 0x01,
         referenced_in_task = 0x02,
         //
         referenced_across_bridge = 0x40,
         marked_for_delete        = 0x80,
         //
         referenced_anywhere = referenced_in_lua | referenced_in_task | referenced_across_bridge,
      };
   };
   using hierarchy_flags_t = std::underlying_type_t<hierarchy_flag::type>;

   struct found_hierarchy {
      QObject* root = nullptr;
      QVector<found_hierarchy*> bridged_to;
      hierarchy_flags_t flags = 0;
   };

   class hierarchy_abandonment_checker {
      protected:
         bool referenced = false;
         QVector<found_hierarchy*> seen;

         void _recurse(found_hierarchy& target) {
            if (this->seen.contains(&target))
               return;
            this->seen.push_back(&target);
            if (target.flags & hierarchy_flag::referenced_anywhere) {
               this->referenced = true;
               return;
            }
            for (auto* h : target.bridged_to)
               this->_recurse(*h);
         }

      public:
         bool start_from(found_hierarchy& basis) {
            this->_recurse(basis);
            if (this->referenced)
               for (auto* h : this->seen)
                  h->flags |= hierarchy_flag::referenced_across_bridge;
            return this->referenced;
         }
   };
   
   class bridge_crossing_hierarchy_deleter {
      protected:
         void _recurse(found_hierarchy& target) {
            if (target.flags & (hierarchy_flag::marked_for_delete | hierarchy_flag::referenced_anywhere))
               return;
            target.root->deleteLater();
            target.flags |= hierarchy_flag::marked_for_delete;
            for (auto* h : target.bridged_to)
               this->_recurse(*h);
         }

      public:
         void start_from(found_hierarchy& basis) {
            this->_recurse(basis);
         }
   };
}

namespace dovahscript::impl {
   bool lifetime_check_queue::_empty() const noexcept {
      if (this->queues.model_observers.empty())
         return true;
      if (this->queues.objects.empty())
         return true;
      return false;
   }

   void lifetime_check_queue::queue_check(model_observer_t& target) {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->queues.model_observers;
      list.push_back(&target);
   }
   void lifetime_check_queue::queue_check(QObject& target) {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->queues.objects;
      list.push_back(&target);
   }

   void lifetime_check_queue::main_thread_handler(subsystem_passkey) {
      auto guard = std::lock_guard(this->lock);
      if (!this->opportunity_handle.is_active()) {
         if (!this->_empty())
            this->opportunity_handle.request();
      }
      if (!this->opportunity_handle.is_ready())
         return;
      //
      static_assert(false, "TODO: Perform lifetime checks on the queued objects.");

         //
         // To begin:
         //  - Create lists of all non-hierarchy objects confirmed 
         //    to be unreferenced
         //  - Create a QVector<_hierarchy*> seen_hierarchies.
         // 
         // For each hierarchy object:
         //  - Check if task-referenced
         //  - Check if Lua-referenced
         //  - Get hierarchy root
         //     - If status is known, early-out
         //  - Heap-allocate a new (_hierarchy) instance and store it in 
         //    the seen_hierarchies list.
         //     - Walk the root to see if anything in the hierarchy is 
         //       Lua- or task-referenced, and to get a list of all of 
         //       the hierarchy bridges.
         //     - For each hierarchy bridge, get the bridged-to root, 
         //       get-or-create a (_hierarchy) for it, and bridge that 
         //       with the (_hierarchy) we started from.
         // 
         // For each non-hierarchy object:
         //  - Check if task-referenced
         //  - Check if Lua-referenced
         //  - If unreferenced, add the object to the list of non-hierarchy 
         //    objects to be deleted.
         // 
         // Once we're done:
         //  - For each (_hierarchy), use an (_abandonment_checker) to 
         //    see if it's abandoned. If so, mark the hierarchy root and 
         //    all bridged-to roots for deletion.
         //  - For each non-hierarchy object pending deletion, delete it.
         //
   }
   void lifetime_check_queue::on_script_teardown(subsystem_passkey) {
      auto guard = std::lock_guard(this->lock);
      //
      this->queues.model_observers.clear();
      this->queues.objects.clear();
      //
      this->opportunity_handle.release();
   }
}