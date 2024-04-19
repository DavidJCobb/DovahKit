#include "QLinedTreeView.h"
#include <QPainter>

void QLinedTreeView::_BranchStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption* opt, QPainter* painter, const QWidget* widget) const {
   if (pe != PE_IndicatorBranch) {
      QProxyStyle::drawPrimitive(pe, opt, painter, widget);
      return;
   }
   constexpr int button_size = 9;
   //
   int x_start = opt->rect.x();
   int x_mid   = x_start + opt->rect.width() / 2;
   int x_end   = x_start + opt->rect.width();
   int y_start = opt->rect.y();
   int y_mid   = y_start + opt->rect.height() / 2;
   int y_end   = y_start + opt->rect.height();
   //
   auto hints   = painter->renderHints();
   QPen old_pen = painter->pen();
   auto color   = old_pen.color();
   QPen dotted  = painter->pen();
   dotted.setColor(opt->palette.shadow().color());
   dotted.setStyle(Qt::DotLine);
   dotted.setWidth(0);
   dotted.setDashPattern({ 1.0F, 1.0F });
   painter->setPen(dotted);
   painter->setRenderHints({});
   #pragma region Draw branches
      if (opt->state & (State_Sibling | State_Item)) {
         int y_start_even = y_start - (y_start % 2);
         int y_mid_even   = y_mid   - (y_mid   % 2);
         int y_end_even   = y_end   + ((y_end   % 2) ? 0 : 1);
         //
         // Vertical line:
         //
         if (opt->state & State_Sibling)
            painter->drawLine(x_mid, y_start_even, x_mid, y_end_even);
         else
            painter->drawLine(x_mid, y_start_even, x_mid, y_mid_even);
         //
         // Horizontal line:
         //
         int y_horiz = (opt->state & State_Children) ? y_mid : y_mid_even + 2;
         if (opt->state & State_Item)
            painter->drawLine(x_mid, y_horiz, x_end - 1, y_horiz);
      }
   #pragma endregion
   painter->setPen(old_pen);
   painter->setRenderHints(hints);
   //
   if (opt->state & State_Children) { // code for the expand/collapse buttons.
      int button_l = x_mid - button_size / 2;
      int button_r = x_mid + button_size / 2;
      int button_t = y_mid - button_size / 2;
      int button_b = y_mid + button_size / 2;
      //
      // Below is code for the expand/collapse buttons' outlines.
      //
      painter->fillRect(button_l, button_t, button_size - 1, button_size - 1, opt->palette.base().color());
      painter->setPen(opt->palette.shadow().color());
      painter->drawRect(button_l, button_t, button_size - 1, button_size - 1);
      //
      // Next, the plus/minus labels:
      //
      painter->setPen(color);
      painter->drawLine(button_l + 2, button_t + 4, button_l + 6, button_t + 4);
      if (!(opt->state & State_Open))
         painter->drawLine(button_l + 4, button_t + 2, button_l + 4, button_t + 6);
   }
}