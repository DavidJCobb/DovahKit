#pragma once
#include <functional>
#include <QWidget>

namespace cobb::qt {
   extern void for_each_widget_in_hierarchy(QWidget* root, std::function<bool(QWidget*)> functor); // return (true) to stop early
   extern QWidget* topmost_container_of(QWidget*);

   // returns the nearest self-or-ancestor widget of the given type
   extern QWidget* nearest_widget_of_type(const QWidget* base, const QMetaObject& type);

   extern bool object_is_or_contains(const QObject* haystack, const QObject* needle);
}
