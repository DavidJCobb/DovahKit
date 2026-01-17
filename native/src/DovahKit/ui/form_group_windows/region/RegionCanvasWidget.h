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
      RegionCanvasWidget(QWidget* parent = nullptr);

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
         struct {
            bool audio     = false;
            bool grass     = false;
            bool landscape = false;
            bool map       = false;
            bool objects   = false;
            bool weather   = false;
         } presence;
         dovah::form_stub* stub = nullptr;
      };

      struct {
         float last_rendered_zoom = 1.0F;
         float zoom = 1.0F;
         QRect view_size; // in pixels
         struct {
            QPoint min; // grid Y is flipped, so this is the local bottom-left
            QPoint max; // grid Y is flipped, so this is the local top-right
         } grid_extents;

         std::vector<KnownCell> cells;
         dovah::form_stub* region     = nullptr;
         dovah::form_stub* worldspace = nullptr;

         std::unordered_map<dovah::form_stub*, KnownRegion> known_regions;
      } state;
      struct {
         QScrollBar* scrollbar_x = nullptr;
         QScrollBar* scrollbar_y = nullptr;
      } subwidgets;

   public:
      void setRegion(dovah::form_stub*);
      void setWorldspace(dovah::form_stub*);

      QPoint localPosToGridPos(const QPoint&) const;
      QPoint gridPosToLocalPos(const QPoint&) const; // returns centerpoint
      QPoint gridPosToCanvasPos(const QPoint&) const; // returns centerpoint; does not adjust for scrolling

      void forceRegionColor(dovah::form_stub&, QColor);

      #pragma region Widget API
         virtual QSize minimumSizeHint() const override;
      #pragma endregion
      #pragma region Events
         virtual void contextMenuEvent(QContextMenuEvent*) override;
         virtual void mouseMoveEvent(QMouseEvent*) override;
         virtual void mousePressEvent(QMouseEvent*) override;
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

      void _recalc_layout();
      void _recalc_scrollbars(bool reset_scroll);
};
