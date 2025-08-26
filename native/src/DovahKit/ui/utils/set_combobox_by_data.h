#pragma once
#include <type_traits>
#include <QComboBox>

namespace ui {
   template<bool EvenIfAbsent = false>
   void set_combobox_by_data(QComboBox& widget, int v) {
      int i = widget.findData(v);
      if constexpr (!EvenIfAbsent)
         if (i < 0)
            return;
      widget.setCurrentIndex(i);
   }

   template<bool EvenIfAbsent = false, typename Enum> requires std::is_enum_v<Enum>
   void set_combobox_by_data(QComboBox& widget, Enum v) {
      set_combobox_by_data<EvenIfAbsent>(widget, (int)v);
   }
}