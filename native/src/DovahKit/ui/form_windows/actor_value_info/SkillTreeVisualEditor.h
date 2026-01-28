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

      struct PerkNodePosition {
         struct {
            uint32_t x = 0;
            uint32_t y = 0;
         } grid;
         struct {
            float x = 0;
            float y = 0;
         } offset;
      };
      
      struct PerkNodeData {
         dovah::form_stub* perk  = nullptr;
         dovah::form_stub* skill = nullptr;
         PerkNodePosition position;
         bool parent_required = true;
      };

      struct LineStyle {
         QColor color     = QColor(0, 0, 0);
         size_t thickness = 1; // canvas-relative size

         QPen pen(float zoom = 1) const;
      };
      struct ShapeStyle {
         QBrush    fill;
         LineStyle line;
      };
      struct TextStyle {
         QColor    color;
         QFont     font;
         LineStyle stroke;
      };

      struct ConnectorStyle {
         struct : public ShapeStyle {
            size_t length = 10;
         } head;
         LineStyle stem;
      };
      struct NodeStyle : public ShapeStyle {
         float radius = 5; // canvas-relative size
         struct : public TextStyle {
            float distance = 7; // vertical displacement from point; canvas-relative size
         } text;
      };

   protected:
      static constexpr const unsigned int grid_cell_w = 125;
      static constexpr const unsigned int grid_cell_h = 100;
      static constexpr const float        offset_factor_x = 50;
      static constexpr const float        offset_factor_y = 50;

      static constexpr const size_t minimum_row_count = 5;
      static constexpr const size_t minimum_col_count = 5;
      static constexpr const size_t maximum_row_count = 5; // 0 = no maximum

      static constexpr const float minimum_node_distance = 5;

      struct PerkNode {
         public:
            node_id      id = 0; // INAM
            PerkNodeData data;
            std::vector<node_id> connects_to;
            struct {
               QString perk_editor_id;
               struct {
                  QPointF centerpoint;
                  QRect   box; // used for root
                  QRect   text;
               } geometry;
            } _cached;

         public:
            bool has_outbound_connection_to(const PerkNode&) const;
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

         bool nodeIsRoot(node_id) const;

         constexpr const optional_node_id& selectedNodeID() const { return this->_selected_node_id; }
         void setSelectedNodeID(optional_node_id);

         const PerkNodeData* nodeData(node_id id) const;
         void setNodeData(node_id id, const PerkNodeData&);
      #pragma endregion
      #pragma region Overrides
         virtual QSize minimumSizeHint() const override;
         virtual QSize sizeHint() const override;

         virtual void contextMenuEvent(QContextMenuEvent*) override;
         virtual void focusOutEvent(QFocusEvent*) override;
         virtual void mousePressEvent(QMouseEvent*) override;
         virtual void mouseMoveEvent(QMouseEvent*) override;
         virtual void mouseReleaseEvent(QMouseEvent*) override;
         #pragma region Override helpers: paintEvent
            void _prepare_painter_for_style(QPainter&, const LineStyle&);
            void _prepare_painter_for_style(QPainter&, const ShapeStyle&);
            void _prepare_painter_for_style(QPainter&, const TextStyle&);
            void _prepare_painter_for_style(QPainter&, const TextStyle& child, const TextStyle& parent);
            void _draw_gridlines(QPainter&);
            void _draw_connection(QPainter&, const PerkNode& src, const PerkNode& dst);
            void _draw_node_label(QPainter&, const PerkNode&, const NodeStyle&);
            void _draw_node_shape(QPainter&, const PerkNode&, const NodeStyle&);
         #pragma endregion
         virtual void paintEvent(QPaintEvent*) override;
      #pragma endregion

   signals:
      void nodePositionChanged(node_id nodeID, const PerkNodePosition&);
      void selectionChanged(optional_node_id nodeID, optional_node_id priorNodeID);

   protected:
      dovah::form_stub* _actor_value = nullptr;
      std::vector<std::unique_ptr<PerkNode>> _nodes;
      optional_node_id _selected_node_id;
      //
      QPointer<QScrollArea> _scroll_area;
      float _zoom = 1.0F;
      //
      struct {
         struct {
            QColor color;
            QFont  font;
            size_t indent  =  5; // canvas-relative size
            size_t width   = 40; // canvas-relative size
            bool   visible = true;
         } advisory_level_labels;
         QColor background;
         struct {
            ConnectorStyle optional;
            ConnectorStyle required;
         } connectors;
         LineStyle gridline;
         struct {
            NodeStyle general;
            NodeStyle selected;
            struct {
               size_t    corner_radius = 5; // canvas-relative size
               size_t    distance = 0; // canvas-relative size
               size_t    padding  = 0; // canvas-relative size
               TextStyle text;
            } root;
         } nodes;
         struct {
            QMarginsF scaled;
            QMarginsF absolute;
         } margins; // spacing between the inner edges of the scroll area, and the outer edges of the canvas
      } _style;
      struct {
         uint32_t row_count = 0;
         uint32_t col_count = 0;
         QSize    canvas_size;
      } _cached;
      struct {
         QMenu menu;

         QAction* new_perk         = nullptr;
         QAction* new_connection   = nullptr;
         QAction* sever_connection = nullptr;
         QAction* snap_to_grid     = nullptr;
         QAction* remove_perk      = nullptr;

         PerkNode* target = nullptr;
      } _context_menu;
      struct {
         QPoint    mousedown_at; // widget-relative
         PerkNode* mousedown_on = nullptr;
         QPoint    mouse_prev_pos; // screen-relative
         bool      is_drag_moving = false;
         bool      is_panning     = false;
         bool      is_cancelled   = false;
      } _mouse;
      struct {
         optional_node_id source_node;
         bool is_connecting = true; // false == severing a connection
      } _connecting;

      #pragma region Data (protected)
         bool _node_is_root(const PerkNode&) const;

         PerkNode* _create_node_at(const QPoint& global_pos);
         void _delete_node(PerkNode&);
         node_id _find_free_node_id() const;
         const PerkNode* _node_by_id(node_id) const;
         PerkNode* _node_by_id(node_id);

         // If there's an existing node too close to `mapped_pos`, then nudges `mapped_pos` away.
         void _find_available_node_position(QPoint& mapped_pos, optional_node_id node_id_to_ignore = {});
      #pragma endregion
      #pragma region Node interactions
         void _select_node(const PerkNode*);
         #pragma region Connect/disconnect nodes
            bool _is_in_node_connection_interaction() const;
            void _start_node_connection_interaction(const PerkNode&, bool connecting);
            void _cancel_node_connection_interaction();
            void _complete_node_connection_interaction(PerkNode* dst);
         #pragma endregion
         void _set_node_position(PerkNode&, QPointF canvas_relative);
      #pragma endregion

      // Map a point from global to canvas-relative, i.e. accounting for the current zoom 
      // and margins.
      QPoint _map_from_global(const QPoint&);

      PerkNode* _get_node_at_point(const QPoint& canvas_relative);

      // Canvas-relative.
      QPointF _node_centerpoint(const PerkNode&) const;

      void _update_cursor(const QMouseEvent*);

      struct _geometry_update_info {
         _geometry_update_info();

         float max_radius = 0;
         struct {
            QFontMetrics font_metrics;
            float max_distance = 0;
            int   max_size     = 0;
         } label;
      };
      //
      _geometry_update_info _get_geometry_update_info() const;
      void _update_geometry();
      void _update_node_geometry(PerkNode&, const _geometry_update_info&);
      void _update_node_geometry(PerkNode&);
};
