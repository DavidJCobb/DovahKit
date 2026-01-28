#include "./paint_arrow.h"
#include "helpers/math/rotation/unit_conversion.h"
#include "helpers/math/cosine.h"
#include "helpers/math/sine.h"

namespace ui::utils {
   void paint_arrow(
      QPainter&         painter,
      const QPointF&    src_point,
      const QPointF&    dst_point,
      const float       dst_margin,
      const paint::ArrowStyle& style
   ) {
      painter.setPen(style.stem.pen);
      painter.drawLine(src_point, dst_point);

      painter.setPen(style.head.pen);
      painter.setBrush(style.head.brush);

      QPointF direction = (dst_point - src_point);
      if (direction.x() || direction.y()) {
         float mag = sqrt(direction.x()*direction.x() + direction.y()*direction.y());
         direction /= mag;
      } else {
         direction = { 0, 1 };
      }

      const auto arrow_angle = style.head.angle * cobb::degrees_to_radians_mult;
      const auto arrow_cos   = cobb::cosine(arrow_angle);
      const auto arrow_sin   = cobb::sine(arrow_angle);
      //
      const auto x_cos = direction.x() * arrow_cos;
      const auto x_sin = direction.x() * arrow_sin;
      const auto y_cos = direction.y() * arrow_cos;
      const auto y_sin = direction.y() * arrow_sin;

      // The use case for the "destination margin" is for when you're drawing a line to, say, 
      // a circle. If all you know is the centerpoint of that circle, then pass the radius as 
      // the margin, and we'll ensure that the arrowhead touches the edge of the circle rather 
      // than being partially covered by it.
      const auto arrow_head_pos = dst_point - (direction * dst_margin);

      const QPointF points[] = {
         arrow_head_pos,
         { arrow_head_pos.x() - (x_cos - y_sin)*style.head.length, arrow_head_pos.y() - (y_cos + x_sin)*style.head.length },
         { arrow_head_pos.x() - (x_cos + y_sin)*style.head.length, arrow_head_pos.y() - (y_cos - x_sin)*style.head.length },
      };
      painter.drawPolygon(points, std::extent<decltype(points)>::value);
   }
}