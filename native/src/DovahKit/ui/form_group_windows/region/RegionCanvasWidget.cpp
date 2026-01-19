#include "./RegionCanvasWidget.h"
#include <QPainter>
#include <QWheelEvent>
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

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &RegionCanvasWidget::_on_data_abandoned);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &RegionCanvasWidget::_on_data_acquired);
   QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   if (editor.has_data()) {
      this->_on_data_acquired();
   }
}
RegionCanvasWidget::~RegionCanvasWidget() {
}

void RegionCanvasWidget::setRegion(const ui::types::regions::region& region) {
   if (!region.stub)
      return;
   if (region.bounds.worldspace && region.bounds.worldspace != this->state.worldspace)
      return;
   this->state.current_region.stub  = region.stub;
   this->state.current_region.areas = region.bounds.areas;
   //
   // TODO: Update any extant context menus?
   //
   this->repaint(); // to render region areas
}
void RegionCanvasWidget::setNoRegion() {
   this->state.current_region = {};
   //
   // TODO: Update any extant context menus?
   //
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
      this->state.grid_extents = {};
      this->update();
      return;
   }
   if (stub->form_type != dovah::form_type::worldspace)
      return;
   if (stub == this->state.worldspace)
      return;

   this->state.worldspace = stub;
   this->state.current_region = {};

   this->_gather_regions();
   this->_gather_cells_from(*stub);
   this->update();
}

void RegionCanvasWidget::setColorRequirements(const RegionColorRequirements& req) {
   this->state.color_requirements = req;
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
      return src - QPoint(this->subwidgets.scrollbar_x->value(), this->subwidgets.scrollbar_y->value());
   }
   QPoint RegionCanvasWidget::mapWidgetToCanvasPos(const QPoint& src) const {
      return src + QPoint(this->subwidgets.scrollbar_x->value(), this->subwidgets.scrollbar_y->value());
   }

   QPoint RegionCanvasWidget::mapWidgetToScreenPos(const QPoint& p) const {
      return this->mapToGlobal(p);
   }
   QPoint RegionCanvasWidget::mapScreenToWidgetPos(const QPoint& p) const {
      return this->mapFromGlobal(p);
   }
#pragma endregion

void RegionCanvasWidget::forceRegionColor(dovah::form_stub& region, QColor c) {
   auto it = this->state.known_regions.find(&region);
   if (it != this->state.known_regions.end()) {
      it->second.color = c;
      this->repaint();
   }
}
void RegionCanvasWidget::forceRegionDataPresence(dovah::form_stub& region, const RegionDataPresence& presence) {
   auto it = this->state.known_regions.find(&region);
   if (it != this->state.known_regions.end()) {
      it->second.presence = presence;
      this->repaint();
   }
}

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
      //
      // TODO
      //
   }
   /*virtual*/ void RegionCanvasWidget::leaveEvent(QEvent* event) /*override*/ {
      this->_clear_status_panels();
   }
   /*virtual*/ void RegionCanvasWidget::mouseMoveEvent(QMouseEvent* event) /*override*/ {
      if (this->state.panning) {
         auto pos   = event->globalPos();
         auto delta = pos - this->state.panning_from;
         this->state.panning_from = pos;

         auto* sbx = this->subwidgets.scrollbar_x;
         auto* sby = this->subwidgets.scrollbar_y;
         sbx->setValue(sbx->value() + delta.x());
         sby->setValue(sby->value() + delta.y());
      }
      this->_update_status_panels(mapCoords<CoordinateSpace::Widget, CoordinateSpace::Canvas>(event->localPos().toPoint()));
   }
   /*virtual*/ void RegionCanvasWidget::mousePressEvent(QMouseEvent* event) /*override*/ {
      auto local_pos    = event->localPos().toPoint();
      bool is_on_canvas = this->state.view_size.contains(local_pos);
      if (is_on_canvas) {
         switch (event->button()) {
            case Qt::MouseButton::MiddleButton:
               this->_start_panning(event->globalPos());
               break;
            case Qt::MouseButton::LeftButton:
               //
               // TODO: handle drawing a region area
               //
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

      painter.setClipRect(this->state.view_size);
      painter.setBrush(QColor(0, 0, 0));
      painter.drawRect(this->state.view_size);

      const float cell_size = default_cell_size * this->state.zoom;

      int scroll_x_px = this->subwidgets.scrollbar_x->value();
      int scroll_y_px = this->subwidgets.scrollbar_y->value();
      int scroll_x_gr = scroll_x_px / cell_size;
      int scroll_y_gr = scroll_y_px / cell_size;
      for (auto& cell : this->state.cells) {
         QPoint cell_centerpoint = mapCoords<CoordinateSpace::Grid, CoordinateSpace::Widget>({ cell.grid.x, cell.grid.y });
         QRect  cell_rect;
         cell_rect.setX(cell_centerpoint.x() - (cell_size / 2) + (cell_border_width / 2));
         cell_rect.setY(cell_centerpoint.y() - (cell_size / 2) + (cell_border_width / 2));
         cell_rect.setWidth(cell_size - cell_border_width);
         cell_rect.setHeight(cell_size - cell_border_width);

         painter.setBrush(_recalc_cell_color(cell));
         painter.drawRect(cell_rect);
      }
      //
      // Draw the current region's areas.
      //
      {
         auto& areas = this->state.current_region.areas;
         if (!areas.empty()) {
            painter.setBrush(QColor(0, 0, 0, 0));
            {
               QPen pen;
               pen.setCosmetic(true);
               pen.setColor(QColor(128, 0, 0));
               pen.setWidth(3);
               painter.setPen(pen);
            }
            for (const auto& area : areas) {
               QPolygon polygon;
               for (const auto& point : area.points) {
                  polygon << mapCoords<CoordinateSpace::World, CoordinateSpace::Widget>({ point.x, point.y });
               }
               painter.drawPolygon(polygon);
            }
         }
      }
      //
      // TODO: If user is editing an area, draw that area now
      //
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
      this->repaint();
   }
   void RegionCanvasWidget::_on_form_created(dovah::form_stub& stub) {
      if (stub.is_exterior_cell() && stub.get_parent_form() == this->state.worldspace) {
         this->_cache_cell(stub);
         this->repaint();
         for (auto& cell_info : this->state.cells) {
            if (cell_info.stub != &stub)
               continue;
            for (auto* region : cell_info.regions) {
               auto it = this->state.known_regions.find(region);
               if (it == this->state.known_regions.end())
                  this->_cache_region(*region);
            }
         }
         return;
      }
      if (stub.form_type == dovah::form_type::region) {
         auto* world = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::region::worldspace>(stub);
         if (world && world == this->state.worldspace) {
            this->_cache_region(stub);
            this->repaint();
            return;
         }
      }
   }
   void RegionCanvasWidget::_on_form_modified(dovah::form_stub& stub) {
      if (stub.is_exterior_cell() && stub.get_parent_form() == this->state.worldspace) {
         this->_cache_cell(stub);
         this->repaint();
         return;
      }
      if (stub.form_type == dovah::form_type::region) {
         auto* world = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::region::worldspace>(stub);
         if (world && world == this->state.worldspace) {
            this->_cache_region(stub);
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
         }
         for (auto& cell : this->state.cells) {
            auto it = std::find(cell.regions.begin(), cell.regions.end(), &stub);
            if (it != cell.regions.end())
               cell.regions.erase(it);
         }
         this->state.known_regions.erase(&stub);
         this->repaint();
         return;
      }
   }
#pragma endregion

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

QColor RegionCanvasWidget::_recalc_cell_color(KnownCell& cell) const {
   if (!cell.regions.size())
      return QColor(255, 255, 255);

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
      return QColor(255, 255, 255);
   r = (float)r / count;
   g = (float)g / count;
   b = (float)b / count;
   return QColor(r, g, b);
}

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
   this->state.view_size = inner;

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
   //
   float center_x = 0;
   float center_y = 0;
   if (!reset_scroll) {
      auto scroll_x = this->subwidgets.scrollbar_x->value() + (float)this->subwidgets.scrollbar_x->pageStep() / 2;
      auto scroll_y = this->subwidgets.scrollbar_y->value() + (float)this->subwidgets.scrollbar_y->pageStep() / 2;
      center_x = scroll_x / cell_size_prior;
      center_y = scroll_y / cell_size_prior;
   }

   QRect view_rect = this->state.view_size;
   QRect grid_rect;
   grid_rect.setBottomLeft(
      mapCoords<CoordinateSpace::Grid, CoordinateSpace::Canvas>(this->state.grid_extents.min) -
      (grid_to_corner_offset * cell_size).toPoint()
   );
   grid_rect.setTopRight(
      mapCoords<CoordinateSpace::Grid, CoordinateSpace::Canvas>(this->state.grid_extents.max) +
      (grid_to_corner_offset * cell_size).toPoint()
   );

   int x1 = grid_rect.left();
   int x2 = grid_rect.right() - view_rect.width();
   if (x2 < x1)
      x2 = x1;
   this->subwidgets.scrollbar_x->setRange(x1, x2);
   this->subwidgets.scrollbar_x->setPageStep(view_rect.width());

   int y1 = grid_rect.top();
   int y2 = grid_rect.bottom() - view_rect.height();
   if (y2 < y1)
      y2 = y1;
   this->subwidgets.scrollbar_y->setRange(y1, y2);
   this->subwidgets.scrollbar_y->setPageStep(view_rect.height());

   this->subwidgets.scrollbar_x->setValue(center_x * cell_size - view_rect.width()/2);
   this->subwidgets.scrollbar_y->setValue(center_y * cell_size - view_rect.height()/2);
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
   this->state.panning      = true;
   this->state.panning_from = pos;
   this->setCursor(Qt::CursorShape::ClosedHandCursor);
}
void RegionCanvasWidget::_stop_panning() {
   if (!this->state.panning)
      return;
   this->state.panning = false;
   this->unsetCursor();
}