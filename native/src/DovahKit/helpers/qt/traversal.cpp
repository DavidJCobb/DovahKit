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

   extern QWidget* nearest_widget_of_type(const QWidget* base, const QMetaObject& type) {
      auto* parent = base;
      do {
         const auto* pm = parent->metaObject();
         if (pm->inherits(&type))
            return const_cast<QWidget*>(parent);
      } while (parent = parent->parentWidget());
      return nullptr;
   }

   extern bool object_is_or_contains(const QObject* haystack, const QObject* needle) {
      do {
         if (needle == haystack)
            return true;
      } while (needle = needle->parent());
      return false;
   }
}
