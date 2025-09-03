#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <QMenu>
#include <QPointer>
#include <QScrollArea>
#include <QWidget>
namespace dovah {
   class form_stub;
}

class SkillTreeVisualEditor : public QWidget {
   Q_OBJECT;
   public:
      using node_id = uint32_t;
      using optional_node_id = std::optional<node_id>;

      struct PerkNodeData {
         dovah::form_stub* perk  = nullptr;
         dovah::form_stub* skill = nullptr;
         struct {
            struct {
               uint32_t x = 0;
               uint32_t y = 0;
            } grid;
            struct {
               float x = 0;
               float y = 0;
            } offset;
         } position;
         bool parent_required = true;
      };

      struct ConnectorStyle {
         struct {
            QBrush fill = QColor(255, 255, 255);
            struct {
               QColor color     = QColor(0, 0, 0);
               size_t thickness = 1;
            } line;
         } head;
         struct {
            QColor color     = QColor(0, 0, 0);
            size_t thickness = 1;
         } stem;
      };
      struct NodeStyle {
         QBrush fill   = QColor(128, 128, 128);
         float  radius = 5;
         struct {
            QColor color     = QColor(0, 0, 0);
            size_t thickness = 1;
         } line;
         struct {
            float  distance = 7; // vertical displacement from point
            QColor color    = QColor(0, 0, 0);
            QFont  font;
         } text;
      };

   protected:
      static constexpr const unsigned int grid_cell_w = 125;
      static constexpr const unsigned int grid_cell_h = 100;
      static constexpr const float        offset_factor_x = 50;
      static constexpr const float        offset_factor_y = 50;

      static constexpr const float minimum_node_distance = 5;

      struct PerkNode {
         node_id      id = 0; // INAM
         PerkNodeData data;
         std::vector<node_id> connects_to;
         struct {
            QPointF centerpoint;
            QString perk_editor_id;
         } _cached;
      };

   public:
      SkillTreeVisualEditor(QWidget* parent = nullptr);
      ~SkillTreeVisualEditor();

      // HACK. Ideally, the QScrollArea should be part of this widget itself.
      void setContainingScrollArea(QScrollArea*);

      #pragma region Properties
         constexpr float zoom() const noexcept { return this->_zoom; }
         void setZoom(float);
      #pragma endregion

      #pragma region Data
         constexpr dovah::form_stub* currentActorValue() const noexcept { return this->_actor_value; }
         void setCurrentActorValue(dovah::form_stub*);

         // Import data to/from the current AV. These prefer the AV's working-copy 
         // if there is one.
         void importData();
         void exportData();

         void clear();

         constexpr const optional_node_id& selectedNodeID() const { return this->_selected_node_id; }
         void setSelectedNodeID(optional_node_id);

         const PerkNodeData* nodeData(node_id id) const;
         void setNodeData(node_id id, const PerkNodeData&);
      #pragma endregion
      #pragma region Overrides
         virtual QSize minimumSizeHint() const override;
         virtual QSize sizeHint() const override;

         virtual void contextMenuEvent(QContextMenuEvent* event) override;
         virtual void mousePressEvent(QMouseEvent*) override;
         virtual void mouseMoveEvent(QMouseEvent*) override;
         virtual void mouseReleaseEvent(QMouseEvent*) override;
         #pragma region Drag and drop
            virtual void dragEnterEvent(QDragEnterEvent*) override;
            virtual void dragMoveEvent(QDragMoveEvent*) override;
            virtual void dropEvent(QDropEvent*) override;
         #pragma endregion
         virtual void paintEvent(QPaintEvent*) override;
      #pragma endregion

   signals:
      void selectionChanged(std::optional<uint32_t> nodeID, std::optional<uint32_t> priorNodeID);

   protected:
      dovah::form_stub* _actor_value = nullptr;
      std::vector<std::unique_ptr<PerkNode>> _nodes;
      optional_node_id _selected_node_id;
      QPointer<QScrollArea> _scroll_area;
      float _zoom = 1.0F;
      struct {
         struct {
            ConnectorStyle optional = {
               .head = {
                  .line = {
                     .color = QColor(0, 140, 255)
                  }
               },
               .stem = {
                  .color = QColor(0, 140, 255)
               }
            };
            ConnectorStyle required;
         } connectors;
         struct {
            QColor color     = QColor(160, 160, 160);
            size_t thickness = 1;
         } gridline;
         struct {
            NodeStyle general;
            NodeStyle selected = {
               .fill = QColor(255, 0, 0),
               .line = {
                  .color = QColor(255, 0, 0),
               },
               .text = {
                  .color = QColor(255, 0, 0),
               }
            };
         } nodes;
         QMarginsF margins = { 20, 20, 20, 20 };
      } _style;
      struct {
         uint32_t row_count = 0;
         uint32_t col_count = 0;
         QSize    size;
      } _cached;
      struct {
         QMenu menu;

         QAction* new_perk     = nullptr;
         QAction* snap_to_grid = nullptr;
         QAction* remove_perk  = nullptr;
      } _context_menu;
      struct {
         QPoint    mousedown_at; // widget-relative
         PerkNode* mousedown_on = nullptr;
         QPoint    mouse_prev_pos; // screen-relative
         bool      is_panning   = false;
      } _mouse;

      #pragma region Data (protected)
         PerkNode* _create_node_at(const QPoint& global_pos);
         node_id _find_free_node_id() const;
         const PerkNode* _node_by_id(node_id) const;
         PerkNode* _node_by_id(node_id);
         bool node_is_invisible(const PerkNode&) const;

         // If there's an existing node too close to `mapped_pos`, then nudges `mapped_pos` away.
         void _find_available_node_position(QPoint& mapped_pos, optional_node_id node_id_to_ignore = {});
      #pragma endregion

      // Map a point from global to canvas-relative, i.e. accounting for the current zoom 
      // and margins.
      QPoint _map_from_global(const QPoint&);

      PerkNode* _get_node_at_point(const QPoint& local_pos);

      QPointF _node_centerpoint(const PerkNode&) const;

      void _update_cursor(const QMouseEvent*);
      void _update_geometry();

      void _select_node(const PerkNode*);
};
