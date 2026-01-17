#include "./RegionCanvasWidget.h"
#include <QPainter>
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/for_each_inbound_use_with_flag.h"
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/forms/components/extra_data/types/c/cell_region_list.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/Region.h"
#include "dovah/use_info/entry_flags/region.h"
#include "editor/core.h"

RegionCanvasWidget::RegionCanvasWidget(QWidget* parent) : QWidget(parent) {
   {
      auto* scrollbar = this->subwidgets.scrollbar_x = new QScrollBar(this);
      scrollbar->setOrientation(Qt::Orientation::Horizontal);
   }
   {
      auto* scrollbar = this->subwidgets.scrollbar_y = new QScrollBar(this);
      scrollbar->setOrientation(Qt::Orientation::Vertical);
   }

   auto& editor = DovahKitCore::get();
   static_assert(false, "TODO: editor events for form creation/modification/deletion");
}

void RegionCanvasWidget::scrollContentsBy(int dx, int dy);
void RegionCanvasWidget::scrollTo(int x, int y);
void RegionCanvasWidget::setRegion(dovah::form_stub* stub);
void RegionCanvasWidget::setWorldspace(dovah::form_stub* stub) {
   if (!stub) {
      if (!this->state.worldspace)
         return;
      this->state.cells.clear();
      this->state.known_regions.clear();
      this->state.region = nullptr;
      this->state.worldspace = nullptr;
      this->state.bounds = {};
      this->update();
      return;
   }
   if (stub->form_type != dovah::form_type::worldspace)
      return;
   if (stub == this->state.worldspace)
      return;

   this->state.region = nullptr;
   this->state.worldspace = stub;

   this->_gather_cells_from(*stub);
   this->_gather_regions();
   this->update();
}

dovah::form_stub* RegionCanvasWidget::cellAt(int x_px, int y_px) const;
std::vector<dovah::form_stub*> RegionCanvasWidget::regionsAt(int x_px, int y_px) const;

void RegionCanvasWidget::forceRegionColor(dovah::form_stub& region, QColor c) {
   auto it = this->state.known_regions.find(&region);
   if (it != this->state.known_regions.end()) {
      it->second.color = c;
      this->repaint();
   }
}

#pragma region Widget API
   /*virtual*/ QSize RegionCanvasWidget::minimumSizeHint() const /*override*/ {
      this->subwidgets.scrollbar_x->ensurePolished();
      this->subwidgets.scrollbar_y->ensurePolished();
      int w = this->subwidgets.scrollbar_y->width();
      int h = this->subwidgets.scrollbar_x->height();
      return QSize{ w * 2, h * 2 };
   }
#pragma endregion
#pragma region Events
   /*virtual*/ void RegionCanvasWidget::contextMenuEvent(QContextMenuEvent* event) /*override*/ {
      //
      // TODO
      //
   }
   /*virtual*/ void RegionCanvasWidget::mouseMoveEvent(QMouseEvent* event) /*override*/ {
      //
      // TODO: update status bar based on mouse position
      //
   }
   /*virtual*/ void RegionCanvasWidget::mousePressEvent(QMouseEvent* event) /*override*/ {
      //
      // TODO: handle drawing a region area
      //
   }
   /*virtual*/ void RegionCanvasWidget::paintEvent(QPaintEvent* event) /*override*/ {
      QPainter painter(this);

      painter.setBrush(QColor(0, 0, 0));
      painter.drawRect(this->rect());

      const float cell_size = default_cell_size * this->state.zoom;

      int scroll_x_px = this->subwidgets.scrollbar_x->value();
      int scroll_y_px = this->subwidgets.scrollbar_y->value();
      int scroll_x_gr = scroll_x_px / cell_size;
      int scroll_y_gr = scroll_y_px / cell_size;
      for (auto& cell : this->state.cells) {
         int cell_x_px = cell.grid.x * cell_size - scroll_x_px;
         int cell_y_px = cell.grid.y * cell_size - scroll_y_px;

         QRect cell_rect;
         cell_rect.setX(cell_x_px - (cell_size / 2));
         cell_rect.setY(cell_y_px - (cell_size / 2));
         cell_rect.setWidth(cell_size - 2);
         cell_rect.setHeight(cell_size - 2);

         auto color = QColor(255, 255, 255);
         {
            std::vector<QColor> colors_to_blend;
            for (auto* region_stub : cell.regions) {
               if (!region_stub)
                  continue;
               auto it = this->state.known_regions.find(region_stub);
               if (it == this->state.known_regions.end())
                  continue;
               auto& region_info = it->second;
               if (region_info.color == QColor(0, 0, 0)) // black = no color
                  continue;
               //
               // TODO: If region isn't visible, skip it.
               //
               colors_to_blend.push_back(region_info.color);
            }
            if (!colors_to_blend.empty()) {
               uint32_t r = color.red();
               uint32_t g = color.green();
               uint32_t b = color.blue();
               for (auto& other : colors_to_blend) {
                  r += other.red();
                  g += other.green();
                  b += other.blue();
               }
               r = (float)r / (colors_to_blend.size() + 1);
               g = (float)g / (colors_to_blend.size() + 1);
               b = (float)b / (colors_to_blend.size() + 1);
               color = QColor(r, g, b);
            }
         }
         painter.setBrush(color);
         painter.drawRect(cell_rect);
      }

      //
      // TODO: Draw current region's areas
      //

      //
      // TODO: If user is editing an area, draw that area now
      //
   }
   /*virtual*/ void RegionCanvasWidget::resizeEvent(QResizeEvent* event) /*override*/ {
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
      this->state.region     = nullptr;
      this->state.worldspace = nullptr;
      this->repaint();
   }
   void RegionCanvasWidget::_on_form_created(dovah::form_stub& stub);
   void RegionCanvasWidget::_on_form_modified(dovah::form_stub& stub);
   void RegionCanvasWidget::_on_form_deleted(dovah::form_stub& stub);
#pragma endregion

void RegionCanvasWidget::_gather_cells_from(dovah::form_stub& worldspace) {
   this->state.cells.clear();
   dovah::form_stub_helpers::for_each_child_form(worldspace, [this](dovah::form_stub& cell) {
      if (cell.form_type != dovah::form_type::cell)
         return;
      auto& info = this->state.cells.emplace_back();
      info.stub = &cell;
      this->_cache_cell(info);

      auto& min_x = this->state.bounds.grid.min.rx();
      auto& min_y = this->state.bounds.grid.min.ry();
      auto& max_x = this->state.bounds.grid.max.rx();
      auto& max_y = this->state.bounds.grid.max.ry();
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
void RegionCanvasWidget::_cache_cell(dovah::form_stub& cell);
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
         for (auto& use : extra->regions)
            list.push_back(use.get_form_stub());
         auto last = std::unique(list.begin(), list.end());
         list.erase(last, list.end());
      }
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
      for (auto& data : loaded_ptr->generable_content) {
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

void RegionCanvasWidget::_recalc_scrollbars(bool reset_scroll) {
   //
   // Start by calculating the worldspace size in pixels.
   //
   const float cell_size       = default_cell_size * this->state.zoom;
   const float cell_size_prior = default_cell_size * this->state.last_rendered_zoom;
   //
   int center_grid_x = 0;
   int center_grid_y = 0;
   if (!reset_scroll) {
      auto scroll_x = this->subwidgets.scrollbar_x->value() + (float)this->subwidgets.scrollbar_x->pageStep() / 2;
      auto scroll_y = this->subwidgets.scrollbar_y->value() + (float)this->subwidgets.scrollbar_y->pageStep() / 2;
      center_grid_x = std::round(scroll_x / cell_size_prior);
      center_grid_y = std::round(scroll_y / cell_size_prior);
   }
   this->subwidgets.scrollbar_x->setRange(this->state.bounds.grid.min.x() * cell_size, this->state.bounds.grid.max.x() * cell_size);
   this->subwidgets.scrollbar_x->setPageStep(this->rect().width());
   this->subwidgets.scrollbar_y->setRange(this->state.bounds.grid.min.y() * cell_size, this->state.bounds.grid.max.y() * cell_size);
   this->subwidgets.scrollbar_y->setPageStep(this->rect().height());
   this->subwidgets.scrollbar_x->setValue(center_grid_x * cell_size);
   this->subwidgets.scrollbar_y->setValue(center_grid_y * cell_size);
   //
   this->state.last_rendered_zoom = this->state.zoom;
}