#include "traversal.h"

namespace cobb::qt {
   extern void for_each_widget_in_hierarchy(QWidget* root, std::function<bool(QWidget*)> functor) {
      auto widgets = root->findChildren<QWidget*>();
      if ((functor)(root))
         return;
      for (auto* w : widgets)
         if ((functor)(w))
            return;
   }
   extern QWidget* topmost_container_of(QWidget* subject) {
      while (auto* parent = subject->parentWidget())
         subject = parent;
      return subject;
   }
}
