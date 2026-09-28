#include "./make_readonly_textbox_more_obviously_so.h"
#include <array>
#include <QPalette>

namespace ui {
   extern void make_readonly_textbox_more_obviously_so(QLineEdit& widget) {
      auto palette = widget.palette();
      for (auto group : std::array{
         QPalette::ColorGroup::Normal,
         QPalette::ColorGroup::Inactive
      }) {
         auto fgcolor = palette.color(group, QPalette::ColorRole::Text);
         fgcolor.setAlpha(200);
         palette.setColor(group, QPalette::ColorRole::Text, fgcolor);
      }
      widget.setPalette(palette);
   }
}