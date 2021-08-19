#include "add_child_ui_widget.h"
#include <QBoxLayout>
#include <QGridLayout>
#include "../../../helpers/qt/traversal.h"
#include "../../core/subsystems/lifetime.h"
#include "../../widget_overrides.h"

namespace dovahscript::tasks::s2m {
   void add_child_ui_widget::_exec_impl() {
      assert(this->child);
      assert(this->parent);
      if (get_widget_forced_parent(this->child)) {
         this->error = error_code::child_has_a_forced_parent;
         return;
      }
      if (auto* window = qobject_cast<QDialog*>(this->child)) {
         this->error = error_code::child_is_a_window;
         return;
      }
      auto* prior_parent = this->child->parent();
      if (cobb::qt::object_is_or_contains(this->child, this->parent)) {
         this->error = error_code::would_be_cyclical;
         return;
      }
      auto* layout = this->parent->layout();
      if (!layout) {
         this->child->setParent(this->parent);
      } else if (auto* grid = qobject_cast<QGridLayout*>(layout)) {
         if (this->layout.row >= 0 && this->layout.col >= 0) {
            grid->addWidget(this->child, this->layout.row, this->layout.col, this->layout.rowspan, this->layout.colspan);
         } else {
            grid->addWidget(this->child);
         }
      } else if (auto* box = qobject_cast<QBoxLayout*>(layout)) {
         if (this->layout.row >= 0) {
            box->insertWidget(this->layout.row, this->child);
         } else {
            box->addWidget(this->child);
         }
      } else {
         this->error = error_code::unknown_layout_type;
         return;
      }
      core::subsystems::lifetime::get().on_hierarchy_item_parent_changed(this->child, prior_parent);
   }
}