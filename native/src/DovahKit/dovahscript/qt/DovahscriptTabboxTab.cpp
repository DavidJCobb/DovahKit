#include "DovahscriptTabboxTab.h"

namespace {
   //
   // If true, then we identify a tab's containing tabbox by searching upward through 
   // the hierarchy for any QTabWidget. If false, we check whether the tab's grandparent 
   // widget is a QTabWidget.
   // 
   // Qt's documentation makes no guarantees about *how* a QTabWidget will take ownership 
   // of a tab. Currently, per the source code, every QTabWidget creates a QStackedWidget 
   // child and places tabs within that, but in theory this could change in the future.
   //
   static constexpr const bool use_fuzzy_hierarchy = false;
}

QTabWidget* DovahscriptTabboxTab::containingTabWidget() const noexcept {
   if constexpr (use_fuzzy_hierarchy) {
      auto* parent = this->parentWidget();
      for (auto* parent = this->parentWidget(); parent; parent = parent->parentWidget()) {
         const auto* pm = parent->metaObject();
         if (pm->inherits(&QTabWidget::staticMetaObject))
            return (QTabWidget*) parent;
      }
      return nullptr;
   }
   //
   auto* parent = this->parentWidget();
   if (parent) {
      return qobject_cast<QTabWidget*>(parent->parentWidget());
   }
   return nullptr;
}