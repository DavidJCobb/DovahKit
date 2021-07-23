#include "hierarchy_finder.h"
#include <QAbstractButton>
#include "../../../../helpers/qt/get_model_of.h"
#include "../../../../helpers/qt/traversal.h"
#include "../../../../ui/generic/CanvasWidget.h"
#include "../coordinator.h"
#include "../lifetime.h"
#include "../userdata.h"
#include "../../verify_threading.h"

namespace dovahscript::impl {
   #pragma region results
   void hierarchy_finder::results::append(const results& other) noexcept {
      this->total_widget_count += other.total_widget_count;
      this->root_widgets.append(other.root_widgets);
      this->button_groups.append(other.button_groups);
   }
   void hierarchy_finder::results::clear() noexcept {
      this->total_widget_count = 0;
      this->root_widgets.clear();
      this->button_groups.clear();
   }
   bool hierarchy_finder::results::empty() const noexcept {
      if (!this->root_widgets.empty())
         return false;
      if (!this->button_groups.empty())
         return false;
      return true;
   }
   bool hierarchy_finder::results::has_hierarchy_bridges() const noexcept {
      if (!this->button_groups.empty())
         return true;
      return false;
   }
   //
   bool hierarchy_finder::results::contains(QButtonGroup* o) const noexcept {
      return this->button_groups.contains(o);
   }
   bool hierarchy_finder::results::contains(QWidget* o) const noexcept {
      return this->root_widgets.contains(o);
   }
   //
   QList<QWidget*> hierarchy_finder::results::get_linked_hierarchies(const results& ignore) const noexcept {
      QList<QWidget*> list;
      for (auto* o : this->button_groups) {
         if (ignore.button_groups.contains(o))
            continue;
         for (auto* w : o->buttons()) {
            auto* root = cobb::qt::topmost_container_of(w);
            if (!list.contains(root) && !ignore.contains(root))
               list.push_back(root);
         }
      }
      return list;
   }
   #pragma endregion

   bool hierarchy_finder::_traverse_from_basis(QWidget* basis, results& working) {
      results& res       = this->search_results;
      bool     completed = true;
      //
      auto* root = cobb::qt::topmost_container_of(basis);
      if (this->options.halt_and_clear_upon_non_abandoned) {
         if (auto* dialog = qobject_cast<QDialog*>(root)) // QWidget::parentWindow just traverses upward. no need to do it twice
            if (dialog->isVisible()) // visible windows and their contents should never be considered abandoned
               return false;
      }
      if (res.contains(root))
         return true;
      //
      auto& ud_brain = DovahKitScriptVMUserdataInterface::get();
      //
      cobb::qt::for_each_widget_in_hierarchy(root, [this, &ud_brain, &completed, &working, &res](QWidget* w) {
         completed = false;
         //
         ++working.total_widget_count;
         if (auto* button = qobject_cast<QAbstractButton*>(w)) {
            //
            // Here, we're only going to do two things. We're going to check if this button is 
            // in a QButtonGroup; if so, we'll track the group, and we'll check if the group is 
            // Lua-referenced. QButtonGroups are capable of "bridging" multiple hierarchies of 
            // widgets together, but actually crossing those bridges and examining connected 
            // hierarchies is our caller's job, not ours.
            //
            if (auto* g = button->group()) {
               if (this->options.halt_and_clear_upon_non_abandoned)
                  if (ud_brain.wrapper_exists_for(g))
                     return true;
               if (!working.contains(g) && !res.contains(g))
                  working.button_groups.push_back(g);
            }
         }
         if (this->options.halt_and_clear_upon_non_abandoned) {
            if (ud_brain.wrapper_exists_for(w))
               return true;
            //
            if (auto* model = cobb::qt::get_underlying_model_of(w))
               if (auto* lua_model = qobject_cast<ObservableStandardItemModel*>(model))
                  if (this->referenced_models.contains(lua_model))
                     return true;
            //
            if (auto* canvas = qobject_cast<CanvasWidget*>(w)) {
               //
               // A canvas should not be considered abandoned if any of its layers are Lua-
               // referenced. Layer data is irrelevant, however, in that a Lua-referenced 
               // layer data should not keep an otherwise-abandoned canvas layer (or thus 
               // an otherwise-abandoned entire canvas) alive.
               //
               for (auto* layer : canvas->layers()) {
                  if (ud_brain.wrapper_exists_for(layer))
                     return true;
               }
            }
         }
         //
         completed = true;
         return false;
      });
      if (!completed)
         return false;
      working.root_widgets.push_back(root);
      return true;
   }
   bool hierarchy_finder::_start_from_basis(QObject* basis) {
      this->search_results.total_widget_count = 0; // number of all widgets in all found hierarchies, if the hierarchies are all abandoned
      //
      results  working;
      results& results = this->search_results;
      results.clear();
      if (auto* bw = qobject_cast<QWidget*>(basis)) {
         if (!this->_traverse_from_basis(bw, working))
            return false;
      } else if (auto* bg = qobject_cast<QButtonGroup*>(basis)) {
         working.button_groups = { bg };
      } else {
         assert(false && "unsupported QObject type");
      }
      //
      // We may at this point have found one or more root widgets, as well as all bridges 
      // contained within their hierarchies. We must now process all found bridges.
      //
      results.root_widgets = working.root_widgets;
      working.root_widgets.clear();
      while (working.has_hierarchy_bridges()) {
         //
         // The (results) object now contains all already-processed objects -- root widgets 
         // and hierarchy bridges -- while the (working) object contains only unprocessed 
         // bridges. Let's start, then, by getting all unprocessed root widgets that those 
         // bridges connect to.
         //
         QList<QWidget*> wl = working.get_linked_hierarchies(results);
         //
         // Now that we have those widgets on hand, let's transfer all of the bridges from 
         // (working) into (results) to signify that we've processed them. Then, we'll loop 
         // over all of the found roots and traverse from them, storing what we find into 
         // (working).
         //
         results.append(working);
         working.clear();
         for (auto* w : wl) {
            if (!this->_traverse_from_basis(w, working))
               return false;
         }
         //
         // And of course, the widgets in (working) will be all the roots we just looped 
         // over, so let's transfer them to (results) to signify that we've now processed 
         // them.
         //
         results.root_widgets.append(working.root_widgets);
         working.root_widgets.clear();
      }
      return true;
   }

   void hierarchy_finder::submit_non_abandoned_model(ObservableStandardItemModel* model) noexcept {
      if (!model)
         return;
      auto& list = this->referenced_models;
      if (!list.contains(model))
         list.push_back(model);
   }
   void hierarchy_finder::import_non_abandoned_models(ObservableStandardItemModelObserver* exclude) noexcept {
      auto& ls = dovahscript::core::subsystems::lifetime::get();
      for (auto* p : ls.get_extant_model_observers()) {
         assert(p);
         if (exclude && p == exclude)
            continue;
         this->submit_non_abandoned_model(p->model);
      }
   }

   void hierarchy_finder::gather_from(QObject* basis) noexcept {
      if (this->options.halt_and_clear_upon_non_abandoned) {
         //
         // This option can only be used while on the script thread, as we need to be able to 
         // access Lua-side data in order to check whether an object is referenced (by virtue 
         // of checking whether it has a Lua-side wrapper).
         //
         // You cannot use this option when the Lua state is being torn down. You also should 
         // not need to use this option when the Lua state is being torn down: you don't need 
         // to try and find abandoned widgets/objects, to mark them for deletion, because the 
         // teardown process is going to delete everything anyway.
         //
         DovahKitScriptVMCore::require_script_thread();
      }
      if (!this->_start_from_basis(basis)) {
         this->results.clear();
      }
   }
}