#pragma once
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <QLabel>
#include <QMenu>
#include <QScrollBar>
#include <QStatusBar>
#include <QWidget>
#include "ui/types/regions/region.h"
namespace dovah {
   class form_stub;
}

class RegionCanvasWidget : public QWidget {
   Q_OBJECT;
   public:
      static constexpr const int default_cell_size = 32; // size in pixels including the border

      static constexpr const int cell_border_width = 2; // width of the border, treating it as centered between cells

      // i.e. minimum allowed size of a cell, in pixels, divided by default size
      static constexpr const float minimum_zoom = 4.0F / default_cell_size;

      // if the worldspace is large enough to fill the canvas, show at least this many empty cells on 
      // all sizes
      static constexpr const int min_grid_margin = 5;

   public:
      struct RegionDataPresence {
         constexpr bool operator==(const RegionDataPresence&) const noexcept = default;

         bool audio     = false;
         bool grass     = false;
         bool landscape = false;
         bool map       = false;
         bool objects   = false;
         bool weather   = false;

         constexpr bool none() const noexcept {
            return !this->audio && !this->grass && !this->landscape && !this->map && !this->objects && !this->weather;
         }
      };
      
      struct RegionColorRequirements : public RegionDataPresence {
         constexpr bool operator==(const RegionColorRequirements&) const noexcept = default;

         bool empty = false;

         constexpr bool none() const noexcept {
            return this->RegionDataPresence::none() && !this->empty;
         }
      };

      using RegionArea = ui::types::regions::region::area;

      // Ordering of constants in here matters; should be "outer" to "inner."
      enum class CoordinateSpace {
         Screen,

         // Pixel coordinates relative to the widget's bounds.
         Widget,

         // Pixel coordinates within the drawn worldspace, irrespective of any scrolling. 
         // This coordinate space is centered on (0, 0), like grid and world coordinates; 
         // the scrollbars can have negative minimums.
         Canvas,

         // Cell grid coordinates. Integer values. Converting these to any other space 
         // yields the centerpoint of the drawn cell.
         Grid,

         // In-game coordinates measured in world units. A cell is 4096x4096wu.
         World
      };

      template<CoordinateSpace Space>
      using Point = std::conditional_t<
         (Space == CoordinateSpace::World),
         QPointF,
         QPoint
      >;

   public:
      RegionCanvasWidget(QWidget* parent = nullptr);
      ~RegionCanvasWidget();

   protected:
      struct KnownCell {
         QColor  color; // cached, based on region color reqs
         QString editor_id;
         struct {
            int32_t x = 0;
            int32_t y = 0;
         } grid;
         dovah::form_stub* stub = nullptr;
         std::vector<dovah::form_stub*> regions;
      };
      struct KnownRegion {
         QColor  color;
         QString editor_id;
         RegionDataPresence presence;
         dovah::form_stub* stub = nullptr;
      };

      struct {
         QMenu menu;
         struct {
            QAction* clear_last_point = nullptr;
            QAction* cancel_drawing   = nullptr;
            QAction* close_polygon    = nullptr;
         } actions;
      } context;
      struct {
         float last_rendered_zoom = 1.0F;
         float zoom = 1.0F;
         QRect viewport; // in pixels; local coordinates excluding scrollbar areas
         struct {
            QPoint min; // grid Y is flipped, so this is the local bottom-left
            QPoint max; // grid Y is flipped, so this is the local top-right
         } grid_extents;

         std::vector<KnownCell> cells;
         dovah::form_stub* worldspace = nullptr;

         struct {
            dovah::form_stub* stub = nullptr;
            std::vector<RegionArea> areas;
         } current_region;
         RegionArea area_being_drawn;

         std::unordered_map<dovah::form_stub*, KnownRegion> known_regions;

         RegionColorRequirements color_requirements;

         bool   panning = false; // with middle mouse button
         QPoint panning_from;
      } state;
      struct {
         QScrollBar* scrollbar_x = nullptr;
         QScrollBar* scrollbar_y = nullptr;
         QStatusBar* status_bar  = nullptr;
         struct {
            QLabel* grid  = nullptr;
            QLabel* world = nullptr;
         } status_panels;
      } subwidgets;

   public:
      #pragma region Current world/region focus
         constexpr dovah::form_stub* region() const noexcept { return this->state.current_region.stub; }
         void setRegion(const ui::types::regions::region&);
         void setNoRegion();

         constexpr dovah::form_stub* worldspace() const noexcept { return this->state.worldspace; }
         void setWorldspace(dovah::form_stub*);
      #pragma endregion
      #pragma region Editing helpers
         void forceRegionColor(dovah::form_stub&, QColor);
         void forceRegionDataPresence(dovah::form_stub&, const RegionDataPresence&);
      #pragma endregion
      #pragma region Widget state accessors
         constexpr bool isDrawingArea() const noexcept {
            return !this->state.area_being_drawn.points.empty();
         }
         QPointF scrollCenter() const noexcept;
         QPoint scrollPosition() const noexcept;
      #pragma endregion
      #pragma region Worldspace contents accessors
         [[nodiscard]] std::vector<size_t> areasUnderPoint(const QPoint& local_pos) const noexcept;

         constexpr const std::vector<RegionArea>& regionAreas() const noexcept { return this->state.current_region.areas; }

         // Queries by cell, not by region area; it's inexact, but to an acceptable degree.
         [[nodiscard]] std::vector<std::pair<dovah::form_stub*, QString>> regionsUnderPoint(const QPoint& local_pos) const noexcept;
      #pragma endregion
      
      void setColorRequirements(const RegionColorRequirements&);

      #pragma region Coordinate space conversions
         template<CoordinateSpace src_space, CoordinateSpace dst_space> requires (src_space != dst_space)
         auto mapCoords(const Point<src_space>&) const;

         QPoint mapWorldToGridPos(const QPointF&) const;
         QPointF mapGridToWorldPos(const QPoint&) const;
         QPoint mapWorldToCanvasPos(const QPointF&) const;
         QPointF mapCanvasToWorldPos(const QPoint&) const;

         QPoint mapGridToCanvasPos(const QPoint&) const;
         QPoint mapCanvasToGridPos(const QPoint&) const;

         QPoint mapCanvasToWidgetPos(const QPoint&) const;
         QPoint mapWidgetToCanvasPos(const QPoint&) const;

         QPoint mapWidgetToScreenPos(const QPoint&) const;
         QPoint mapScreenToWidgetPos(const QPoint&) const;
      #pragma endregion

      #pragma region Widget API
         virtual QSize minimumSizeHint() const override;
      #pragma endregion
      #pragma region Events
         virtual void contextMenuEvent(QContextMenuEvent*) override;
         virtual void leaveEvent(QEvent*) override;
         virtual void mouseMoveEvent(QMouseEvent*) override;
         virtual void mousePressEvent(QMouseEvent*) override;
         virtual void mouseReleaseEvent(QMouseEvent*) override;
         virtual void paintEvent(QPaintEvent*) override;
         virtual void resizeEvent(QResizeEvent*) override;
         virtual void wheelEvent(QWheelEvent*) override;
      #pragma endregion

   signals:
      void onRegionAreasEdited();
      void onRegionChangeRequested(dovah::form_stub* region);

   protected:
      #pragma region Form events
         void _on_data_acquired();
         void _on_data_abandoned();
         void _on_form_created(dovah::form_stub&);
         void _on_form_modified(dovah::form_stub&);
         void _on_form_deleted(dovah::form_stub&);
      #pragma endregion

      #pragma region Worldspace contents
         void _gather_cells_from(dovah::form_stub& worldspace);
         void _gather_regions();
         void _cache_cell(dovah::form_stub&);
         void _cache_cell(KnownCell&);
         void _cache_region(dovah::form_stub&);
         void _cache_region(KnownRegion&);

         void _recalc_all_cell_colors();
         void _recalc_cell_colors_affected_by(dovah::form_stub& region);
         void _recalc_cell_color(KnownCell&);
      #pragma endregion

      void _recalc_layout();
      void _recalc_scrollbars(bool reset_scroll);

      void _clear_status_panels();
      void _update_status_panels(const QPoint& canvas_pos);

      void _start_panning(QPoint pos);
      void _stop_panning();

      bool _can_close_polygon_at(const QPoint& canvas_pos) const;
      void _draw_point_at(const QPoint& canvas_pos);
      void _close_polygon_being_drawn();

      void _update_cursor();

      #pragma region Context menu
         void _build_context_menu();

         #pragma region Region area actions
            void _context_delete_region_area(size_t i);
         #pragma endregion
         #pragma region Drawing-new-area actions
            void _context_clear_last_point();
            void _context_cancel_drawing_area();
            void _context_finish_drawing_area();
         #pragma endregion
      #pragma endregion
};

#include "./RegionCanvasWidget.inl"