#pragma once
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <QBrush>
#include <QLabel>
#include <QMenu>
#include <QPen>
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
      // Width of the border drawn between cells; a constant size regardless of zoom level. 
      // The border is drawn between cells, such that half the border width overlaps the 
      // cell on each side of the border.
      static constexpr const int cell_border_width = 2;

      // Size (width and height) of a cell in pixels at the default zoom. This size includes 
      // the area overlapped by the border between cells.
      static constexpr const int default_cell_size = 32;

      // The minimum zoom factor, computed from the minimum allowed size of a cell in pixels 
      // divided by the default size in pixels.
      static constexpr const float minimum_zoom = (float)(cell_border_width * 2) / default_cell_size;

      // If the worldspace is large enough to fill the canvas, show at least this many empty cells on 
      // all sizes. This allows the user to create new cells by drawing region areas out in the void.
      static constexpr const int min_grid_margin = 5;

   protected:
      static constexpr const size_t index_of_none = (size_t)-1;

   public:
      #pragma region Region-data-presence definitions
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
      
         // A region will affect cells' colors if at least one bool is `true` in both the region data 
         // presence and in the color requirements, or if no region-data-presence bool is true and the 
         // `empty` bool is true in the color requirements.
         struct RegionColorRequirements : public RegionDataPresence {
            constexpr bool operator==(const RegionColorRequirements&) const noexcept = default;

            bool empty = false;

            constexpr bool none() const noexcept {
               return this->RegionDataPresence::none() && !this->empty;
            }
         };
      #pragma endregion

      using RegionArea = ui::types::regions::region::area;

      #pragma region Coordinate space definitions
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
      #pragma endregion

      #pragma region Widget style definitions
         struct RegionAreaStyle {
            QBrush fill;
            QPen   line;
         };
         struct VertexStyle {
            QBrush fill;
            QPen   line;
            int    radius = 0;
         };
         struct RegionAreaInProgressStyle {
            RegionAreaStyle area;
            VertexStyle first_vertex;
            VertexStyle latest_vertex;
         };

         struct Style {
            struct {
               QBrush background;
            } grid;
            struct {
               RegionAreaStyle normal;
               RegionAreaStyle highlight;
               RegionAreaInProgressStyle being_drawn;
            } region_area;
         };
      #pragma endregion

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
         } grid; // i.e. position within the worldspace
         dovah::form_stub* stub = nullptr;
         std::vector<dovah::form_stub*> regions; // CELL/XCLR: all regions overlapping this cell
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
         float last_rendered_zoom = 1.0F; // used to preserve scroll-center position when the zoom changes
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

         QPoint last_mouse_pos; // global position. for panning, and for moving region areas.

         bool panning = false; // with middle mouse button

         size_t  moving_region_area = index_of_none;
         QPointF moving_area_delta; // distance the area has been moved, measured in world coordinates

         // When the mouse cursor is over a context menu item pertaining to a specific 
         // region area, we want to highlight that area.
         size_t region_area_to_highlight = index_of_none;
      } state;
      Style styles;
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
         constexpr bool isMovingArea() const noexcept {
            return this->state.moving_region_area != index_of_none;
         }

         // The centerpoint of the viewport, in canvas-relative coordinates.
         QPointF scrollCenter() const noexcept;

         // The top-left corner of the viewport, in canvas-relative coordinates.
         QPoint scrollPosition() const noexcept;
      #pragma endregion
      #pragma region Worldspace contents accessors
         [[nodiscard]] std::vector<size_t> areasUnderPoint(const QPoint& local_pos) const noexcept;

         constexpr const std::vector<RegionArea>& regionAreas() const noexcept { return this->state.current_region.areas; }

         // Queries by cell, not by region area; it's inexact, but to an acceptable degree.
         [[nodiscard]] std::vector<std::pair<dovah::form_stub*, QString>> regionsUnderPoint(const QPoint& local_pos) const noexcept;
      #pragma endregion
      
      void setColorRequirements(const RegionColorRequirements&);

      constexpr const Style& widgetStyles() const noexcept { return this->styles; }
      void setWidgetStyles(const Style& s) {
         if (&s == &this->styles)
            return;
         this->styles = s;
         this->repaint();
      }

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

      void _start_moving_area(size_t which);
      void _cancel_moving_area();
      void _finish_moving_area();

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