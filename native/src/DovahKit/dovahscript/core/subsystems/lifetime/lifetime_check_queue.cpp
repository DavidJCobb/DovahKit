#include "lifetime_check_queue.h"
#include <QVariant>
#include "helpers/unordered_map.h"
#include "helpers/qt/traversal.h"
#include "ui/generic/CanvasWidget.h"
#include "../lifetime.h"
#include "../userdata.h"
#include "./hierarchy_crawler.h"
#include "./task_reference_state_multi_checker.h"

#include "dovahscript/qt/DovahscriptCanvasWidgetLayerData.h"
#include "dovahscript/qt/DovahscriptStandardItemModel.h"

namespace {
   using lifetime_passkey = cobb::passkey<dovahscript::core::subsystems::lifetime, dovahscript::impl::lifetime_check_queue>;
}

namespace dovahscript::impl {
   bool lifetime_check_queue::_empty() const noexcept {
      if (!this->queues.hierarchy_objects.empty())
         return false;
      if (!this->queues.model_observers.empty())
         return false;
      if (!this->queues.non_hierarchy_objects.empty())
         return false;
      return true;
   }

   void lifetime_check_queue::queue_check(model_observer_t& target) {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->queues.model_observers;
      if (!list.contains(&target))
         list.push_back(&target);
   }
   void lifetime_check_queue::queue_check(QObject& target) {
      auto  guard = std::lock_guard(this->lock);
      if (target.isWidgetType() || qobject_cast<CanvasWidgetEntity*>(&target) || qobject_cast<QButtonGroup*>(&target)) {
         auto& list = this->queues.hierarchy_objects;
         if (!list.contains(&target))
            list.push_back(&target);
         return;
      } else {
         auto& list = this->queues.non_hierarchy_objects;
         if (!list.contains(&target))
            list.push_back(&target);
      }
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
      auto  task_ref_access = task_reference_state_multi_checker();
      //
      {  // Hierarchy objects.
         hierarchy_crawler crawler;
         crawler.initialize();
         //
         // Before we start crawling, we need to be able to handle model observers. 
         // Simplest way to do that is to just go over all extant model observers 
         // here, and track the models that we know to be referenced (i.e. any 
         // that have observers which aren't pending a lifetime check).
         //
         lifetime_s.for_each_known_model_observer([this, &crawler, &userdata_s, &task_ref_access](model_observer_t* observer) {
            assert(observer);
            auto& list  = crawler.models_known_to_be_referenced;
            auto* model = qobject_cast<DovahscriptStandardItemModel*>(observer->model);
            if (!model)
               return;
            if (list.contains(model))
               return;
            if (!userdata_s.wrapper_exists_for(observer) && !task_ref_access.is_task_referenced(*observer))
               return;
            list.push_back(model);
         });
         //
         // Now let's crawl some hierarchies!
         //
         for (auto* object : this->queues.hierarchy_objects) {
            assert(object);
            crawler.crawl_from(*object);
         }
         for (auto* object : this->queues.model_observers) {
            assert(object);
            crawler.crawl_from(*object);
         }
         crawler.finalize();
         crawler.delete_abandoned();
         lifetime_s.decrease_extant_widget_count(lifetime_passkey(), crawler.total_widgets_deleted);
         //
         this->queues.hierarchy_objects.clear();
         this->queues.model_observers.clear();
      }
      //
      // Non-hierarchy objects are much simpler to handle:
      //
      for (auto* obj : this->queues.non_hierarchy_objects) {
         assert(obj);
         //
         // First, let's check whether the object is referenced within the script engine, 
         // whether by a Lua value or by a task:
         //
         if (task_ref_access.is_task_referenced(*obj))
            continue;
         if (userdata_s.wrapper_exists_for(obj))
            continue;
         //
         // Next, let's do type-specific checks to see if the object is in use by some 
         // object within the UI. If it's in use by the UI, let's avoid deleting it, 
         // even if it may not be reachable for scripts anymore.
         // 
         // (Incidentally, this check probably hinges on us handling hierarchical 
         // objects first.)
         //
         if (auto* cwld = qobject_cast<DovahscriptCanvasWidgetLayerData*>(obj)) {
            if (cwld->is_lua_referenced)
               continue;
            if (!cwld->users().isEmpty())
               continue;
         }
         //
         // We've confirmed that the object is not referenced within the script engine, 
         // and that it's not in use anywhere. Let's get rid of it:
         //
         if (!obj->property("deleted").isValid()) { // ensure we only delete an object once even if it was marked for multiple checks
            obj->setProperty("deleted", true);
            lifetime_s.destroy_non_hierarchy_object(lifetime_passkey(), *obj);
         }
      }
      this->queues.non_hierarchy_objects.clear();
      //
      // And now we're done!
      //
      this->opportunity_handle.release();
      assert(this->_empty()); // Let's make sure we didn't forget to process and clear any lists.
   }
   void lifetime_check_queue::on_script_teardown(subsystem_passkey) {
      auto guard = std::lock_guard(this->lock);
      //
      this->queues.hierarchy_objects.clear();
      this->queues.model_observers.clear();
      this->queues.non_hierarchy_objects.clear();
      assert(this->_empty()); // Let's make sure we didn't forget to process and clear any lists.
      //
      this->opportunity_handle.release();
   }
   bool lifetime_check_queue::empty() const noexcept {
      auto guard = std::lock_guard(this->lock);
      return this->_empty() && !this->opportunity_handle.is_active();
   }
}