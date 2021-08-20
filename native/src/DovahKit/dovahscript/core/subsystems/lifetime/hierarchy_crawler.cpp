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

#include "../../../qt/DovahscriptDialog.h"
#include "../../../qt/DovahscriptStandardItemModel.h"

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
   hierarchy_crawler::hierarchy_crawler() : lifetime_sys(core::subsystems::lifetime::get()), userdata_sys(core::subsystems::userdata::get()) {
   }
   hierarchy_crawler::~hierarchy_crawler() {
      for (auto* h : this->found)
         if (h)
            delete h;
      this->found.clear();
      //
      this->severed_bridges.objects.clear();
      this->severed_bridges.model_observers.clear();
      //
      this->stop_at = nullptr;
   }
   void hierarchy_crawler::initialize() {
      this->task_ref_checker.set_active_state(true);
   }
   void hierarchy_crawler::crawl_from(QObject& basis) {
      this->stop_at = core::subsystems::coordinator::get().get_ui_parent();

      auto* root = _get_root_of(basis, this->stop_at);
      if (!root) {
         if (auto* group = qobject_cast<QButtonGroup*>(&basis)) {
            auto& list = this->severed_bridges.objects;
            if (!list.contains(&basis))
               list.push_back(&basis);
            return;
         }
      }
      for (auto* h : this->found)
         if (h->root == root)
            //
            // If the basis object is part of a hierarchy that we've already handled, then 
            // we need not proceed any further.
            //
            return;
      auto* base = new found_hierarchy;
      base->root = root;
      this->found.push_back(base);
      //
      // Next, we need to do the following things:
      // 
      //  - We need to check whether the hierarchy contains any widgets or connected objects 
      //    that are Lua-referenced or task-referenced.
      // 
      //  - If the hierarchy contains no referenced objects and has no referenced bridges, 
      //    then we need to count how many widgets exist in the hierarchy. This is so that 
      //    if we end up deleting the hierarchy, we can update the script engine's counter 
      //    for how many scripted widgets exist.
      // 
      //  - If the hierarchy contains no referenced objects and has no referenced bridges, 
      //    then we need to traverse each of its bridges and see if the bridged-to hierarchies 
      //    contain any referenced objects or bridges (and so on).
      // 
      // If a hierarchy contains any widgets or connected objects that are Lua-referenced or 
      // task-referenced, then we don't need to traverse the hierarchy any further: we don't 
      // need the widget count because we're not deleting anything; and we don't need to cross 
      // any bridges, because if this hierarchy *is* bridged to any hierarchies that are also 
      // pending lifetime checks, then we'll end up crossing those bridges from the opposite 
      // direction if we need to cross them at all.
      // 
      // That's all quite convenient for us; it means we can stop traversing the hierarchy as 
      // soon as we see anything that we know is in use.
      //
      if (auto* dialog = qobject_cast<DovahscriptDialog*>(root)) {
         //
         // Visible dialogs should never be considered abandoned.
         //
         if (dialog->lastVisibleState()) {
            base->flags |= hierarchy_flag::is_visible_dialog;
            return; // break
         }
      }
      if (this->task_ref_checker.is_task_referenced(basis)) {
         base->flags |= hierarchy_flag::referenced_in_task;
         return; // break
      }
      if (this->userdata_sys.wrapper_exists_for(&basis)) {
         base->flags |= hierarchy_flag::referenced_in_lua;
         return; // break
      }
      //
      if (root->isWidgetType()) {
         hierarchy_flags_t flags = 0;
         QVector<QWidget*> bridged_to;
         cobb::qt::for_each_widget_in_hierarchy((QWidget*)root, [this, base, &bridged_to, &flags](QWidget* item) {
            if (item->isWidgetType()) {
               ++base->widget_count;
            }
            //
            // Let's start by checking whether the object is referenced:
            //
            if (this->task_ref_checker.is_task_referenced(*item)) {
               flags |= hierarchy_flag::referenced_in_task;
               return true; // break
            }
            if (this->userdata_sys.wrapper_exists_for(item)) {
               flags |= hierarchy_flag::referenced_in_lua;
               return true; // break
            }
            //
            // Now, let's handle non-widget descendant objects, along with any hierarchy bridges:
            //
            if (auto* model = qobject_cast<DovahscriptStandardItemModel*>(cobb::qt::get_underlying_model_of(item))) {
               //
               // This widget has a scriptable model. Check if any item in the model is referenced.
               // 
               // We should have been given a list of models known to be referenced, in advance of 
               // being asked to crawl anything, so we can just check if the model is in that list.
               //
               if (this->models_known_to_be_referenced.contains(model)) {
                  flags |= hierarchy_flag::referenced_general;
                  return true; // break
               }
               //
               // Let's plan for the future a bit: as of this writing, models can't be shared by 
               // multiple widgets (we have no code to manage lifetimes on the models themselves), 
               // but let's account for that case anyway.
               //
               auto list = model->associatedWidgets();
               for (auto* widget : list)
                  if (widget != item)
                     bridged_to.push_back(widget);
            } else if (auto* canvas = qobject_cast<CanvasWidget*>(item)) {
               //
               // Canvas widget entities can be Lua-referenced or task-referenced. We need to 
               // check all child and descendant entities of this canvas, although we can stop 
               // early should we find any that are referenced.
               //
               auto entities = canvas->allDescendantLayers();
               for (auto* entity : entities) {
                  if (this->task_ref_checker.is_task_referenced(*entity)) {
                     flags |= hierarchy_flag::referenced_in_task;
                     return true; // break
                  }
                  if (this->userdata_sys.wrapper_exists_for(entity)) {
                     flags |= hierarchy_flag::referenced_in_lua;
                     return true; // break
                  }
               }
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
         if (flags & hierarchy_flag::referenced_anywhere) {
            base->widget_count = 0;
         }
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
   void hierarchy_crawler::crawl_from(ObservableStandardItemModelObserver& observer) {
      auto* model = qobject_cast<DovahscriptStandardItemModel*>(observer.model);
      if (model) {
         //
         // We currently handle model observers (and consequently the models themselves) as hierarchy 
         // bridges. It's the easiest and most convenient way to handle them, and it allows us to be 
         // a bit more prepared for the future -- for the possibility that we might allow Lua scripts 
         // to work directly with models and share them between widgets someday. (We currently do not 
         // allow this because we have no design or implementation for managing the lifetimes of the 
         // models themselves; currently, models are owned and used only by a single widget.)
         // 
         // Accordingly, we can treat these things the same way we do QButtonGroups: get a list of 
         // the widgets which use the model in question, pick any one, and crawl from it.
         //
         auto list = model->associatedWidgets();
         if (!list.isEmpty()) {
            assert(list[0]);
            this->crawl_from(*list[0]);
         }
      } else {
         auto& list = this->severed_bridges.model_observers;
         if (!list.contains(&observer))
            list.push_back(&observer);
      }
   }
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
         this->lifetime_sys.destroy_hierarchy_object(lifetime_passkey(), *h->root);
         //
         for (auto* b : h->unowned_bridges)
            if (!b->parent())
               this->lifetime_sys.destroy_hierarchy_object(lifetime_passkey(), *b);
      }
      //
      for (auto* b : this->severed_bridges.objects) {
         this->lifetime_sys.destroy_hierarchy_object(lifetime_passkey(), *b);
      }
      this->severed_bridges.objects.clear();
      //
      for (auto* b : this->severed_bridges.model_observers) {
         this->lifetime_sys.destroy_hierarchy_object(lifetime_passkey(), *b);
      }
      this->severed_bridges.model_observers.clear();
   }
}