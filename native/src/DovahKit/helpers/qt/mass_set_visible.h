#pragma once
#include <QWidget>
#include "../concepts.h"

namespace cobb::qt {
   inline void set_visibility_of(bool v, QWidget* widget) {
      widget->setVisible(v);
   }

   template<typename... T> requires (sizeof...(T) > 1 && cobb::is_base_of_all<QWidget, std::remove_pointer_t<T>...>)
   inline void set_visibility_of(bool v, T... widgets) {
      (set_visibility_of(v, widgets) , ...);
   }
}