#include "hierarchy_crawler.h"
#include <QAbstractButton>
#include <QButtonGroup>
#include "../../../helpers/unordered_map.h"
#include "../../../helpers/qt/get_model_of.h"
#include "../../../helpers/qt/traversal.h"
#include "../../../ui/generic/CanvasWidget.h"
#include "../coordinator.h"
#include "../lifetime.h"
#include "../userdata.h"

namespace {
   using lifetime_passkey = cobb::passkey<dovahscript::core::subsystems::lifetime, dovahscript::impl::hierarchy_crawler>;
}

namespace {
   static QObject* _get_root_of(QObject& basis, QWidget* stop_at) {
      if (auto* group = qobject_cast<QButtonGroup*>(&basis)) {
         //
         // If we were asked to do a lifetime check on a QButtonGroup, then just run the check 
         // on the first button in the group. The hierarchy test will end up hitting the button 
         // group again and finding the other hierarchies that it's bridged to, if any.
         //
         auto list = group->buttons();
         if (list.isEmpty())
            return nullptr;
         return cobb::qt::topmost_container_of(list[0]);
      }
      QObject* root   = &basis;
      QObject* parent = nullptr;
      while (parent = root->parent()) {
         if (parent == stop_at)
            break;
         root = parent;
      }
      return root;
   }

   class hierarchy_abandonment_checker {
      using found_hierarchy = dovahscript::impl::hierarchy_crawler::found_hierarchy;
      using hierarchy_flag  = dovahscript::impl::hierarchy_crawler::hierarchy_flag;
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
         void start_from(found_hierarchy& basis) {
            if (basis.flags & hierarchy_flag::referenced_across_bridge)
               return;
            this->_recurse(basis);
            if (this->referenced)
               for (auto* h : this->seen)
                  h->flags |= hierarchy_flag::referenced_across_bridge;
         }
   };
}

namespace dovahscript::impl {
   hierarchy_crawler::hierarchy_crawler() : lifetime_sys(core::subsystems::lifetime::get()) {
   }
   hierarchy_crawler::~hierarchy_crawler() {
      for (auto* h : this->found) {
         if (h)
            delete h;
      }
      this->found.clear();
      this->severed_bridges.clear();
      //
      this->stop_at = nullptr;
   }
   void hierarchy_crawler::crawl_from(QObject& basis) {
      this->stop_at = core::subsystems::coordinator::get().get_ui_parent();

      auto* root = _get_root_of(basis, this->stop_at);
      if (!root) {
         if (auto* group = qobject_cast<QButtonGroup*>(&basis)) {
            this->severed_bridges.push_back(&basis);
            return;
         }
      }
      for (auto* h : this->found)
         if (h->root == root)
            return;
      auto* base = new found_hierarchy;
      base->root = root;
      this->found.push_back(base);
      //
      if (root->isWidgetType()) {
         hierarchy_flags_t flags = 0;
         QVector<QWidget*> bridged_to;
         cobb::qt::for_each_widget_in_hierarchy((QWidget*)root, [base, &bridged_to, &flags](QWidget* item) {
            if (item->isWidgetType()) {
               ++base->widget_count;
            }

            static_assert(false, "TODO: Check if (item) is referenced; set flags and return true (break) if so");
            if (auto* model = cobb::qt::get_model_of(item)) {
               static_assert(false, "TODO: Check if any model item is referenced; set flags and return true (break) if so");
            } else if (auto* canvas = qobject_cast<CanvasWidget*>(item)) {
               auto entities = canvas->allDescendantLayers();
               static_assert(false, "TODO: Check if any entity is referenced; set flags and return true (break) if so");
            } else if (auto* button = qobject_cast<QAbstractButton*>(item)) {
               //
               // The hierarchy item we've traversed to is a QAbstractButton. See if it has a 
               // QButtonGroup and if so, add that group to our hierarchy's bridge list and 
               // retain its elements for later checking.
               //
               auto* group = button->group();
               if (group) {
                  auto& ubl = base->unowned_bridges;
                  if (!ubl.contains(group)) {
                     ubl.push_back(group);
                     auto list = group->buttons();
                     for (auto* b : list)
                        if (b != button)
                           bridged_to.push_back(b);
                  }
               }
            }
            return false; // continue
         });
         base->flags |= flags;
         //
         // Handle bridged-to hierarchies:
         //
         for (auto* b : bridged_to) {
            auto* bridged_root = _get_root_of(*b, this->stop_at);
            if (bridged_root == root)
               continue;
            bool known = false;
            for (auto* h : this->found) {
               if (h->root == root) {
                  base->bridged_to.push_back(h);
                  h->bridged_to.push_back(base);
                  known = true;
                  break;
               }
            }
            if (known)
               continue;
            //
            // Crawl the new bridged-to hierarchy:
            //
            this->crawl_from(*bridged_root);
         }
      }
      //
   }
   void hierarchy_crawler::crawl_from(ObservableStandardItemModelObserver& observer);
   void hierarchy_crawler::finalize() {
      for (auto* h : this->found) {
         assert(h);
         hierarchy_abandonment_checker checker;
         checker.start_from(*h);
      }
   }
   void hierarchy_crawler::delete_abandoned() {
      for (auto* h : this->found) {
         if (h->flags & hierarchy_flag::referenced_anywhere)
            continue;
         h->flags |= hierarchy_flag::marked_for_delete;
         this->total_widgets_deleted += h->widget_count;
         this->lifetime_sys.destroy_native_object(lifetime_passkey(), *h->root);
      }
      for (auto* b : this->severed_bridges)
         b->deleteLater();
      this->severed_bridges.clear();
   }
}