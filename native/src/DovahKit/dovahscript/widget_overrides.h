#pragma once
#include <QWidget>
#include <QVariant>

namespace dovahscript {
   [[nodiscard]] extern QWidget* get_widget_forced_parent(QWidget* widget);
   extern void set_widget_forced_parent(QWidget* subject, QWidget* parent);

   [[nodiscard]] extern const char* get_widget_metatable_override(QWidget* widget);
   extern void set_widget_metatable_override(QWidget* widget, const char* metatable_key);
}