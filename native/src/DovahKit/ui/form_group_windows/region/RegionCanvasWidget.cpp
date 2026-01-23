#include "./RegionCanvasWidget.h"
#include <QApplication>
#include <QPainter>
#include <QWheelEvent>
#include "helpers/bound_mem_fn.h"
#include "dovah/core_constants/exterior_cell_side_length.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/for_each_inbound_use_with_flag.h"
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/form_stubs/helpers/get_worldspace_persistent_cell.h"
#include "dovah/forms/components/extra_data/types/c/cell_region_list.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/Region.h"
#include "dovah/use_info/entry_flags/region.h"
#include "dovah/utils/get_region_worldspace.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

namespace {
   static constexpr const char* context_menu_item_region_area_index_property = "area-index";
}

RegionCanvasWidget::RegionCanvasWidget(QWidget* parent) : QWidget(parent) {
   {
      auto* scrollbar = this->subwidgets.scrollbar_x = new QScrollBar(this);
      scrollbar->setOrientation(Qt::Orientation::Horizontal);
      QObject::connect(scrollbar, &QScrollBar::valueChanged, this, qOverload<>(&QWidget::repaint));
   }
   {
      auto* scrollbar = this->subwidgets.scrollbar_y = new QScrollBar(this);
      scrollbar->setOrientation(Qt::Orientation::Vertical);
      QObject::connect(scrollbar, &QScrollBar::valueChanged, this, qOverload<>(&QWidget::repaint));
   }
   {
      auto* status_bar = this->subwidgets.status_bar = new QStatusBar(this);
      {
         auto* panel = this->subwidgets.status_panels.grid = new QLabel(this);
         panel->setMinimumWidth(150);
         panel->setMaximumWidth(150);
         status_bar->addWidget(panel);
      }
      {
         auto* panel = this->subwidgets.status_panels.world = new QLabel(this);
         panel->setMinimumWidth(150);
         panel->setMaximumWidth(150);
         status_bar->addWidget(panel);
      }
   }
   this->setMouseTracking(true); // for status bar coordinate updates

   #pragma region Default styles
      this->styles.grid.background = QColor(0, 0, 0);
      {
         QPen border_pen;
         border_pen.setCosmetic(true);
         border_pen.setWidth(3);

         QPen vertex_outline_pen;
         vertex_outline_pen.setCosmetic(true);
         vertex_outline_pen.setWidth(1);

         this->styles.region_area = decltype(Style::region_area){
            .normal = {
               .fill = QColor(0, 0, 0, 0),
               .line = border_pen,
            },
            .highlight = {
               .fill = QColor(255, 255, 255, 64),
               .line = border_pen,
            },
            .being_drawn = {
               .area = {
                  .fill = QColor(0, 0, 0, 0),
                  .line = border_pen,
               },
               .first_vertex  = {
                  .fill   = QColor(255, 0, 0),
                  .line   = vertex_outline_pen,
                  .radius = 3,
               },
               .latest_vertex = {
                  .fill   = QColor(255, 192, 180),
                  .line   = vertex_outline_pen,
                  .radius = 3,
               },
            },
         };
         this->styles.region_area.normal.line.setColor(QColor(128, 0, 0));
         this->styles.region_area.highlight.line.setColor(QColor(255, 0, 0));
         this->styles.region_area.being_drawn.area.line.setColor(QColor(255, 0, 0));

         this->styles.region_area.being_drawn.first_vertex.line.setColor(QColor(255, 0, 0));
         this->styles.region_area.being_drawn.latest_vertex.line.setColor(QColor(255, 0, 0));
      }
   #pragma endregion

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &RegionCanvasWidget::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &RegionCanvasWidget::_on_data_acquired);
   QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   if (editor.has_data()) {
      this->_on_data_acquired();
   }

   //
   // Context menu
   //
   {
      auto& menu     = this->context.menu;
      auto& actions  = this->context.actions;
      #pragma region Drawing-region-area actions
         {
            auto* action = actions.clear_last_point = new QAction(tr("Clear last point"), this);
            QObject::connect(action, &QAction::triggered, this, cobb__bound_this_fn(_context_clear_last_point));
         }
         {
            auto* action = actions.close_polygon = new QAction(tr("Close polygon"), this);
            QObject::connect(action, &QAction::triggered, this, cobb__bound_this_fn(_context_finish_drawing_area));
         }
         {
            auto* action = actions.cancel_drawing = new QAction(tr("Cancel drawing area"), this);
            QObject::connect(action, &QAction::triggered, this, cobb__bound_this_fn(_context_cancel_drawing_area));
         }
      #pragma endregion
      QObject::connect(&menu, &QMenu::aboutToShow, this, cobb__bound_this_fn(_build_context_menu));
      QObject::connect(&menu, &QMenu::hovered, this, [this](QAction* action) {
         size_t& stored_index  = this->state.region_area_to_highlight;
         size_t  current_index = index_of_none;
         if (action) {
            auto prop = action->property(context_menu_item_region_area_index_property);
            if (prop.isValid())
               current_index = prop.toInt();
         }
         if (stored_index != current_index) {
            stored_index = current_index;
            this->repaint();
         }
      });
      QObject::connect(&menu, &QMenu::aboutToHide, this, [this]() {
         auto& index = this->state.region_area_to_highlight;
         if (index != index_of_none) {
            index = index_of_none;
            this->repaint();
         }
      });
   }
}
RegionCanvasWidget::~RegionCanvasWidget() {
}

#pragma region Current world/region focus
   void RegionCanvasWidget::setRegion(const ui::types::regions::region& region) {
      if (!region.stub)
         return;
      if (region.bounds.worldspace && region.bounds.worldspace != this->state.worldspace)
         return;
      this->_cancel_moving_area();
      this->state.current_region.stub  = region.stub;
      this->state.current_region.areas = region.bounds.areas;
      this->state.area_being_drawn = {};

      this->context.menu.hide();
      this->context.menu.clear();

      this->repaint(); // to render region areas
   }
   void RegionCanvasWidget::setNoRegion() {
      this->_cancel_moving_area();
      this->state.current_region = {};

      this->context.menu.hide();
      this->context.menu.clear();

      this->repaint();
   }
   void RegionCanvasWidget::setWorldspace(dovah::form_stub* stub) {
   if (!stub) {
      if (!this->state.worldspace)
         return;
      this->state.cells.clear();
      this->state.known_regions.clear();
      this->state.worldspace = nullptr;
      this->state.current_region = {};
      this->state.area_being_drawn = {};
      this->state.grid_extents = {};

      this->_cancel_moving_area();
      this->context.menu.hide();
      this->context.menu.clear();

      this->update();
      return;
   }
   if (stub->form_type != dovah::form_type::worldspace)
      return;
   if (stub == this->state.worldspace)
      return;

   this->state.worldspace = stub;
   this->state.current_region = {};
   this->state.area_being_drawn = {};

   this->_gather_regions();
   this->_gather_cells_from(*stub);
   this->_recalc_all_cell_colors();

   this->_cancel_moving_area();
   this->context.menu.hide();
   this->context.menu.clear();

   this->update();
}
#pragma endregion
#pragma region Editing helpers
   void RegionCanvasWidget::forceRegionColor(dovah::form_stub& region, QColor c) {
      auto it = this->state.known_regions.find(&region);
      if (it != this->state.known_regions.end()) {
         it->second.color = c;
         this->_recalc_cell_colors_affected_by(region);
         this->repaint();
      }
   }
   void RegionCanvasWidget::forceRegionDataPresence(dovah::form_stub& region, const RegionDataPresence& presence) {
      auto it = this->state.known_regions.find(&region);
      if (it != this->state.known_regions.end()) {
         it->second.presence = presence;
         this->_recalc_cell_colors_affected_by(region);
         this->repaint();
      }
   }
#pragma endregion
#pragma region Widget state
   QPointF RegionCanvasWidget::scrollCenter() const noexcept {
      auto* sbx = this->subwidgets.scrollbar_x;
      auto* sby = this->subwidgets.scrollbar_y;
      return QPointF{
         sbx->value() + (float)sbx->pageStep() / 2,
         sby->value() + (float)sby->pageStep() / 2,
      };
   }
   QPoint RegionCanvasWidget::scrollPosition() const noexcept {
      return QPoint{
         this->subwidgets.scrollbar_x->value(),
         this->subwidgets.scrollbar_y->value(),
      };
   }
#pragma endregion
#pragma region Worldspace contents accessors
   std::vector<size_t> RegionCanvasWidget::areasUnderPoint(const QPoint& local_pos) const noexcept {
      auto world_pos = mapCoords<CoordinateSpace::Widget, CoordinateSpace::World>(local_pos);
   
      std::vector<size_t> out;

      auto& areas = this->state.current_region.areas;
      for (size_t i = 0; i < areas.size(); ++i) {
         QPolygonF poly;
         for (auto& point : areas[i].points)
            poly << QPointF(point.x, point.y);

         if (poly.containsPoint(world_pos, Qt::FillRule::OddEvenFill)) {
            out.push_back(i);
         }
      }

      return out;
   }
   std::vector<std::pair<dovah::form_stub*, QString>> RegionCanvasWidget::regionsUnderPoint(const QPoint& local_pos) const noexcept {
      std::vector<std::pair<dovah::form_stub*, QString>> out;

      auto grid_pos = mapCoords<CoordinateSpace::Widget, CoordinateSpace::Grid>(local_pos);
      for (auto& cell : this->state.cells) {
         if (cell.grid.x != grid_pos.x())
            continue;
         if (cell.grid.y != grid_pos.y())
            continue;

         for (auto* region : cell.regions) {
            out.emplace_back(
               region,
               QString::fromStdString(region->editorID)
            );
         }
         std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
            return a.second.localeAwareCompare(b.second) < 0;
         });
         break;
      }

      return out;
   }
#pragma endregion

void RegionCanvasWidget::setColorRequirements(const RegionColorRequirements& req) {
   this->state.color_requirements = req;
   this->_recalc_all_cell_colors();
   this->repaint();
}

#pragma region Coordinate space conversions
   namespace {
      constexpr const QPoint  world_axis_invert     = QPoint(1, -1);
      constexpr const QPointF grid_to_corner_offset = QPointF(0.5, 0.5);

      constexpr const float world_units_per_cell           = dovah::core_constants::exterior_cell_side_length;
      constexpr const float world_units_per_unscaled_pixel = world_units_per_cell / RegionCanvasWidget::default_cell_size;

      static QPointF _apply_world_axis_invert(const QPointF& f) {
         QPointF g = f;
         g.setX(f.x() * world_axis_invert.x());
         g.setY(f.y() * world_axis_invert.y());
         return g;
      }
   }
   QPoint RegionCanvasWidget::mapWorldToGridPos(const QPointF& src) const {
      QPointF dst = src;
      dst += (grid_to_corner_offset * world_units_per_cell);
      dst /= world_units_per_cell;
      return dst.toPoint();
   }
   QPointF RegionCanvasWidget::mapGridToWorldPos(const QPoint& src) const {
      QPointF dst = src;
      dst *= dovah::core_constants::exterior_cell_side_length;
      dst -= (grid_to_corner_offset * world_units_per_cell);
      return dst;
   }
   QPoint RegionCanvasWidget::mapWorldToCanvasPos(const QPointF& src) const {
      QPointF dst = src;
      dst  = _apply_world_axis_invert(dst);
      dst -= QPoint(world_units_per_cell, 0); // not sure why we need this lmao
      dst /= (world_units_per_unscaled_pixel / this->state.zoom);
      return dst.toPoint();
   }
   QPointF RegionCanvasWidget::mapCanvasToWorldPos(const QPoint& src) const {
      QPointF dst = src;
      dst *= (world_units_per_unscaled_pixel / this->state.zoom);
      dst += QPoint(world_units_per_cell, 0); // not sure why we need this lmao
      dst  = _apply_world_axis_invert(dst);
      return dst;
   }

   QPoint RegionCanvasWidget::mapGridToCanvasPos(const QPoint& src) const {
      const float cell_size = default_cell_size * this->state.zoom;

      QPointF dst = src;
      dst  = _apply_world_axis_invert(dst);
      dst -= grid_to_corner_offset;
      dst *= cell_size;
      return dst.toPoint();
   }
   QPoint RegionCanvasWidget::mapCanvasToGridPos(const QPoint& src) const {
      const float cell_size = default_cell_size * this->state.zoom;

      QPointF dst = src;
      dst /= cell_size;
      dst += grid_to_corner_offset;
      dst  = _apply_world_axis_invert(dst);
      return dst.toPoint();
   }

   QPoint RegionCanvasWidget::mapCanvasToWidgetPos(const QPoint& src) const {
      return src - this->scrollPosition();
   }
   QPoint RegionCanvasWidget::mapWidgetToCanvasPos(const QPoint& src) const {
      return src + this->scrollPosition();
   }

   QPoint RegionCanvasWidget::mapWidgetToScreenPos(const QPoint& p) const {
      return this->mapToGlobal(p);
   }
   QPoint RegionCanvasWidget::mapScreenToWidgetPos(const QPoint& p) const {
      return this->mapFromGlobal(p);
   }
#pragma endregion

#pragma region Widget API
   /*virtual*/ QSize RegionCanvasWidget::minimumSizeHint() const /*override*/ {
      this->subwidgets.scrollbar_x->ensurePolished();
      this->subwidgets.scrollbar_y->ensurePolished();
      this->subwidgets.status_bar->ensurePolished();
      int w = this->subwidgets.scrollbar_y->width();
      int h = this->subwidgets.scrollbar_x->height();
      return QSize{ w * 2, h * 2 + this->subwidgets.status_bar->height()};
   }
#pragma endregion
#pragma region Events
   /*virtual*/ void RegionCanvasWidget::contextMenuEvent(QContextMenuEvent* event) /*override*/ {
      this->_stop_panning();
      this->_clear_status_panels();

      this->context.menu.popup(event->globalPos());
   }
   /*virtual*/ void RegionCanvasWidget::leaveEvent(QEvent* event) /*override*/ {
      this->_clear_status_panels();
   }
   /*virtual*/ void RegionCanvasWidget::mouseMoveEvent(QMouseEvent* event) /*override*/ {
      QPoint delta;
      if (this->state.panning || this->isMovingArea()) {
         auto pos = event->globalPos();
         delta = pos - this->state.last_mouse_pos;
         this->state.last_mouse_pos = pos;
      }
      if (this->state.panning) {
         auto* sbx = this->subwidgets.scrollbar_x;
         auto* sby = this->subwidgets.scrollbar_y;
         sbx->setValue(sbx->value() + delta.x());
         sby->setValue(sby->value() + delta.y());
      }
      if (this->isMovingArea()) {
         auto world_delta = QPointF(delta) * (world_units_per_unscaled_pixel / this->state.zoom);
         world_delta = _apply_world_axis_invert(world_delta);

         this->state.moving_area_delta += world_delta;
         this->repaint();
      }
      auto local_pos = event->localPos().toPoint();
      if (!this->state.panning && this->isDrawingArea()) {
         if (_can_close_polygon_at(mapCoords<CoordinateSpace::Widget, CoordinateSpace::Canvas>(local_pos))) {
            this->setCursor(Qt::CursorShape::PointingHandCursor);
         } else {
            this->setCursor(Qt::CursorShape::CrossCursor);
         }
      }
      this->_update_status_panels(mapCoords<CoordinateSpace::Widget, CoordinateSpace::Canvas>(local_pos));
   }
   /*virtual*/ void RegionCanvasWidget::mousePressEvent(QMouseEvent* event) /*override*/ {
      auto local_pos    = event->localPos().toPoint();
      bool is_on_canvas = this->state.viewport.contains(local_pos);
      if (is_on_canvas) {
         switch (event->button()) {
            case Qt::MouseButton::MiddleButton:
               this->_start_panning(event->globalPos());
               break;
            case Qt::MouseButton::LeftButton:
               if (this->isMovingArea()) {
                  this->_finish_moving_area();
                  event->accept();
               } else {
                  this->_draw_point_at(mapCoords<CoordinateSpace::Widget, CoordinateSpace::Canvas>(local_pos));
               }
               break;
            case Qt::MouseButton::RightButton:
               if (this->isMovingArea()) {
                  this->_cancel_moving_area();
                  event->accept();
               }
               break;
         }
      }
   }
   /*virtual*/ void RegionCanvasWidget::mouseReleaseEvent(QMouseEvent* event) /*override*/ {
      if (event->button() == Qt::MouseButton::MiddleButton) {
         this->_stop_panning();
      }
   }
   /*virtual*/ void RegionCanvasWidget::paintEvent(QPaintEvent* event) /*override*/ {
      QPainter painter(this);

      painter.setClipRect(this->state.viewport);
      painter.setBrush(this->styles.grid.background);
      painter.drawRect(this->state.viewport);

      if (!this->state.cells.empty()) {
         const float  cell_size = default_cell_size * this->state.zoom;
         const QPoint cell_px_offset = QPoint(
            (cell_border_width / 2) - (cell_size / 2),
            (cell_border_width / 2) - (cell_size / 2)
         );
         const QSize cell_px_size = QSize(
            cell_size - cell_border_width,
            cell_size - cell_border_width
         );
         for (auto& cell : this->state.cells) {
            QPoint cell_centerpoint = mapCoords<CoordinateSpace::Grid, CoordinateSpace::Widget>({ cell.grid.x, cell.grid.y });
            QRect  cell_rect;
            cell_rect.setTopLeft(cell_centerpoint + cell_px_offset);
            cell_rect.setSize(cell_px_size);

            painter.setBrush(cell.color);
            painter.drawRect(cell_rect);
         }
      }
      //
      // Draw the current region's areas.
      //
      {
         auto& areas = this->state.current_region.areas;
         if (!areas.empty()) {
            auto _draw_poly = [this, &painter](size_t area_index, const RegionArea& area) {
               QPolygon polygon;
               for (const auto& point : area.points) {
                  QPointF world_point = QPointF(point.x, point.y);
                  if (area_index == this->state.moving_region_area) {
                     world_point += this->state.moving_area_delta;
                  }
                  polygon << mapCoords<CoordinateSpace::World, CoordinateSpace::Widget>(world_point);
               }
               painter.drawPolygon(polygon);
            };
            //
            // Draw non-highlighted areas first; then the highlighted area separately. 
            // This minimizes the number of brush/pen switches done while drawing the 
            // overall region.
            //
            painter.setBrush(this->styles.region_area.normal.fill);
            painter.setPen(this->styles.region_area.normal.line);
            for (size_t i = 0; i < areas.size(); ++i) {
               if (i == this->state.region_area_to_highlight)
                  continue;
               _draw_poly(i, areas[i]);
            }
            if (this->state.region_area_to_highlight < areas.size()) {
               painter.setBrush(this->styles.region_area.highlight.fill);
               painter.setPen(this->styles.region_area.highlight.line);

               auto i = this->state.region_area_to_highlight;
               _draw_poly(i, areas[i]);
            }
         }
      }
      if (this->isDrawingArea()) {
         painter.setBrush(this->styles.region_area.being_drawn.area.fill);
         painter.setPen(this->styles.region_area.being_drawn.area.line);

         const auto& area = this->state.area_being_drawn;
         QPolygon    polygon;
         for (const auto& point : area.points) {
            polygon << mapCoords<CoordinateSpace::World, CoordinateSpace::Widget>({ point.x, point.y });
         }
         painter.drawPolyline(polygon);
         //
         // Draw first and most-recent vertices.
         //
         painter.setBrush(this->styles.region_area.being_drawn.first_vertex.fill);
         painter.setPen(this->styles.region_area.being_drawn.first_vertex.line);
         if (area.points.size() > 1) {
            painter.drawEllipse(polygon.point(0), 3, 3);
         }
         painter.setBrush(this->styles.region_area.being_drawn.latest_vertex.fill);
         painter.setPen(this->styles.region_area.being_drawn.latest_vertex.line);
         painter.drawEllipse(polygon.point(area.points.size() - 1), 3, 3);
      }
   }
   /*virtual*/ void RegionCanvasWidget::resizeEvent(QResizeEvent* event) /*override*/ {
      this->_recalc_layout();
      this->repaint();
   }
   /*virtual*/ void RegionCanvasWidget::wheelEvent(QWheelEvent* event) /*override*/ {
      const auto delta = event->angleDelta().y();
      if (delta < 0) {
         float z = this->state.zoom - 0.1F;
         if (z < minimum_zoom)
            return;
         this->state.zoom = z;
      } else if (delta > 0) {
         if (this->state.zoom > 3.0F)
            return;
         this->state.zoom += 0.1F;
      }
      this->_recalc_scrollbars(false);
      this->repaint();
   }
#pragma endregion

#pragma region Form events
   void RegionCanvasWidget::_on_data_acquired() {
   }
   void RegionCanvasWidget::_on_data_abandoned() {
      this->state.cells.clear();
      this->state.known_regions.clear();
      this->state.current_region = {};
      this->state.worldspace = nullptr;

      this->context.menu.hide();
      this->context.menu.clear();

      this->repaint();
   }
   void RegionCanvasWidget::_on_form_created(dovah::form_stub& stub) {
      if (stub.is_exterior_cell() && stub.get_parent_form() == this->state.worldspace) {
         this->_cache_cell(stub);
         for (auto& cell_info : this->state.cells) {
            if (cell_info.stub != &stub)
               continue;
            for (auto* region : cell_info.regions) {
               auto it = this->state.known_regions.find(region);
               if (it == this->state.known_regions.end())
                  this->_cache_region(*region);
            }
            this->_recalc_cell_color(cell_info);
         }
         //
         // Handle the case of the worldspace being made larger by the addition of this cell.
         //
         {
            int32_t gx;
            int32_t gy;
            if (stub.get_grid_coordinates(gx, gy)) {
               bool changed = false;
               if (gx < this->state.grid_extents.min.x()) {
                  this->state.grid_extents.min.setX(gx);
                  changed = true;
               }
               if (gy < this->state.grid_extents.min.y()) {
                  this->state.grid_extents.min.setY(gy);
                  changed = true;
               }
               if (gx > this->state.grid_extents.max.x()) {
                  this->state.grid_extents.max.setX(gx);
                  changed = true;
               }
               if (gy > this->state.grid_extents.max.y()) {
                  this->state.grid_extents.max.setY(gy);
                  changed = true;
               }
               if (changed) {
                  this->_recalc_scrollbars(false);
               }
            }
         }
         //
         this->repaint();
         return;
      }
      if (stub.form_type == dovah::form_type::region) {
         auto* world = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::region::worldspace>(stub);
         if (world && world == this->state.worldspace) {
            this->_cache_region(stub);
            this->_recalc_cell_colors_affected_by(stub);
            this->repaint();
            return;
         }
      }
   }
   void RegionCanvasWidget::_on_form_modified(dovah::form_stub& stub) {
      if (stub.is_exterior_cell() && stub.get_parent_form() == this->state.worldspace) {
         this->_cache_cell(stub);
         for (auto& info : this->state.cells) {
            if (info.stub == &stub) {
               this->_recalc_cell_color(info);
               break;
            }
         }
         this->repaint();
         return;
      }
      if (stub.form_type == dovah::form_type::region) {
         auto* world = dovah::utils::get_region_worldspace(stub);
         if (world && world == this->state.worldspace) {
            this->_cache_region(stub);
            this->_recalc_cell_colors_affected_by(stub);
            this->repaint();
            return;
         }
      }
   }
   void RegionCanvasWidget::_on_form_deleted(dovah::form_stub& stub) {
      if (stub.form_type == dovah::form_type::worldspace) {
         if (this->state.worldspace == &stub) {
            this->setWorldspace(nullptr);
         }
         return;
      }
      if (stub.form_type == dovah::form_type::cell) {
         auto& list = this->state.cells;
         for (size_t i = 0; i < list.size(); ++i) {
            if (list[i].stub == &stub) {
               list.erase(list.begin() + i);
               this->repaint();
               break;
            }
         }
         return;
      }
      if (stub.form_type == dovah::form_type::region) {
         if (this->state.current_region.stub == &stub) {
            this->state.current_region = {};
            this->_cancel_moving_area();
         }
         for (auto& cell : this->state.cells) {
            auto it = std::find(cell.regions.begin(), cell.regions.end(), &stub);
            if (it != cell.regions.end()) {
               cell.regions.erase(it);
               this->_recalc_cell_color(cell);
            }
         }
         this->state.known_regions.erase(&stub);
         for (auto* action : this->context.menu.actions()) {
            auto data = action->property("region");
            if (data.isValid()) {
               action->setProperty("region", {});
               this->context.menu.removeAction(action);
            }
         }
         this->repaint();
         return;
      }
   }
#pragma endregion

#pragma region Worldspace contents
   void RegionCanvasWidget::_gather_cells_from(dovah::form_stub& worldspace) {
      this->state.grid_extents = {};
      this->state.cells.clear();
      auto* persistent_cell = dovah::form_stub_helpers::get_worldspace_persistent_cell(worldspace);
      dovah::form_stub_helpers::for_each_child_form(worldspace, [this, persistent_cell](dovah::form_stub& cell) {
         if (cell.form_type != dovah::form_type::cell)
            return;
         if (persistent_cell == &cell)
            return;
         auto& info = this->state.cells.emplace_back();
         info.stub = &cell;
         this->_cache_cell(info);

         auto& min_x = this->state.grid_extents.min.rx();
         auto& min_y = this->state.grid_extents.min.ry();
         auto& max_x = this->state.grid_extents.max.rx();
         auto& max_y = this->state.grid_extents.max.ry();
         min_x = std::min(min_x, info.grid.x);
         min_y = std::min(min_y, info.grid.y);
         max_x = std::max(max_x, info.grid.x);
         max_y = std::max(max_y, info.grid.y);
      });
      this->_recalc_scrollbars(true);
      this->repaint();
   }
   void RegionCanvasWidget::_gather_regions() {
      this->state.known_regions.clear();
      if (this->state.worldspace) {
         dovah::form_stub_helpers::for_each_inbound_use_with_flag<dovah::use_info::entry_flags::region::worldspace>(
            *this->state.worldspace,
            [this](dovah::form_stub& region) {
               this->_cache_region(region);
            }
         );
      }
   }
   void RegionCanvasWidget::_cache_cell(dovah::form_stub& cell) {
      for (auto& info : this->state.cells) {
         if (info.stub == &cell) {
            this->_cache_cell(info);
            return;
         }
      }
      auto& info = this->state.cells.emplace_back();
      info.stub = &cell;
      this->_cache_cell(info);
   }
   void RegionCanvasWidget::_cache_cell(KnownCell& info) {
      auto& cell = *info.stub;
      cell.get_grid_coordinates(info.grid.x, info.grid.y);
      info.editor_id = QString::fromStdString(info.stub->editorID);

      info.regions.clear();
      auto loaded_ptr = cell.load().ptr_cast<dovah::loaded_forms::Cell>();
      if (loaded_ptr) {
         auto* extra = loaded_ptr->extra_data.get<dovah::loaded_forms::components::extra_data_types::cell_region_list>();
         if (extra) {
            auto& list = info.regions;
            for (auto& use : extra->regions) {
               auto* region = use.get_form_stub();
               if (!region || region->form_type != dovah::form_type::region)
                  continue;
               list.push_back(region);
            }
            auto last = std::unique(list.begin(), list.end());
            list.erase(last, list.end());
         }
      }
      //
      // Sometimes, REGN/WNAM isn't set but the REGN is still used in a given worldspace, and 
      // this can only be determined via examination of the cells it touches.
      //
      for (auto* region : info.regions) {
         auto it = this->state.known_regions.find(region);
         if (it != this->state.known_regions.end())
            continue;
         auto* world = dovah::utils::get_region_worldspace(*region);
         if (world && world == this->state.worldspace)
            this->_cache_region(*region);
      }
   }
   void RegionCanvasWidget::_cache_region(dovah::form_stub& region) {
      assert(region.form_type == dovah::form_type::region);
      auto& info = this->state.known_regions[&region];
      info.stub = &region;
      this->_cache_region(info);
   }
   void RegionCanvasWidget::_cache_region(KnownRegion& info) {
   info.editor_id = QString::fromStdString(info.stub->editorID);

   info.presence = {};
   auto loaded_ptr = info.stub->load().ptr_cast<dovah::loaded_forms::Region>();
   if (loaded_ptr) {
      info.color = QColor(loaded_ptr->map_color.r, loaded_ptr->map_color.g, loaded_ptr->map_color.b);
      for (const auto& data : loaded_ptr->generable_content) {
         if (data.as<dovah::loaded_forms::structs::region::generable_content::audio>())
            info.presence.audio = true;
         else if (data.as<dovah::loaded_forms::structs::region::generable_content::grass_collection>())
            info.presence.grass = true;
         else if (data.as<dovah::loaded_forms::structs::region::generable_content::landscape>())
            info.presence.landscape = true;
         else if (data.as<dovah::loaded_forms::structs::region::generable_content::map>())
            info.presence.map = true;
         else if (data.as<dovah::loaded_forms::structs::region::generable_content::raw_object_collection>())
            info.presence.objects = true;
         else if (data.as<dovah::loaded_forms::structs::region::generable_content::weather_collection>())
            info.presence.weather = true;
      }
   }
}

   void RegionCanvasWidget::_recalc_all_cell_colors() {
      for (auto& info : this->state.cells)
         this->_recalc_cell_color(info);
   }
   void RegionCanvasWidget::_recalc_cell_colors_affected_by(dovah::form_stub& region) {
      for (auto& info : this->state.cells) {
         bool found = false;
         for (auto* r : info.regions) {
            if (r == &region) {
               found = true;
               break;
            }
         }
         if (!found)
            continue;
         this->_recalc_cell_color(info);
      }
   }
   void RegionCanvasWidget::_recalc_cell_color(KnownCell& cell) {
      cell.color = QColor(255, 255, 255);
      if (!cell.regions.size())
         return;

      uint32_t r     = 0;
      uint32_t g     = 0;
      uint32_t b     = 0;
      size_t   count = 0;
      for (auto* region_stub : cell.regions) {
         if (!region_stub)
            continue;
         auto it = this->state.known_regions.find(region_stub);
         if (it == this->state.known_regions.end())
            continue;
         auto& region_info = it->second;
         if (region_info.color == QColor(0, 0, 0)) // black = no color
            continue;

         bool any = false;
         if (
            (this->state.color_requirements.audio     && region_info.presence.audio)
         || (this->state.color_requirements.grass     && region_info.presence.grass)
         || (this->state.color_requirements.landscape && region_info.presence.landscape)
         || (this->state.color_requirements.map       && region_info.presence.map)
         || (this->state.color_requirements.objects   && region_info.presence.objects)
         || (this->state.color_requirements.weather   && region_info.presence.weather)
         || (this->state.color_requirements.empty     && region_info.presence.none())
         ) {
            any = true;
         }
         if (!any)
            continue;

         r += region_info.color.red();
         g += region_info.color.green();
         b += region_info.color.blue();
         ++count;
      }
      if (!count)
         return;
      r = (float)r / count;
      g = (float)g / count;
      b = (float)b / count;
      cell.color = QColor(r, g, b);
   }
#pragma endregion

void RegionCanvasWidget::_recalc_layout() {
   QRect inner = this->rect();

   this->subwidgets.scrollbar_x->ensurePolished();
   this->subwidgets.scrollbar_y->ensurePolished();
   this->subwidgets.status_bar->ensurePolished();

   auto rect_sb_x = QRect({ 0, 0 }, this->subwidgets.scrollbar_x->sizeHint());
   auto rect_sb_y = QRect({ 0, 0 }, this->subwidgets.scrollbar_y->sizeHint());

   auto rect_stat = QRect({ 0, 0 }, this->subwidgets.status_bar->sizeHint());

   inner.setWidth( inner.width()  - rect_sb_y.width());
   inner.setHeight(inner.height() - rect_sb_x.height() - rect_stat.height());
   this->state.viewport = inner;

   rect_sb_x.translate(0, inner.height());
   rect_sb_x.setWidth(inner.width());
   rect_sb_y.translate(inner.width(), 0);
   rect_sb_y.setHeight(inner.height());
   this->subwidgets.scrollbar_x->setGeometry(rect_sb_x);
   this->subwidgets.scrollbar_y->setGeometry(rect_sb_y);

   rect_stat.translate(0, rect_sb_x.bottom());
   rect_stat.setWidth(inner.width());
   this->subwidgets.status_bar->setGeometry(rect_stat);

   this->_recalc_scrollbars(false);
}
void RegionCanvasWidget::_recalc_scrollbars(bool reset_scroll) {
   //
   // Start by calculating the worldspace size in pixels.
   //
   const float cell_size       = default_cell_size * this->state.zoom;
   const float cell_size_prior = default_cell_size * this->state.last_rendered_zoom;
   
   QPointF scroll_center = { 0, 0 };
   if (!reset_scroll) {
      scroll_center = this->scrollCenter() / cell_size_prior;
   }

   const auto& viewport = this->state.viewport;

   QRect worldspace_canvas_rect;
   {
      const auto corner_offset = (grid_to_corner_offset * cell_size).toPoint();
      const auto margin_offset = QPoint(min_grid_margin, min_grid_margin);
      worldspace_canvas_rect.setBottomLeft(
         mapCoords<CoordinateSpace::Grid, CoordinateSpace::Canvas>(
            this->state.grid_extents.min - margin_offset
         ) - corner_offset
      );
      worldspace_canvas_rect.setTopRight(
         mapCoords<CoordinateSpace::Grid, CoordinateSpace::Canvas>(
            this->state.grid_extents.max + margin_offset
         ) + corner_offset
      );

      auto grid_center = worldspace_canvas_rect.center();
      if (worldspace_canvas_rect.width() < viewport.width()) {
         worldspace_canvas_rect.setWidth(viewport.width());
         worldspace_canvas_rect.moveCenter(grid_center);
      }
      if (worldspace_canvas_rect.height() < viewport.height()) {
         worldspace_canvas_rect.setHeight(viewport.height());
         worldspace_canvas_rect.moveCenter(grid_center);
      }
   }

   int x1 = worldspace_canvas_rect.left();
   int x2 = worldspace_canvas_rect.right() - viewport.width();
   if (x2 < x1)
      x2 = x1;
   this->subwidgets.scrollbar_x->setRange(x1, x2);
   this->subwidgets.scrollbar_x->setPageStep(viewport.width());

   int y1 = worldspace_canvas_rect.top();
   int y2 = worldspace_canvas_rect.bottom() - viewport.height();
   if (y2 < y1)
      y2 = y1;
   this->subwidgets.scrollbar_y->setRange(y1, y2);
   this->subwidgets.scrollbar_y->setPageStep(viewport.height());

   this->subwidgets.scrollbar_x->setValue(scroll_center.x() * cell_size - (float)viewport.width()  / 2);
   this->subwidgets.scrollbar_y->setValue(scroll_center.y() * cell_size - (float)viewport.height() / 2);
   this->state.last_rendered_zoom = this->state.zoom;
}

void RegionCanvasWidget::_clear_status_panels() {
   this->subwidgets.status_panels.grid->setText("");
   this->subwidgets.status_panels.world->setText("");
}
void RegionCanvasWidget::_update_status_panels(const QPoint& canvas_pos) {
   auto    grid_pos = mapCoords<CoordinateSpace::Canvas, CoordinateSpace::Grid>(canvas_pos);
   QString cell_name;
   for (auto& cell : this->state.cells) {
      if (grid_pos.x() == cell.grid.x && grid_pos.y() == cell.grid.y) {
         cell_name = cell.editor_id;
         break;
      }
   }
   if (!cell_name.isEmpty()) {
      this->subwidgets.status_panels.grid->setText(tr("(%1, %2)\"%3\"", "status bar: cell grid pos and editor ID").arg(grid_pos.x()).arg(grid_pos.y()).arg(cell_name));
   } else {
      this->subwidgets.status_panels.grid->setText(tr("(%1, %2)", "status bar: cell grid pos").arg(grid_pos.x()).arg(grid_pos.y()));
   }

   auto world_pos = mapCoords<CoordinateSpace::Canvas, CoordinateSpace::World>(canvas_pos);
   this->subwidgets.status_panels.world->setText(tr("(%1, %2)", "status bar: world pos").arg(world_pos.x()).arg(world_pos.y()));
}

void RegionCanvasWidget::_start_panning(QPoint pos) {
   if (this->state.panning)
      return;
   this->state.panning        = true;
   this->state.last_mouse_pos = pos;
   this->_update_cursor();
}
void RegionCanvasWidget::_stop_panning() {
   if (!this->state.panning)
      return;
   this->state.panning = false;
   this->_update_cursor();
}

bool RegionCanvasWidget::_can_close_polygon_at(const QPoint& canvas_pos) const {
   if (!this->isDrawingArea())
      return false;
   const auto& area = this->state.area_being_drawn;
   if (area.points.size() < 3)
      return false;
   
   auto& first_point     = area.points[0];
   auto  first_on_canvas = mapCoords<CoordinateSpace::World, CoordinateSpace::Canvas>({ first_point.x, first_point.y });
   //
   auto  diff     = QPointF(first_on_canvas) - canvas_pos;
   auto  distance = std::sqrt(QPointF::dotProduct(diff, diff));
   return (distance < QApplication::startDragDistance());
}
void RegionCanvasWidget::_draw_point_at(const QPoint& canvas_pos) {
   if (this->state.current_region.stub == nullptr) {
      return;
   }
   auto& area = this->state.area_being_drawn;
   if (_can_close_polygon_at(canvas_pos)) {
      if (area.would_become_self_intersecting(area.points[0])) {
         QApplication::beep();
         this->subwidgets.status_bar->showMessage(tr("Can't close the polygon. The polygon would become self-intersecting."), 3);
         return;
      }
      this->_close_polygon_being_drawn();
   } else {
      auto world_pos = mapCoords<CoordinateSpace::Canvas, CoordinateSpace::World>(canvas_pos);

      RegionArea::point to_add;
      to_add.x = world_pos.x();
      to_add.y = world_pos.y();
      if (area.would_become_self_intersecting(to_add)) {
         QApplication::beep();
         this->subwidgets.status_bar->showMessage(tr("Can't place a point there. The polygon would become self-intersecting."), 3);
      } else {
         area.points.push_back(to_add);
         this->_update_cursor();
         this->repaint();
      }
   }
}
void RegionCanvasWidget::_close_polygon_being_drawn() {
   this->state.current_region.areas.push_back(std::move(this->state.area_being_drawn));
   this->state.area_being_drawn.points.clear();
   this->_update_cursor();
   this->repaint();
   emit onRegionAreasEdited();
}

void RegionCanvasWidget::_start_moving_area(size_t which) {
   if (this->isDrawingArea())
      return;
   if (this->isMovingArea())
      this->_cancel_moving_area();

   if (!this->region())
      return;
   if (which >= this->state.current_region.areas.size())
      return;

   this->state.moving_region_area = which;
   this->state.moving_area_delta  = QPointF(0, 0);
   this->state.last_mouse_pos     = QCursor::pos();
   this->_update_cursor();

   this->subwidgets.status_bar->showMessage(
      tr("Left-click to finish; right-click to cancel.")
   );
}
void RegionCanvasWidget::_cancel_moving_area() {
   if (!this->isMovingArea())
      return;
   this->state.moving_region_area = index_of_none;
   this->_update_cursor();
   this->repaint();

   this->subwidgets.status_bar->clearMessage();
}
void RegionCanvasWidget::_finish_moving_area() {
   if (!this->isMovingArea() || !this->region())
      return;
   auto which = this->state.moving_region_area;
   if (which >= this->state.current_region.areas.size())
      return;
   auto& area = this->state.current_region.areas[which];
   for (auto& point : area.points) {
      point.x += this->state.moving_area_delta.x();
      point.y += this->state.moving_area_delta.y();
   }

   this->state.moving_region_area = index_of_none;
   this->_update_cursor();
   this->repaint();

   this->subwidgets.status_bar->clearMessage();

   emit this->onRegionAreasEdited();
}

void RegionCanvasWidget::_update_cursor() {
   if (this->isMovingArea()) {
      this->setCursor(Qt::CursorShape::SizeAllCursor);
   } else if (this->state.panning) {
      this->setCursor(Qt::CursorShape::ClosedHandCursor);
   } else if (this->isDrawingArea()) {
      auto local_pos  = mapFromGlobal(QCursor::pos());
      auto canvas_pos = mapCoords<CoordinateSpace::Widget, CoordinateSpace::Canvas>(local_pos);
      if (_can_close_polygon_at(canvas_pos)) {
         this->setCursor(Qt::CursorShape::PointingHandCursor);
      } else {
         this->setCursor(Qt::CursorShape::CrossCursor);
      }
   } else {
      this->unsetCursor();
   }
}

#pragma region Context menu
   void RegionCanvasWidget::_build_context_menu() {
      auto& menu    = this->context.menu;
      auto& actions = this->context.actions;

      menu.clear();
      if (this->isDrawingArea()) {
         menu.addAction(actions.clear_last_point);
         menu.addAction(actions.close_polygon);
         menu.addAction(actions.cancel_drawing);

         bool can_delete = this->state.area_being_drawn.points.size() >= 2;
         bool can_close  = this->state.area_being_drawn.points.size() >= 3;
         actions.clear_last_point->setEnabled(can_delete);
         actions.close_polygon->setEnabled(can_close);
      } else {
         auto local_pos = mapFromGlobal(QCursor::pos());

         std::vector<size_t> areas;
         auto regions = this->regionsUnderPoint(local_pos);
         if (this->region()) {
            areas = this->areasUnderPoint(local_pos);
         }
         
         if (!areas.empty()) {
            for (size_t area_index : areas) {
               auto* action = new QAction(tr("Move area %1").arg(area_index), &menu);
               action->setProperty(context_menu_item_region_area_index_property, (int)area_index);
               menu.addAction(action);
               QObject::connect(action, &QAction::triggered, this, [this, action]() {
                  auto index = action->property(context_menu_item_region_area_index_property).toInt();
                  this->_start_moving_area(index);
               });
            }
            menu.addSeparator();
            for (size_t area_index : areas) {
               auto* action = new QAction(tr("Delete area %1").arg(area_index), &menu);
               action->setProperty(context_menu_item_region_area_index_property, (int)area_index);
               menu.addAction(action);
               QObject::connect(action, &QAction::triggered, this, [this, action]() {
                  auto index = action->property(context_menu_item_region_area_index_property).toInt();
                  this->_context_delete_region_area(index);
               });
            }
         }
         if (!areas.empty() && !regions.empty()) {
            menu.addSeparator();
         }
         for (auto& item : regions) {
            auto* action = new QAction(tr("Switch to %1").arg(item.second), &menu);
            action->setProperty("region", QVariant::fromValue(item.first));
            if (item.first == this->region()) {
               action->setEnabled(false);
            }
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, [this, action]() {
               auto stub = action->property("region").value<dovah::form_stub*>();
               emit this->onRegionChangeRequested(stub);
            });
         }
      }
   }

   #pragma region Region area actions
      void RegionCanvasWidget::_context_delete_region_area(size_t i) {
         auto& areas = this->state.current_region.areas;
         if (i >= areas.size())
            return;
         areas.erase(areas.begin() + i);
         this->repaint();
      }
   #pragma endregion
   #pragma region Drawing-new-area actions
      void RegionCanvasWidget::_context_clear_last_point() {
         auto& area = this->state.area_being_drawn;
         if (area.points.size() < 2)
            return;
         area.points.pop_back();
         this->_update_cursor();
         this->repaint();
      }
      void RegionCanvasWidget::_context_cancel_drawing_area() {
         this->state.area_being_drawn = {};
         this->_update_cursor();
         this->repaint();
      }
      void RegionCanvasWidget::_context_finish_drawing_area() {
         auto& area = this->state.area_being_drawn;
         if (area.points.size() < 3)
            return;
         if (area.would_become_self_intersecting(area.points[0]))
            return;
         this->_close_polygon_being_drawn();
      }
   #pragma endregion
#pragma endregion