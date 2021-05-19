#pragma once
#include <functional>
#include <QWidget>

namespace cobb::qt {
   extern void for_each_widget_in_hierarchy(QWidget* root, std::function<bool(QWidget*)> functor); // return (true) to stop early
   extern QWidget* topmost_container_of(QWidget*);

   extern QWidget* nearest_widget_of_type(QWidget* base, const QMetaObject& type);
}
