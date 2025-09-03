#include "./SkillTreeVisualEditor.h"
#include <QContextMenuEvent>
#include <QEvent>
#include <QPainter>
#include "helpers/bitset.h"
#include "helpers/math/rotation/unit_conversion.h"
#include "helpers/math/cosine.h"
#include "helpers/math/sine.h"
#include "dovah/forms/ActorValueInfo.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#pragma region Drag and drop
   #include <QByteArray>
   #include <QDrag>
   #include <QMimeData>
#pragma endregion
#pragma region Panning
   #include <QApplication>
   #include <QScrollBar>
#pragma endregion

namespace {
   static constexpr const std::string_view drag_mime_type_for_perk_nodes = "application/dovah-kit.perk-tree-editor.perk-node";
}

SkillTreeVisualEditor::SkillTreeVisualEditor(QWidget* parent) : QWidget(parent) {
   this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
   this->setSizePolicy({ QSizePolicy::Fixed, QSizePolicy::Fixed });

   this->setAcceptDrops(true);
   this->setMouseTracking(true); // to update the cursor depending on what drawn boxes it's over

   #pragma region Form data updates
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         switch (stub->form_type) {
            case dovah::form_type::perk:
               break;
            default:
               return;
         }
         bool any_changed = false;
         for (auto& node_ptr : this->_nodes) {
            if (node_ptr->data.perk == stub) {
               any_changed = true;
               node_ptr->_cached.perk_editor_id = QString::fromStdString(stub->editorID);
            }
         }
         if (any_changed) {
            this->_update_geometry();
            this->update();
         }
      });
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
         if (stub == this->_actor_value) {
            this->_actor_value = nullptr;
         }
         {
            auto& list        = this->_nodes;
            bool  any_deleted = false;
            for (auto& node_ptr : list) {
               if (node_ptr->data.perk == stub) {
                  any_deleted = true;
                  node_ptr->data.perk = nullptr;
                  if (node_ptr->id == this->_selected_node_id) {
                     this->_selected_node_id = {};
                  }
               }
               if (node_ptr->data.skill == stub) {
                  node_ptr->data.skill = nullptr;
               }
            }
            if (any_deleted) {
               std::erase_if(
                  list,
                  [](const auto& node_ptr) {
                     return node_ptr->data.perk == nullptr;
                  }
               );
               this->_update_geometry();
               this->update();
            }
         }
      });
   #pragma endregion
   #pragma region Context menu
   {
      auto& menu    = this->_context_menu.menu;
      auto& actions = this->_context_menu;

      {
         auto* action = actions.new_perk = new QAction(tr("Add perk"));
         menu.addAction(action);
         //
         // We need to know where the user clicked, so this has to be done in the context menu 
         // event handler, not here.
         //
      }
      {
         auto* action = actions.snap_to_grid = new QAction(tr("Snap to grid"));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            if (!this->_selected_node_id.has_value())
               return;
            auto* node = this->_node_by_id(this->_selected_node_id.value());
            if (!node)
               return;
            node->data.position.offset = {};
            this->update();
         });
      }
      {
         auto* action = actions.remove_perk = new QAction(tr("Remove perk"));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            const optional_node_id prior = this->_selected_node_id;
            if (!prior.has_value())
               return;
            auto& list = this->_nodes;
            for (size_t i = 0; i < list.size(); ++i) {
               if (list[i]->id != prior)
                  continue;
               list.erase(list.begin() + i);
               this->_selected_node_id = {};
               this->_update_geometry();
               this->update();
               emit selectionChanged({}, prior);
               return;
            }
         });
      }
   }
   #pragma endregion
}
SkillTreeVisualEditor::~SkillTreeVisualEditor() {
}

void SkillTreeVisualEditor::setContainingScrollArea(QScrollArea* w) {
   this->_scroll_area = w;
}

#pragma region Properties
   void SkillTreeVisualEditor::setZoom(float z) {
      if (fabs(z - this->_zoom) < 0.00001F)
         return;
      this->_zoom = z;
      this->_update_geometry();
      this->update();
   }
#pragma endregion
#pragma region Data
   void SkillTreeVisualEditor::setCurrentActorValue(dovah::form_stub* av) {
      if (av && av->form_type != dovah::form_type::actor_value_info)
         return;
      this->_actor_value = av;
   }

   // Import data to/from the current AV. These prefer the AV's working-copy 
   // if there is one.
   void SkillTreeVisualEditor::importData() {
      using loaded_form_type = dovah::loaded_forms::ActorValueInfo;

      this->_nodes.clear();
      this->_selected_node_id.reset();
      this->_cached = {};

      if (!this->_actor_value)
         return;
      dovah::loaded_form_ptr<loaded_form_type> loaded_ptr;
      loaded_form_type* loaded = nullptr;
      if (auto* working = this->_actor_value->get_working_copy()) {
         loaded = (loaded_form_type*) working;
      } else {
         loaded_ptr = this->_actor_value->load().ptr_cast<loaded_form_type>();
         loaded     = loaded_ptr.unwrap();
      }
      if (!loaded)
         return;

      const auto& src_list = loaded->perk_tree_nodes;
      auto&       dst_list = this->_nodes;
      dst_list.reserve(src_list.size());
      for (auto& src_node : src_list) {
         auto& dst_node_ptr = dst_list.emplace_back(std::make_unique<PerkNode>());
         auto& dst_node     = *dst_node_ptr;
         dst_node.id         = src_node.id;
         dst_node.data.perk  = src_node.perk.get_form_stub();
         dst_node.data.skill = src_node.skill.get_form_stub();
         dst_node.data.position = {
            .grid = {
               .x = src_node.x,
               .y = src_node.y,
            },
            .offset = {
               .x = src_node.position.h,
               .y = src_node.position.v,
            }
         };
         dst_node.data.parent_required = src_node.flags & loaded_form_type::perk_tree_node::flag::parents_required;
         dst_node.connects_to = src_node.connections;

         if (dst_node.data.perk) {
            dst_node._cached.perk_editor_id = QString::fromStdString(dst_node.data.perk->editorID);
         }
      }
      //
      // Enforce unique IDs.
      //
      for (size_t i = 0; i < dst_list.size(); ++i) {
         auto& a_ptr = dst_list[i];
         for (size_t j = i + 1; j < dst_list.size(); ++j) {
            auto& b_ptr = dst_list[j];
            if (a_ptr->id == b_ptr->id) {
               b_ptr->id = _find_free_node_id();
            }
         }
      }

      this->_update_geometry();
      this->update();
   }
   void SkillTreeVisualEditor::exportData() {
      using loaded_form_type = dovah::loaded_forms::ActorValueInfo;

      if (!this->_actor_value)
         return;
      dovah::loaded_form_ptr<loaded_form_type> loaded_ptr;
      loaded_form_type* loaded = nullptr;
      if (auto* working = this->_actor_value->get_working_copy()) {
         loaded = (loaded_form_type*)working;
      } else {
         loaded_ptr = this->_actor_value->load().ptr_cast<loaded_form_type>();
         loaded = loaded_ptr.unwrap();
      }
      if (!loaded)
         return;

      const auto& src_list = this->_nodes;
      auto&       dst_list = loaded->perk_tree_nodes;
      for (auto& dst_node : dst_list) {
         dst_node.perk.set(*loaded, nullptr);
         dst_node.skill.set(*loaded, nullptr);
      }
      dst_list.clear();

      dst_list.reserve(src_list.size());
      for (auto& src_item_ptr : src_list) {
         assert(src_item_ptr != nullptr);
         auto& src_item = *src_item_ptr;
         auto& dst_item = dst_list.emplace_back();
         dst_item.id = src_item.id;
         dst_item.perk.set(*loaded, src_item.data.perk);
         dst_item.skill.set(*loaded, src_item.data.skill);
         dst_item.x = src_item.data.position.grid.x;
         dst_item.y = src_item.data.position.grid.y;
         dst_item.position.h = src_item.data.position.offset.x;
         dst_item.position.v = src_item.data.position.offset.y;
         if (src_item.data.parent_required)
            dst_item.flags |= loaded_form_type::perk_tree_node::flag::parents_required;
         dst_item.connections = src_item.connects_to;
      }
   }

   void SkillTreeVisualEditor::clear() {
      this->_actor_value = nullptr;
      this->_nodes.clear();
      this->_selected_node_id.reset();
      this->_cached = {};
      this->update();
   }

   void SkillTreeVisualEditor::setSelectedNodeID(optional_node_id id) {
      if (id == this->_selected_node_id)
         return;
      if (id.has_value()) {
         if (!this->_node_by_id(id.value())) {
            id.reset();
            if (!this->_selected_node_id.has_value())
               return;
         }
      }
      this->_selected_node_id = id;
      this->update();
   }

   const SkillTreeVisualEditor::PerkNodeData* SkillTreeVisualEditor::nodeData(node_id id) const {
      auto* node = this->_node_by_id(id);
      if (node)
         return &node->data;
      return nullptr;
   }
   void SkillTreeVisualEditor::setNodeData(node_id id, const PerkNodeData& src) {
      auto* node = this->_node_by_id(id);
      if (!node)
         return;
      node->data = src;
      if (node->data.perk) {
         node->_cached.perk_editor_id = QString::fromStdString(node->data.perk->editorID);
      } else {
         node->_cached.perk_editor_id = tr("<MISSING PERK>");
      }
      node->_cached.centerpoint = _node_centerpoint(*node);
      this->update();
   }
#pragma endregion
#pragma region Data (protected)
   SkillTreeVisualEditor::PerkNode* SkillTreeVisualEditor::_create_node_at(const QPoint& global_pos) {
      node_id id       = _find_free_node_id();
      auto&   node_ptr = this->_nodes.emplace_back(std::make_unique<PerkNode>());
      auto&   node     = *node_ptr;
      node.id = id;

      auto pos = _map_from_global(global_pos);
      if (pos.x() < 0)
         pos.setX(0);
      if (pos.y() < 0)
         pos.setY(0);
      _find_available_node_position(pos);
      node.data.position.grid.x = pos.x() / grid_cell_w;
      node.data.position.grid.y = pos.x() / grid_cell_h;
      node.data.position.offset.x = (pos.x() % grid_cell_w) / offset_factor_x;
      node.data.position.offset.y = (pos.y() % grid_cell_h) / offset_factor_y;
      
      this->_update_geometry();
      this->update();

      return &node;
   }
   SkillTreeVisualEditor::node_id SkillTreeVisualEditor::_find_free_node_id() const {
      if (this->_nodes.empty())
         return 0;
      //
      // Optimization on the assumption that most perk trees won't have in 
      // excess of 64 nodes.
      //
      cobb::bitset<64> low_bits;
      for (const auto& node_ptr : this->_nodes)
         if (node_ptr->id < 64)
            low_bits.set(node_ptr->id);
      if (!low_bits.all()) {
         return low_bits.find_first_clear();
      }
      //
      // Uh oh, we've got a huge perk tree.
      //
      std::vector<PerkNode*> sorted;
      {
         const auto size = this->_nodes.size();
         sorted.resize(size);
         for (size_t i = 0; i < size; ++i) {
            sorted[i] = this->_nodes[i].get();
         }
         std::sort(
            sorted.begin(),
            sorted.end(),
            [](const auto* a, const auto* b) {
               return a->id < b->id;
            }
         );
      }
      node_id i = 64;
      for (; i < 999999; ++i) {
         for (auto* node : sorted) {
            if (node->id < i)
               continue;
            if (node->id == i)
               break;
            if (node->id > i)
               return i;
         }
      }
      return i;
   }
   const SkillTreeVisualEditor::PerkNode* SkillTreeVisualEditor::_node_by_id(node_id id) const {
      for (auto& node_ptr : _nodes)
         if (node_ptr->id == id)
            return node_ptr.get();
      return nullptr;
   }
   SkillTreeVisualEditor::PerkNode* SkillTreeVisualEditor::_node_by_id(node_id id) {
      return const_cast<PerkNode*>(std::as_const(*this)._node_by_id(id));
   }
   bool SkillTreeVisualEditor::node_is_invisible(const PerkNode& node) const {
      //
      // All skill trees have an invisible root node. The root has no perk, and typically 
      // has wildly invalid position values (probably uninitialized memory). To prevent 
      // this from causing problems when drawing, calculating size, doing hit testing, et 
      // cetera, we want to filter this node out from most such tasks.
      //
      return node.id == 0;
   }

   void SkillTreeVisualEditor::_find_available_node_position(QPoint& mapped_pos, optional_node_id node_id_to_ignore) {
      constexpr auto _dist = [](const QPointF& a, const QPoint& b) -> float {
         QPointF diff = a - b;
         return sqrt(diff.x()*diff.x() + diff.y()*diff.y());
      };

      auto _has_overlap = [this, &node_id_to_ignore](const QPoint& pos) -> bool {
         for (const auto& node_ptr : this->_nodes) {
            if (node_ptr->id == node_id_to_ignore)
               continue;
            if (_dist(node_ptr->_cached.centerpoint, pos) < minimum_node_distance)
               return true;
         }
         return false;
      };

      if (!_has_overlap(mapped_pos))
         return;
      //
      // Try nearby coordinates.
      // 
      // We'll start by grouping all extant nodes into buckets, based on their (rounded) 
      // distance from the mapped position. Then, for each integer distance, we'll search 
      // in a ring around the mapped position to find an open spot.
      // 
      // This is probably a bad algorithm, but frankly, I think a user would have to be 
      // actively trying to do something stupid in order to get especially poor performance 
      // from this (e.g. packing a perk tree with several hundred perks in a dense grid).
      //
      std::unordered_map<uint32_t, std::vector<QPointF>> occupied_points_by_distance;
      for (const auto& node_ptr : this->_nodes) {
         if (node_ptr->id == node_id_to_ignore)
            continue;
         float distance = _dist(node_ptr->_cached.centerpoint, mapped_pos);
         occupied_points_by_distance[(uint32_t)distance].push_back(node_ptr->_cached.centerpoint);
      }
      for (uint32_t radius = 2; radius < 50; radius += 5) {
         auto it = occupied_points_by_distance.find(radius);
         if (it == occupied_points_by_distance.end()) {
            //
            // There are no points at this distance.
            //
            mapped_pos.rx() += radius;
            return;
         }
         auto& list = it->second;

         constexpr const float search_angle_step = cobb::degrees_to_radians_mult * 15;
         for (float degrees = 0; degrees < (cobb::degrees_to_radians_mult * 360); degrees += search_angle_step) {
            QPoint search_pos = mapped_pos;
            search_pos.rx() += radius * cos(degrees);
            search_pos.ry() += radius * sin(degrees);
            bool bad = false;
            for (auto& point : list) {
               auto diff = search_pos - point;
               auto dist = sqrt(diff.rx()*diff.rx() + diff.ry()*diff.ry());
               if (dist < minimum_node_distance) {
                  bad = true;
                  break;
               }
            }
            if (!bad) {
               mapped_pos = search_pos;
               return;
            }
         }
      }
      //
      // Welp, we tried. *shrug*
      //
   }
#pragma endregion
#pragma region Overrides
   /*virtual*/ QSize SkillTreeVisualEditor::minimumSizeHint() const /*override*/ {
      return this->sizeHint();
   }
   /*virtual*/ QSize SkillTreeVisualEditor::sizeHint() const /*override*/ {
      return this->_cached.size;
   }

   /*virtual*/ void SkillTreeVisualEditor::contextMenuEvent(QContextMenuEvent* event) /*override*/ {
      auto& menu  = this->_context_menu.menu;
      auto& items = this->_context_menu;

      auto* clicked_node  = this->_get_node_at_point(event->globalPos());
      bool  has_selection = this->_selected_node_id.has_value();

      items.new_perk->setEnabled(clicked_node == nullptr);
      items.remove_perk->setEnabled(has_selection);

      auto* chosen = menu.exec(event->globalPos());
      if (chosen == items.new_perk) {
         auto* created = this->_create_node_at(event->globalPos());
         if (created)
            this->_select_node(created);
      }
   }

   /*virtual*/ void SkillTreeVisualEditor::mousePressEvent(QMouseEvent* event) /*override*/ {
      if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
         return;
      event->setAccepted(true);

      this->_mouse.mousedown_at   = event->localPos().toPoint();
      this->_mouse.mouse_prev_pos = event->screenPos().toPoint();
      this->_mouse.mousedown_on   = this->_get_node_at_point(_map_from_global(event->globalPos()));

      this->_select_node(this->_mouse.mousedown_on);
   }
   /*virtual*/ void SkillTreeVisualEditor::mouseMoveEvent(QMouseEvent* event) /*override*/ {
      bool lmb = event->buttons() & Qt::LeftButton;
      bool mmb = event->buttons() & Qt::MiddleButton;
      if (!lmb && !mmb) {
         this->_update_cursor(event);
         return;
      }
      if (this->_mouse.is_panning) {
         if (!this->_scroll_area)
            return;
         auto pos   = event->screenPos().toPoint();
         auto delta = this->_mouse.mouse_prev_pos - pos; // the order here is not a mistake; panning means that dragging left should scroll right, and vice versa
         this->_mouse.mouse_prev_pos = pos;

         auto* hs = this->_scroll_area->horizontalScrollBar();
         auto* vs = this->_scroll_area->verticalScrollBar();
         hs->setValue(hs->value() + delta.x());
         vs->setValue(vs->value() + delta.y());

         return;
      }
      if (lmb || mmb) {
         this->_mouse.mouse_prev_pos = event->screenPos().toPoint();
         if ((event->pos() - this->_mouse.mousedown_at).manhattanLength() < QApplication::startDragDistance()) {
            this->_update_cursor(event);
            return;
         }
         if (!this->_mouse.mousedown_on) {
            this->_mouse.is_panning = true;
            this->_update_cursor(event);
            return;
         }
      } else if (!lmb) {
         return;
      }

      QDrag*     drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;
      {
         QByteArray  data;
         QDataStream stream(&data, QIODevice::WriteOnly);
         const uint32_t id = this->_mouse.mousedown_on->id;
         stream.writeRawData((const char*)&id, sizeof(id));

         const QString mime_type = QString::fromLatin1(drag_mime_type_for_perk_nodes.data(), drag_mime_type_for_perk_nodes.size());
         mimeData->setData(mime_type, data);
      }
      drag->setMimeData(mimeData);

      Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
   }
   /*virtual*/ void SkillTreeVisualEditor::mouseReleaseEvent(QMouseEvent* event) /*override*/ {
      bool was_panning = this->_mouse.is_panning;
      this->_mouse.is_panning   = false;
      this->_mouse.mousedown_at = {};
      this->_mouse.mousedown_on = {};
      if (was_panning) {
         this->_update_cursor(event);
      }
   }

   #pragma region Drag and drop
      static bool _extract_dragged_action_data(const QMimeData& mime_data, SkillTreeVisualEditor::node_id& dst_id) {
         const QString mime_type = QString::fromLatin1(drag_mime_type_for_perk_nodes.data(), drag_mime_type_for_perk_nodes.size());
         if (!mime_data.hasFormat(mime_type))
            return false;
         QByteArray  bytes = mime_data.data(mime_type);
         QDataStream stream(&bytes, QIODevice::ReadOnly);
         stream.readRawData((char*)&dst_id, sizeof(dst_id));
         return true;
      }

      void SkillTreeVisualEditor::dragEnterEvent(QDragEnterEvent* event) {
         const QString mime_type = QString::fromLatin1(drag_mime_type_for_perk_nodes.data(), drag_mime_type_for_perk_nodes.size());
         if (event->source() != this)
            return;
         if (!event->mimeData()->hasFormat(mime_type))
            return;

         // We'll do further filtering in real-time as the mouse moves, via dragMoveEvent.
         event->acceptProposedAction();
      }
      void SkillTreeVisualEditor::dragMoveEvent(QDragMoveEvent* event) {
         node_id node_id;
         if (!_extract_dragged_action_data(*event->mimeData(), node_id)) {
            event->setDropAction(Qt::IgnoreAction);
            event->accept();
            return;
         }

         auto orig_pos = event->pos();
         if (
            orig_pos.rx() < this->_style.margins.left() ||
            orig_pos.ry() < this->_style.margins.top() ||
            orig_pos.rx() >= this->_cached.size.width() - this->_style.margins.right() ||
            orig_pos.ry() >= this->_cached.size.height() - this->_style.margins.bottom()
         ) {
            //
            // Don't allow moving nodes into the margins.
            //
            event->setDropAction(Qt::IgnoreAction);
            event->accept();
            return;
         }

         auto drag_pos = _map_from_global(orig_pos);
         for (const auto& node_ptr : this->_nodes) {
            bool overlaps = false;
            {
               auto diff     = drag_pos - node_ptr->_cached.centerpoint;
               auto distance = sqrt(diff.rx() * diff.rx() + diff.ry() * diff.ry());
               overlaps = (distance < minimum_node_distance);
            }
            if (!overlaps)
               continue;

            if (node_ptr->id == node_id) {
               //
               // Always allow returning a node to where you dragged it from.
               //
               event->acceptProposedAction();
               return;
            } else {
               //
               // Don't allow putting nodes too close to each other.
               //
               event->setDropAction(Qt::IgnoreAction);
               event->accept();
               return;
            }
         }
         event->acceptProposedAction();
      }
      void SkillTreeVisualEditor::dropEvent(QDropEvent* event) {
         if (event->source() != this && !(event->possibleActions() & Qt::MoveAction))
            return;
         if (event->proposedAction() == Qt::MoveAction) {
            node_id node_id;
            _extract_dragged_action_data(*event->mimeData(), node_id);

            auto* node = this->_node_by_id(node_id);
            if (!node)
               return;

            auto drag_pos = _map_from_global(event->pos());
            //
            // TODO: Do we want to allow drops onto pre-existing nodes (currently blocked in 
            //       the dragModeEvent) and just reposition the node at drop time? If so, we 
            //       should adjust `drag_pos` using `_find_available_node_position`, passing 
            //       the ID of the currently-dragged node as the "node ID to ignore."
            //

            node->data.position.grid.x = drag_pos.x() / grid_cell_w;
            node->data.position.grid.y = drag_pos.x() / grid_cell_h;
            node->data.position.offset.x = (drag_pos.x() % grid_cell_w) / offset_factor_x;
            node->data.position.offset.y = (drag_pos.y() % grid_cell_h) / offset_factor_y;

            this->_update_geometry();
            this->update();
         }
      }
   #pragma endregion

   /*virtual*/ void SkillTreeVisualEditor::paintEvent(QPaintEvent* event) /*override*/ {
      QPainter painter(this);
      painter.translate(this->_style.margins.left(), this->_style.margins.top());
      painter.scale(this->_zoom, this->_zoom);

      const auto canvas_w = this->_cached.row_count * grid_cell_w;
      const auto canvas_h = this->_cached.col_count * grid_cell_h;

      #pragma region Gridlines
         painter.save();
         painter.setBrush(QBrush{});
         {
            const int top    = -(this->_style.margins.top() / 2);
            const int bottom = canvas_h + (this->_style.margins.bottom() / 2);
            const int left   = -(this->_style.margins.left() / 2);
            const int right  = canvas_w + (this->_style.margins.right() / 2);
            {
               QPen pen;
               pen.setWidth(this->_style.gridline.thickness);
               if (this->_style.gridline.thickness == 1)
                  pen.setCosmetic(true);
               painter.setPen(pen);
            }
            {
               painter.save();
               for (uint32_t i = 0; i < this->_cached.row_count; ++i) {
                  painter.translate(grid_cell_w, 0);
                  painter.drawLine(0, top, 0, bottom);
               }
               painter.restore();
            }
            {
               painter.save();
               for (uint32_t i = 0; i < this->_cached.col_count; ++i) {
                  painter.translate(0, grid_cell_h);
                  painter.drawLine(left, 0, right, 0);
               }
               painter.restore();
            }
            painter.drawRect(left, top, right - left, bottom - top);
         }
         painter.restore();
      #pragma endregion
      #pragma region Connecting lines for perks
         painter.save();
         for (auto& src_ptr : this->_nodes) {
            if (node_is_invisible(*src_ptr))
               continue;
            auto& conn = src_ptr->connects_to;
            if (conn.empty())
               continue;

            const auto src_point = src_ptr->_cached.centerpoint;

            for (auto& dst_ptr : this->_nodes) {
               if (node_is_invisible(*dst_ptr))
                  continue;
               uint32_t dst_id = dst_ptr->id;
               bool     found  = false;
               for (auto id : conn) {
                  if (id == dst_id) {
                     found = true;
                     break;
                  }
               }
               if (!found)
                  continue;

               const auto  dst_point = dst_ptr->_cached.centerpoint;
               const auto& style     = (dst_ptr->data.parent_required) ? this->_style.connectors.required : this->_style.connectors.optional;
               {
                  QPen pen;
                  pen.setColor(style.stem.color);
                  pen.setWidth(style.stem.thickness);
                  if (style.stem.thickness == 1)
                     pen.setCosmetic(true);
                  painter.setPen(pen);
               }
               painter.drawLine(src_point, dst_point);
               painter.setBrush(style.head.fill);
               {
                  QPen pen;
                  pen.setColor(style.head.line.color);
                  pen.setWidth(style.head.line.thickness);
                  if (style.head.line.thickness == 1)
                     pen.setCosmetic(true);
                  painter.setPen(pen);
               }
               {
                  constexpr const float arrow_angle = cobb::degrees_to_radians_mult * 22.5F;
                  constexpr const float arrow_cos   = cobb::cosine(arrow_angle);
                  constexpr const float arrow_sin   = cobb::sine(arrow_angle);

                  float dx  = dst_point.x() - src_point.x();
                  float dy  = dst_point.y() - src_point.y();
                  if (dx && dy) {
                     float mag = sqrt(dx * dx + dy * dy);
                     dx /= mag;
                     dy /= mag;
                  } else {
                     dx = 0;
                     dy = 1;
                  }
                  const QPointF points[] = {
                     dst_point,
                     { dst_point.x() + (dx*arrow_cos - dy*arrow_sin), dst_point.y() + (dy*arrow_cos - dx*arrow_sin) },
                     { dst_point.x() + (dx*arrow_cos + dy*arrow_sin), dst_point.y() + (dy*arrow_cos - dx*arrow_sin) },
                  };
                  painter.drawPolygon(points, std::extent<decltype(points)>::value);
               }
            }
         }
         painter.restore();
      #pragma endregion
      #pragma region Points, with the selected point last
         painter.save();
         {
            auto _set_painter_circle_style = [&painter](const NodeStyle& style) {
               painter.setBrush(style.fill);
               {
                  QPen pen;
                  pen.setColor(style.line.color);
                  pen.setWidth(style.line.thickness);
                  if (style.line.thickness == 1)
                     pen.setCosmetic(true);
                  painter.setPen(pen);
               }
            };
            auto _set_painter_text_style = [this, &painter](const NodeStyle& style) {
               QFont font = style.text.font;
               font = font.resolve(this->font());
               painter.setFont(font);

               QPen pen;
               pen.setColor(style.text.color);
               painter.setPen(pen);
            };

            QTextOption text_option;
            text_option.setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignTop);

            const PerkNode* selected = nullptr;
            {
               const auto& style = this->_style.nodes.general;

               _set_painter_text_style(style);
               for (auto& src_ptr : this->_nodes) {
                  if (node_is_invisible(*src_ptr))
                     continue;
                  if (this->_selected_node_id.has_value() && src_ptr->id == this->_selected_node_id.value()) {
                     selected = src_ptr.get();
                     continue;
                  }
                  auto point = src_ptr->_cached.centerpoint;
                  point.ry() += style.text.distance;
                  painter.drawText(point.x(), point.y(), src_ptr->_cached.perk_editor_id);
               }

               _set_painter_circle_style(style);
               for (auto& src_ptr : this->_nodes) {
                  if (this->_selected_node_id.has_value() && src_ptr->id == this->_selected_node_id.value()) {
                     continue;
                  }
                  const QPointF point = src_ptr->_cached.centerpoint;
                  painter.drawEllipse(point, (qreal)style.radius, (qreal)style.radius);
               }
            }
            if (selected) {
               const auto& style = this->_style.nodes.selected;

               _set_painter_text_style(style);
               {
                  auto point = selected->_cached.centerpoint;
                  point.ry() += style.text.distance;
                  painter.drawText(point.x(), point.y(), selected->_cached.perk_editor_id);
               }

               _set_painter_circle_style(style);
               painter.drawEllipse(selected->_cached.centerpoint, (qreal)style.radius, (qreal)style.radius);
            }
         }
         painter.restore();
      #pragma endregion
   }
#pragma endregion

QPoint SkillTreeVisualEditor::_map_from_global(const QPoint& global_pos) {
   auto local = this->mapFromGlobal(global_pos);
   local.rx() -= this->_style.margins.left();
   local.ry() -= this->_style.margins.top();
   return local / this->_zoom;
}

SkillTreeVisualEditor::PerkNode* SkillTreeVisualEditor::_get_node_at_point(const QPoint& local_pos) {
   //
   // Prefer direct clicks on the nodes:
   //
   for (auto& node_ptr : this->_nodes) {
      if (node_is_invisible(*node_ptr))
         continue;
      float radius = this->_style.nodes.general.radius;
      if (this->_selected_node_id.has_value() && node_ptr->id == this->_selected_node_id.value())
         radius = this->_style.nodes.selected.radius;

      QPointF gap    = node_ptr->_cached.centerpoint - local_pos;
      float   length = sqrt(gap.rx() * gap.rx() + gap.ry() * gap.ry());
      if (length <= radius) {
         return node_ptr.get();
      }
   }
   //
   // TODO: Do we want to allow clicks on node text, too? If so, loop over 
   // the nodes again and check their text this time.
   //
   return nullptr;
}

QPointF SkillTreeVisualEditor::_node_centerpoint(const PerkNode& node) const {
   QPointF out;
   out.rx() = node.data.position.grid.x * grid_cell_w + node.data.position.offset.x * offset_factor_x;
   out.ry() = node.data.position.grid.y * grid_cell_h + node.data.position.offset.y * offset_factor_y;
   return out;
}

void SkillTreeVisualEditor::_update_cursor(const QMouseEvent* event) {
   if (this->_mouse.is_panning) {
      this->setCursor(Qt::CursorShape::ClosedHandCursor);
      return;
   }
   if (this->_mouse.mousedown_on) {
      this->setCursor(Qt::CursorShape::ArrowCursor);
      return;
   }
   {
      auto pos = _map_from_global(event->globalPos());
      if (_get_node_at_point(pos)) {
         this->setCursor(Qt::CursorShape::ArrowCursor);
         return;
      }
   }
   this->setCursor(Qt::CursorShape::OpenHandCursor);
}
void SkillTreeVisualEditor::_update_geometry() {
   this->_cached = {};

   float max_radius = std::max(
      this->_style.nodes.general.radius,
      this->_style.nodes.selected.radius
   );
   float max_text_distance = std::max(
      this->_style.nodes.general.text.distance,
      this->_style.nodes.selected.text.distance
   );
   int max_text_size = this->font().pixelSize();
   {
      QFont widget_font = this->font();
      QFont node_g_font = this->_style.nodes.general.text.font;
      QFont node_s_font = this->_style.nodes.selected.text.font;
      node_g_font = node_g_font.resolve(widget_font);
      node_s_font = node_s_font.resolve(widget_font);
      max_text_size = std::max(max_text_size, node_g_font.pixelSize());
      max_text_size = std::max(max_text_size, node_s_font.pixelSize());
   }

   const auto font_metrics = this->fontMetrics();

   int max_x = 0;
   int max_y = 0;
   for (auto& node_ptr : this->_nodes) {
      if (node_is_invisible(*node_ptr))
         continue;
      node_ptr->_cached.centerpoint = _node_centerpoint(*node_ptr);

      this->_cached.col_count = std::max(this->_cached.col_count, node_ptr->data.position.grid.x);
      this->_cached.row_count = std::max(this->_cached.row_count, node_ptr->data.position.grid.y);

      QPointF point = node_ptr->_cached.centerpoint;
      point.ry() += max_radius + max_text_distance + max_text_size;
      max_x = std::max(max_x, (int)point.x());
      max_y = std::max(max_y, (int)point.y());

      max_x += font_metrics.horizontalAdvance(node_ptr->_cached.perk_editor_id) / 2;
   }
   ++this->_cached.col_count;
   ++this->_cached.row_count;
   this->_cached.size = QSize(
      max_x + this->_style.margins.left() + this->_style.margins.right(),
      max_y + this->_style.margins.top() + this->_style.margins.bottom()
   );

   this->updateGeometry();
}

void SkillTreeVisualEditor::_select_node(const PerkNode* node) {
   const optional_node_id prior = this->_selected_node_id;
   bool selection_changed = false;
   if (node) {
      this->_selected_node_id = node->id;
   } else {
      this->_selected_node_id.reset();
   }
   if (prior != this->_selected_node_id) {
      emit selectionChanged(this->_selected_node_id, prior);
      this->repaint();
   }
}