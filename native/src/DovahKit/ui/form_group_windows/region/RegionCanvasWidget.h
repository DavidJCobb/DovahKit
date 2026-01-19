#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <QScrollBar>
#include <QWidget>
namespace dovah {
   class form_stub;
}

class RegionCanvasWidget : public QWidget {
   Q_OBJECT;
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

   public:
      RegionCanvasWidget(QWidget* parent = nullptr);
      ~RegionCanvasWidget();

   protected:
      static constexpr const int default_cell_size = 32; // size in pixels including the border

      // i.e. minimum allowed size of a cell, in pixels, divided by default size
      static constexpr const float minimum_zoom = 4.0F / default_cell_size;

      struct KnownCell {
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
         float last_rendered_zoom = 1.0F;
         float zoom = 1.0F;
         QRect view_size; // in pixels; local coordinates excluding scrollbar areas
         struct {
            QPoint min; // grid Y is flipped, so this is the local bottom-left
            QPoint max; // grid Y is flipped, so this is the local top-right
         } grid_extents;

         std::vector<KnownCell> cells;
         dovah::form_stub* region     = nullptr;
         dovah::form_stub* worldspace = nullptr;

         std::unordered_map<dovah::form_stub*, KnownRegion> known_regions;

         RegionColorRequirements color_requirements;

         bool   panning = false; // with middle mouse button
         QPoint panning_from;
      } state;
      struct {
         QScrollBar* scrollbar_x = nullptr;
         QScrollBar* scrollbar_y = nullptr;
      } subwidgets;

   public:
      void setRegion(dovah::form_stub*);
      void setWorldspace(dovah::form_stub*);

      void setColorRequirements(const RegionColorRequirements&);

      #pragma region Coordinate space conversions
         //
         // Coordinate spaces:
         //  - global = whole screen
         //  - local  = widget
         //  - canvas = entire displayed worldspace, in pixels, not accounting for scrolling
         //  - grid   = cell grid, where each integer coordinate is a cell's centerpoint
         //  - world  = coordinates measured in world units
         //
         QPoint localPosToGridPos(const QPoint&) const;
         QPoint gridPosToLocalPos(const QPoint&) const; // returns centerpoint
         QPoint gridPosToCanvasPos(const QPoint&) const; // returns centerpoint; does not adjust for scrolling
      #pragma endregion

      void forceRegionColor(dovah::form_stub&, QColor);
      void forceRegionDataPresence(dovah::form_stub&, const RegionDataPresence&);

      #pragma region Widget API
         virtual QSize minimumSizeHint() const override;
      #pragma endregion
      #pragma region Events
         virtual void contextMenuEvent(QContextMenuEvent*) override;
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

      void _gather_cells_from(dovah::form_stub& worldspace);
      void _gather_regions();
      void _cache_cell(dovah::form_stub&);
      void _cache_cell(KnownCell&);
      void _cache_region(dovah::form_stub&);
      void _cache_region(KnownRegion&);

      QColor _recalc_cell_color(KnownCell&) const;

      void _recalc_layout();
      void _recalc_scrollbars(bool reset_scroll);

      void _start_panning(QPoint pos);
      void _stop_panning();
};
