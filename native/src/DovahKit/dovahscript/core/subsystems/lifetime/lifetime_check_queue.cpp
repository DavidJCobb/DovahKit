#include "lifetime_check_queue.h"
#include <QVariant>
#include "../../../helpers/unordered_map.h"
#include "../../../helpers/qt/traversal.h"
#include "../lifetime.h"
#include "../userdata.h"
#include "hierarchy_crawler.h"

namespace {
   using lifetime_passkey = cobb::passkey<dovahscript::core::subsystems::lifetime, dovahscript::impl::lifetime_check_queue>;
}

namespace dovahscript::impl {
   bool lifetime_check_queue::_empty() const noexcept {
      if (this->queues.hierarchy_objects.empty())
         return true;
      if (this->queues.model_observers.empty())
         return true;
      if (this->queues.non_hierarchy_objects.empty())
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
      if (target.isWidgetType() || qobject_cast<CanvasWidgetEntity*>(&target)) {
         this->queues.hierarchy_objects.push_back(&target);
         return;
      }
      this->queues.non_hierarchy_objects.push_back(&target);
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
      auto& lifetime_s = core::subsystems::lifetime::get();
      auto& userdata_s = core::subsystems::userdata::get();
      //
      static_assert(false, "TODO: Perform lifetime checks on the queued objects.");
      {  // Hierarchy objects.
         hierarchy_crawler crawler;
         static_assert(false, "What about model observers?");
         for (auto* object : this->queues.hierarchy_objects) {
            assert(object);
            crawler.crawl_from(*object);
         }
         crawler.finalize();
         crawler.delete_abandoned();
         lifetime_s.extant_widget_count -= crawler.total_widgets_deleted;
         static_assert(false, "Use the crawler's total widgets deleted count.");
      }
      for (auto* obj : this->queues.non_hierarchy_objects) {
         static_assert(false, "check if the object is task- or Lua-referenced; `continue` if so");
         if (!obj->property("deleted").isValid()) { // ensure we only delete an object once even if it was marked for multiple checks
            obj->setProperty("deleted", true);
            lifetime_s.destroy_non_hierarchy_object(lifetime_passkey(), *obj);
         }
      }
   }
   void lifetime_check_queue::on_script_teardown(subsystem_passkey) {
      auto guard = std::lock_guard(this->lock);
      //
      this->queues.hierarchy_objects.clear();
      this->queues.model_observers.clear();
      this->queues.non_hierarchy_objects.clear();
      //
      this->opportunity_handle.release();
   }
}