#pragma once
#include "./RegionCanvasWidget.h"

template<RegionCanvasWidget::CoordinateSpace src_space, RegionCanvasWidget::CoordinateSpace dst_space> requires (src_space != dst_space)
auto RegionCanvasWidget::mapCoords(const Point<src_space>& src_point) const {
   if constexpr (src_space == CoordinateSpace::Screen) {
      auto widget_pos = mapScreenToWidgetPos(src_point);
      if constexpr (dst_space == CoordinateSpace::Widget) {
         return widget_pos;
      } else {
         return mapCoords<CoordinateSpace::Widget, dst_space>(widget_pos);
      }
   }
   if constexpr (src_space == CoordinateSpace::Widget) {
      if constexpr (dst_space == CoordinateSpace::Screen) {
         return mapWidgetToScreenPos(src_point);
      } else {
         auto canvas_pos = mapWidgetToCanvasPos(src_point);
         if constexpr (dst_space == CoordinateSpace::Canvas) {
            return canvas_pos;
         } else {
            return mapCoords<CoordinateSpace::Canvas, dst_space>(canvas_pos);
         }
      }
   }
   if constexpr (src_space == CoordinateSpace::Canvas) {
      if constexpr (dst_space == CoordinateSpace::Screen) {
         return mapWidgetToScreenPos(mapCanvasToWidgetPos(src_point));
      }
      if constexpr (dst_space == CoordinateSpace::Widget) {
         return mapCanvasToWidgetPos(src_point);
      }
      if constexpr (dst_space == CoordinateSpace::Grid) {
         return mapCanvasToGridPos(src_point);
      }
      if constexpr (dst_space == CoordinateSpace::World) {
         return mapCanvasToWorldPos(src_point);
      }
      std::unreachable();
   }
   if constexpr (src_space == CoordinateSpace::Grid) {
      if constexpr (dst_space == CoordinateSpace::World) {
         return mapGridToWorldPos(src_point);
      } else {
         auto canvas_pos = mapGridToCanvasPos(src_point);
         if constexpr (dst_space == CoordinateSpace::Canvas) {
            return canvas_pos;
         } else {
            return mapCoords<CoordinateSpace::Canvas, dst_space>(canvas_pos);
         }
      }
   }
   if constexpr (src_space == CoordinateSpace::World) {
      if constexpr (dst_space == CoordinateSpace::Grid) {
         return mapWorldToGridPos(src_point);
      } else {
         auto canvas_pos = mapWorldToCanvasPos(src_point);
         if constexpr (dst_space == CoordinateSpace::Canvas) {
            return canvas_pos;
         } else {
            return mapCoords<CoordinateSpace::Canvas, dst_space>(canvas_pos);
         }
      }
   }
   std::unreachable();
}
