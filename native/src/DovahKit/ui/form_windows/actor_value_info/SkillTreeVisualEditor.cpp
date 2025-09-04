#include "./SkillTreeVisualEditor.h"
#include <cassert>
#include <QContextMenuEvent>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include "helpers/bitset.h"
#include "helpers/math/rotation/unit_conversion.h"
#include "helpers/math/cosine.h"
#include "helpers/math/sine.h"
#include "dovah/forms/ActorValueInfo.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#pragma region Panning
   #include <QApplication>
   #include <QScrollBar>
#pragma endregion

#pragma region SkillTreeVisualEditor::PerkNode
   bool SkillTreeVisualEditor::PerkNode::has_outbound_connection_to(const PerkNode& dst) const {
      for (auto id : this->connects_to)
         if (id == dst.id)
            return true;
      return false;
   }
#pragma endregion

SkillTreeVisualEditor::SkillTreeVisualEditor(QWidget* parent) : QWidget(parent) {
   this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
   this->setSizePolicy({ QSizePolicy::Minimum, QSizePolicy::Minimum });

   this->setAcceptDrops(true);
   this->setMouseTracking(true); // to update the cursor depending on what drawn boxes it's over

   #pragma region Styles
      this->_style.advisory_level_labels.color = QColor(160, 160, 160);
      this->_style.background = QColor(255, 255, 255);
      {
         auto& style = this->_style.connectors.required;
         style.head.line.color = style.stem.color = QColor(0, 0, 0);
         style.head.fill = QColor(0, 0, 0);
      }
      {
         auto& style = this->_style.connectors.optional;
         style.head.line.color = style.stem.color = QColor(0, 140, 255);
         style.head.fill = QColor(0, 140, 255);
      }
      this->_style.gridline.color = QColor(160, 160, 160);
      {
         auto& style = this->_style.nodes.general;
         style.fill = QColor(128, 128, 128);
         style.text.stroke.color = this->_style.background;
      }
      {
         auto& style = this->_style.nodes.selected;
         style.fill = style.line.color = style.text.color = QColor(255, 0, 0);
         style.text.stroke.color = this->_style.background;
      }
      {
         auto& style = this->_style.nodes.root;
         style.text.color = QColor(255, 255, 255);
         style.text.stroke.thickness = 0;
      }
      this->_style.margins = { 20, 20, 20, 20 };
   #pragma endregion
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
                  if (node_ptr->id == this->_connecting.source_node) {
                     this->_cancel_node_connection_interaction();
                  }
                  if (node_ptr.get() == this->_mouse.mousedown_on) {
                     this->_mouse.mousedown_on = nullptr;
                     this->_mouse.is_cancelled = true;
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
                     return node_ptr->data.perk == nullptr && node_ptr->id != 0; // don't delete the root node
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
         auto* action = actions.new_connection = new QAction(tr("Connect to other perk..."));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            auto* node = this->_context_menu.target;
            if (!node) {
               if (!this->_selected_node_id.has_value())
                  return;
               node = this->_node_by_id(this->_selected_node_id.value());
               if (!node)
                  return;
            }
            this->_start_node_connection_interaction(*node, true);
         });
      }
      {
         auto* action = actions.sever_connection = new QAction(tr("Disconnect from other perk..."));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            auto* node = this->_context_menu.target;
            if (!node) {
               if (!this->_selected_node_id.has_value())
                  return;
               node = this->_node_by_id(this->_selected_node_id.value());
               if (!node)
                  return;
            }
            this->_start_node_connection_interaction(*node, false);
         });
      }
      {
         auto* action = actions.snap_to_grid = new QAction(tr("Snap to grid"));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            auto* node = this->_context_menu.target;
            if (!node) {
               if (!this->_selected_node_id.has_value())
                  return;
               node = this->_node_by_id(this->_selected_node_id.value());
               if (!node)
                  return;
            }
            if (this->_node_is_root(*node))
               return;
            node->data.position.offset = { 0, 0 };
            emit nodePositionChanged(node->id, node->data.position);
            this->_update_node_geometry(*node);
            this->update();
         });
      }
      {
         auto* action = actions.remove_perk = new QAction(tr("Remove perk"));
         menu.addAction(action);
         QObject::connect(action, &QAction::triggered, this, [this]() {
            auto* node = this->_context_menu.target;
            if (!node) {
               if (!this->_selected_node_id.has_value())
                  return;
               node = this->_node_by_id(this->_selected_node_id.value());
               if (!node)
                  return;
            }
            this->_delete_node(*node);
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
      //
      bool root_node_exists = false;
      //
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

         if (dst_node.id == 0)
            root_node_exists = true;
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
      if (!root_node_exists) {
         auto& dst_node_ptr = dst_list.emplace_back(std::make_unique<PerkNode>());
         auto& dst_node     = *dst_node_ptr;
         dst_node.id = 0;
         dst_node.data.parent_required = false;
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

   bool SkillTreeVisualEditor::nodeIsRoot(node_id id) const {
      return id == 0;
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
      this->_update_node_geometry(*node);
      this->update();
   }
#pragma endregion
#pragma region Data (protected)
   bool SkillTreeVisualEditor::_node_is_root(const PerkNode& node) const {
      return node.id == 0;
   }

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
   void SkillTreeVisualEditor::_delete_node(PerkNode& node) {
      assert(!_node_is_root(node));
      const auto node_id = node.id;

      auto&  list = this->_nodes;
      size_t i;
      {
         auto it = std::find_if(list.begin(), list.end(), [&node](const auto& uniq) { return uniq.get() == &node; });
         if (it == list.end())
            return;
         i = std::distance(list.begin(), it);
      }
      //
      // Disconnect from all other nodes, and from anything in the widget 
      // that might be tracking it.
      //
      for (size_t j = 0; j < list.size(); ++j) {
         if (j == i)
            continue;
         std::erase(list[j]->connects_to, node_id);
      }
      bool selection_changed = false;
      if (node_id == this->_selected_node_id) {
         this->_selected_node_id = {};
         selection_changed = true;
      }
      if (this->_context_menu.target == &node)
         this->_context_menu.target = nullptr;
      if (this->_mouse.mousedown_on == &node) {
         this->_mouse.mousedown_on = nullptr;
         this->_mouse.is_cancelled = true;
      }
      //
      // Erase the node from the list, implicitly destroying it (since it's 
      // in a std::unique_ptr).
      //
      list.erase(list.begin() + i);
      //
      // Update canvas.
      //
      this->_update_geometry();
      this->update();
      if (selection_changed)
         emit selectionChanged({}, node_id);
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

   void SkillTreeVisualEditor::_find_available_node_position(QPoint& mapped_pos, optional_node_id node_id_to_ignore) {
      constexpr auto _dist = [](const QPointF& a, const QPoint& b) -> float {
         QPointF diff = a - b;
         return sqrt(diff.x()*diff.x() + diff.y()*diff.y());
      };

      auto _has_overlap = [this, &node_id_to_ignore](const QPoint& pos) -> bool {
         for (const auto& node_ptr : this->_nodes) {
            if (node_ptr->id == node_id_to_ignore)
               continue;
            if (_dist(node_ptr->_cached.geometry.centerpoint, pos) < minimum_node_distance)
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
         float distance = _dist(node_ptr->_cached.geometry.centerpoint, mapped_pos);
         occupied_points_by_distance[(uint32_t)distance].push_back(node_ptr->_cached.geometry.centerpoint);
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
      auto size = this->_cached.canvas_size * this->_zoom;
      size.rwidth()  += this->_style.margins.left() + this->_style.margins.right();
      size.rheight() += this->_style.margins.top() + this->_style.margins.bottom();
      return size;
   }

   /*virtual*/ void SkillTreeVisualEditor::contextMenuEvent(QContextMenuEvent* event) /*override*/ {
      static constexpr const bool act_on_selection_only_if_directly_clicked = true;

      auto& menu  = this->_context_menu.menu;
      auto& items = this->_context_menu;

      auto* clicked_node = this->_get_node_at_point(this->_map_from_global(event->globalPos()));

      bool  has_target;
      bool  target_is_not_root;
      if constexpr (act_on_selection_only_if_directly_clicked) {
         has_target         = clicked_node != nullptr;
         target_is_not_root = !clicked_node || !_node_is_root(*clicked_node);
         this->_context_menu.target = clicked_node;
      } else {
         has_target         = this->_selected_node_id.has_value();
         target_is_not_root = !has_target || !this->nodeIsRoot(this->_selected_node_id.value());
         this->_context_menu.target = nullptr;
         if (has_target)
            this->_context_menu.target = _node_by_id(this->_selected_node_id.value());
      }

      items.new_perk->setEnabled(clicked_node == nullptr);
      items.remove_perk->setEnabled(has_target && target_is_not_root);
      items.snap_to_grid->setEnabled(has_target && target_is_not_root);
      items.new_connection->setEnabled(has_target);
      items.sever_connection->setEnabled(has_target);

      auto* chosen = menu.exec(event->globalPos());
      if (chosen == items.new_perk) {
         auto* created = this->_create_node_at(event->globalPos());
         if (created)
            this->_select_node(created);
      }
      this->_context_menu.target = nullptr;
   }

   /*virtual*/ void SkillTreeVisualEditor::focusOutEvent(QFocusEvent* event) /*override*/ {
      this->_cancel_node_connection_interaction();
      if (this->_mouse.is_drag_moving || this->_mouse.is_panning) {
         this->_mouse.is_cancelled = true;
      }
      QWidget::focusOutEvent(event);
   }

   /*virtual*/ void SkillTreeVisualEditor::mousePressEvent(QMouseEvent* event) /*override*/ {
      if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
         return;

      this->_mouse.mousedown_at   = event->localPos().toPoint();
      this->_mouse.mouse_prev_pos = event->screenPos().toPoint();
      this->_mouse.is_cancelled   = false;

      if (this->_is_in_node_connection_interaction()) {
         auto* dst = this->_get_node_at_point(_map_from_global(event->globalPos()));
         this->_complete_node_connection_interaction(dst);
         return;
      }

      event->setAccepted(true);

      this->_mouse.mousedown_on = this->_get_node_at_point(_map_from_global(event->globalPos()));
      if (this->_mouse.mousedown_on)
         this->_select_node(this->_mouse.mousedown_on);
   }
   /*virtual*/ void SkillTreeVisualEditor::mouseMoveEvent(QMouseEvent* event) /*override*/ {
      bool lmb = event->buttons() & Qt::LeftButton;
      bool mmb = event->buttons() & Qt::MiddleButton;
      if (!lmb && !mmb || this->_mouse.is_cancelled) {
         this->_update_cursor(event);
         return;
      }

      if (this->_mouse.is_drag_moving) {
         auto pos   = event->screenPos().toPoint();
         auto delta = this->_mouse.mouse_prev_pos - pos; // the order here is not a mistake; panning means that dragging left should scroll right, and vice versa
         this->_mouse.mouse_prev_pos = pos;
         if (this->_mouse.mousedown_on) {
            auto mapped = _map_from_global(pos);
            this->_set_node_position(*this->_mouse.mousedown_on, mapped);
         }
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
         if (!lmb || !this->_mouse.mousedown_on) {
            this->_mouse.is_panning = true;
            this->_update_cursor(event);
            return;
         }
         if (lmb) {
            this->_mouse.is_drag_moving = true;
            this->_update_cursor(event);
         }
      }
   }
   /*virtual*/ void SkillTreeVisualEditor::mouseReleaseEvent(QMouseEvent* event) /*override*/ {
      bool needs_cursor_update = this->_mouse.is_panning || this->_mouse.is_drag_moving;
      this->_mouse.mousedown_at   = {};
      this->_mouse.mousedown_on   = {};
      this->_mouse.is_drag_moving = false;
      this->_mouse.is_panning     = false;
      this->_mouse.is_cancelled   = false;
      if (needs_cursor_update) {
         this->_update_cursor(event);
      }
   }

   #pragma region Override helpers: paintEvent
      void SkillTreeVisualEditor::_prepare_painter_for_style(QPainter& painter, const LineStyle& style) {
         QPen pen;
         if (style.thickness != 0) {
            pen.setColor(style.color);
            pen.setWidth(style.thickness);
            if (style.thickness == 1 && this->_zoom < 1)
               pen.setCosmetic(true);
         }
         painter.setPen(pen);
      }
      void SkillTreeVisualEditor::_prepare_painter_for_style(QPainter& painter, const ShapeStyle& style) {
         painter.setBrush(style.fill);
         this->_prepare_painter_for_style(painter, style.line);
      }
      void SkillTreeVisualEditor::_prepare_painter_for_style(QPainter& painter, const TextStyle& style) {
         QFont font = style.font.resolve(this->font());
         painter.setFont(font);

         painter.setBrush(style.color);
         this->_prepare_painter_for_style(painter, style.stroke);
      }
      void SkillTreeVisualEditor::_prepare_painter_for_style(QPainter& painter, const TextStyle& child, const TextStyle& parent) {
         QFont font = child.font.resolve(parent.font).resolve(this->font());
         painter.setFont(font);

         if (child.color.isValid())
            painter.setBrush(child.color);
         else
            painter.setBrush(parent.color);

         this->_prepare_painter_for_style(painter, child.stroke);
      }
      void SkillTreeVisualEditor::_draw_single_line_text(QPainter& painter, QPointF at, Qt::Alignment align, QString text) {
         const auto font_metrics = painter.fontMetrics();
         if (align & (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignRight)) {
            QRect bounds = font_metrics.boundingRect(text);
            if (align & Qt::AlignmentFlag::AlignHCenter) {
               at.rx() -= bounds.width() / 2;
            } else if (align & Qt::AlignmentFlag::AlignRight) {
               at.rx() -= bounds.width();
            }
         }
         if (align & Qt::AlignmentFlag::AlignTop) {
            at.ry() += font_metrics.ascent();
         } else if (align & Qt::AlignmentFlag::AlignBottom) {
            at.ry() -= font_metrics.descent();
         }

         QPainterPath path;
         path.addText(
            at.x(),
            at.y(),
            painter.font(),
            text
         );
         painter.strokePath(path, painter.pen());
         painter.fillPath(path, painter.brush());
      }
      void SkillTreeVisualEditor::_draw_multiline_text(QPainter& painter, const QPointF& at, Qt::Alignment align, QString text) {
         QPainterPath path;

         const auto font_metrics = painter.fontMetrics();
         const auto line_height  = font_metrics.ascent() + font_metrics.descent();
         const auto line_count   = text.count('\n') + 1;

         float y = at.y();
         if (align & Qt::AlignmentFlag::AlignTop) {
            y += font_metrics.ascent();
         } else if (align & Qt::AlignmentFlag::AlignBottom) {
            y -= font_metrics.descent();
            y -= (line_height + font_metrics.leading()) * (line_count - 1);
         } else if (align & Qt::AlignmentFlag::AlignVCenter) {
            y -= font_metrics.ascent();

            int text_block_height = (line_height * line_count) + (font_metrics.leading() * (line_count - 1));
            y += text_block_height / 2;
         }

         const auto distance_per_line = line_height + font_metrics.leading();
         int from  = 0;
         int until = text.indexOf('\n', from);
         do {
            if (until == from) {
               do {
                  y    += distance_per_line;
                  from  = until + 1;
                  until = text.indexOf('\n', from);
                  if (until < 0)
                     break;
               } while (until == from);
               if (until < 0)
                  break;
            }
            QString fragment;
            if (until < 0)
               fragment = text.mid(from);
            else
               fragment = text.mid(from, until - from);

            float x = at.x();
            if (align & (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignRight)) {
               QRect bounds = font_metrics.boundingRect(fragment);
               if (align & Qt::AlignmentFlag::AlignHCenter) {
                  x -= bounds.width() / 2;
               } else if (align & Qt::AlignmentFlag::AlignRight) {
                  x -= bounds.width();
               }
            }
            path.addText(x, y, painter.font(), fragment);
            y += distance_per_line;

            if (until < 0)
               break;
            from  = until + 1;
            until = text.indexOf('\n', from);
         } while (true);

         painter.strokePath(path, painter.pen());
         painter.fillPath(path, painter.brush());
      }
      void SkillTreeVisualEditor::_draw_gridlines(QPainter& painter) {
         const auto canvas_w = this->_cached.row_count * grid_cell_w;
         const auto canvas_h = this->_cached.col_count * grid_cell_h;

         painter.save();
         painter.setBrush(QBrush{});
         {
            const int top    = 0;
            const int bottom = canvas_h;
            const int left   = 0;
            const int right  = canvas_w;
            _prepare_painter_for_style(painter, this->_style.gridline);
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
      }
      void SkillTreeVisualEditor::_draw_connection(QPainter& painter, const PerkNode& src, const PerkNode& dst) {
         const auto& src_point = src._cached.geometry.centerpoint;
         const auto  dst_point  = dst._cached.geometry.centerpoint;
         const auto& style      = (dst.data.parent_required) ? this->_style.connectors.required : this->_style.connectors.optional;
         const auto& node_style = (this->_selected_node_id == dst.id) ? this->_style.nodes.selected : this->_style.nodes.general;
         _prepare_painter_for_style(painter, style.stem);
         painter.drawLine(src_point, dst_point);
         painter.setBrush(style.head.fill);
         _prepare_painter_for_style(painter, style.head.line);
         {
            constexpr const float arrow_angle = cobb::degrees_to_radians_mult * 22.5F;
            constexpr const float arrow_cos   = cobb::cosine(arrow_angle);
            constexpr const float arrow_sin   = cobb::sine(arrow_angle);

            QPointF direction = (dst_point - src_point);
            if (direction.x() || direction.y()) {
               float mag = sqrt(direction.x()*direction.x() + direction.y()*direction.y());
               direction /= mag;
            } else {
               direction = { 0, 1 };
            }
            const auto x_cos = direction.x() * arrow_cos;
            const auto x_sin = direction.x() * arrow_sin;
            const auto y_cos = direction.y() * arrow_cos;
            const auto y_sin = direction.y() * arrow_sin;

            const auto arrow_head_pos = dst_point - (direction * node_style.radius);

            const QPointF points[] = {
               arrow_head_pos,
               { arrow_head_pos.x() - (x_cos - y_sin)*style.head.length, arrow_head_pos.y() - (y_cos + x_sin)*style.head.length },
               { arrow_head_pos.x() - (x_cos + y_sin)*style.head.length, arrow_head_pos.y() - (y_cos - x_sin)*style.head.length },
            };
            painter.drawPolygon(points, std::extent<decltype(points)>::value);
         }
      }
      void SkillTreeVisualEditor::_draw_node_label(QPainter& painter, const PerkNode& node, const NodeStyle& style) {
         auto point = node._cached.geometry.centerpoint;
         point.ry() += style.text.distance;

         this->_draw_single_line_text(
            painter,
            point,
            Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignHCenter,
            node._cached.perk_editor_id
         );
      }
      void SkillTreeVisualEditor::_draw_node_shape(QPainter& painter, const PerkNode& node, const NodeStyle& style) {
         painter.drawEllipse(node._cached.geometry.centerpoint, (qreal)style.radius, (qreal)style.radius);
      }
   #pragma endregion
   /*virtual*/ void SkillTreeVisualEditor::paintEvent(QPaintEvent* event) /*override*/ {
      QPainter painter(this);
      painter.fillRect(painter.window(), this->_style.background);

      painter.translate(this->_style.margins.left(), this->_style.margins.top());
      painter.scale(this->_zoom, this->_zoom);

      if (this->_style.advisory_level_labels.visible) {
         const auto& style        = this->_style.advisory_level_labels;
         const auto  font         = style.font.resolve(this->font());
         const auto  font_metrics = QFontMetrics(font);

         painter.save();
         painter.setRenderHint(QPainter::Antialiasing);
         painter.setFont(font);
         {
            QPen pen;
            pen.setColor(style.color);
            painter.setPen(pen);
         }
         for (size_t i = 0; i < 5; ++i) {
            int level = i * 25;
            
            QString text = tr("Lv. %1").arg(level);
            auto    rect = font_metrics.boundingRect(text);
            painter.drawText(
               style.indent,
               (i * grid_cell_h) + (grid_cell_h / 2) - (rect.height() / 2),
               style.width - style.indent,
               rect.height(),
               {},
               text,
               nullptr
            );
         }
         painter.restore();
         painter.translate(style.width, 0);
      }

      this->_draw_gridlines(painter);
      painter.setRenderHint(QPainter::Antialiasing);
      #pragma region Connecting lines for perks
         painter.save();
         for (auto& src_ptr : this->_nodes) {
            if (src_ptr->connects_to.empty())
               continue;
            for (auto& dst_ptr : this->_nodes) {
               if (_node_is_root(*dst_ptr))
                  continue;
               if (!src_ptr->has_outbound_connection_to(*dst_ptr))
                  continue;
               this->_draw_connection(painter, *src_ptr, *dst_ptr);
            }
         }
         painter.restore();
      #pragma endregion
      #pragma region Root perk
      {
         auto* node = this->_node_by_id(0);
         if (node) {
            const auto& base_node_style = (this->_selected_node_id == 0) ? this->_style.nodes.selected : this->_style.nodes.general;
            const auto& root_node_style = this->_style.nodes.root;

            this->_prepare_painter_for_style(painter, base_node_style);
            const auto& box = node->_cached.geometry.box;
            painter.drawRoundedRect(box, root_node_style.corner_radius, root_node_style.corner_radius);

            this->_prepare_painter_for_style(painter, root_node_style.text, base_node_style.text);
            this->_draw_multiline_text(
               painter,
               node->_cached.geometry.centerpoint,
               Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignVCenter,
               tr("Root\nNode")
            );
         }
      }
      #pragma endregion
      #pragma region Points, with the selected point last
         painter.save();
         {
            const PerkNode* selected = nullptr;
            {
               const auto& style = this->_style.nodes.general;

               this->_prepare_painter_for_style(painter, style.text);
               for (auto& src_ptr : this->_nodes) {
                  if (_node_is_root(*src_ptr))
                     continue;
                  if (this->_selected_node_id.has_value() && src_ptr->id == this->_selected_node_id.value()) {
                     selected = src_ptr.get();
                     continue;
                  }
                  this->_draw_node_label(painter, *src_ptr, style);
               }

               this->_prepare_painter_for_style(painter, style);
               for (auto& src_ptr : this->_nodes) {
                  if (_node_is_root(*src_ptr))
                     continue;
                  if (this->_selected_node_id.has_value() && src_ptr->id == this->_selected_node_id.value())
                     continue;
                  this->_draw_node_shape(painter, *src_ptr, style);
               }
            }
            if (selected) {
               const auto& style = this->_style.nodes.selected;

               this->_prepare_painter_for_style(painter, style.text);
               this->_draw_node_label(painter, *selected, style);

               this->_prepare_painter_for_style(painter, style);
               this->_draw_node_shape(painter, *selected, style);
            }
         }
         painter.restore();
      #pragma endregion
   }
#pragma endregion
#pragma region Node interactions
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
   #pragma region Connect/disconnect nodes
      bool SkillTreeVisualEditor::_is_in_node_connection_interaction() const {
         return this->_connecting.source_node.has_value();
      }
      void SkillTreeVisualEditor::_start_node_connection_interaction(const PerkNode& node, bool connecting) {
         this->_connecting.source_node   = node.id;
         this->_connecting.is_connecting = connecting;
      }
      void SkillTreeVisualEditor::_cancel_node_connection_interaction() {
         this->_connecting.source_node.reset();
      }
      void SkillTreeVisualEditor::_complete_node_connection_interaction(PerkNode* dst) {
         if (!this->_is_in_node_connection_interaction())
            return;
         bool  changed = false;
         auto* src     = this->_node_by_id(this->_connecting.source_node.value());
         if (src && dst) {
            if (this->_connecting.is_connecting) {
               if (!_node_is_root(*dst)) {
                  for (auto id : src->connects_to)
                     if (id == dst->id)
                        return;
                  src->connects_to.push_back(dst->id);
                  changed = true;
               }
            } else {
               if (std::erase(src->connects_to, dst->id) != 0)
                  changed = true;
               //
               // Make it so that disconnection is bi-directional. If you right-click one node, 
               // choose "disconnect," and pick another node, then we shouldn't care which node 
               // the connection is outbound from.
               //
               if (std::erase(dst->connects_to, src->id) != 0)
                  changed = true;
            }
         }
         this->_connecting.source_node.reset();
         if (changed) {
            this->repaint();
         }
      }
   #pragma endregion
   void SkillTreeVisualEditor::_set_node_position(PerkNode& node, QPointF canvas_relative) {
      //
      // Snap to nearest gridline, first.
      //
      node.data.position.grid.x = std::max<uint32_t>(0, (canvas_relative.x() - (grid_cell_w / 2)) / grid_cell_w);
      node.data.position.grid.y = std::max<uint32_t>(0, (canvas_relative.y() - (grid_cell_h / 2)) / grid_cell_h);
      if constexpr (maximum_row_count > 0) {
         auto& coord = node.data.position.grid.y;
         if (coord >= maximum_row_count)
            coord = maximum_row_count - 1;
      }
      //
      // Now set offset.
      //
      float ox = canvas_relative.x() - (node.data.position.grid.x * grid_cell_w);
      float oy = canvas_relative.y() - (node.data.position.grid.y * grid_cell_h);
      node.data.position.offset.x = ox / offset_factor_x;
      node.data.position.offset.y = oy / offset_factor_y;
      //
      // Done!
      //
      emit nodePositionChanged(node.id, node.data.position);
      this->_update_geometry();
      this->repaint();
   }
#pragma endregion

QPoint SkillTreeVisualEditor::_map_from_global(const QPoint& global_pos) {
   auto local = this->mapFromGlobal(global_pos);
   local.rx() -= this->_style.margins.left();
   local.ry() -= this->_style.margins.top();
   local /= this->_zoom;
   if (this->_style.advisory_level_labels.visible)
      local.rx() -= this->_style.advisory_level_labels.width;
   return local;
}

SkillTreeVisualEditor::PerkNode* SkillTreeVisualEditor::_get_node_at_point(const QPoint& local_pos) {
   //
   // Prefer direct clicks on the nodes:
   //
   for (auto& node_ptr : this->_nodes) {
      if (_node_is_root(*node_ptr)) {
         if (node_ptr->_cached.geometry.box.contains(local_pos))
            return node_ptr.get();
         continue;
      }
      float radius = this->_style.nodes.general.radius;
      if (this->_selected_node_id.has_value() && node_ptr->id == this->_selected_node_id.value())
         radius = this->_style.nodes.selected.radius;

      QPointF gap    = node_ptr->_cached.geometry.centerpoint - local_pos;
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
   if (this->_is_in_node_connection_interaction()) {
      this->setCursor(Qt::CursorShape::ForbiddenCursor);

      auto  pos = _map_from_global(event->globalPos());
      auto* dst = _get_node_at_point(pos);
      if (dst && dst->id != this->_connecting.source_node) {
         if (this->_connecting.is_connecting) {
            if (_node_is_root(*dst))
               return;
            this->setCursor(Qt::CursorShape::CrossCursor);
         } else {
            auto* src = this->_node_by_id(this->_connecting.source_node.value());
            bool  connected = false;
            if (src) {
               for (auto id : src->connects_to) {
                  if (id == dst->id) {
                     connected = true;
                     break;
                  }
               }
            }
            if (connected) {
               this->setCursor(Qt::CursorShape::CrossCursor);
            }
         }
      }
      return;
   }
   if (!this->_mouse.is_cancelled) {
      if (this->_mouse.is_drag_moving) {
         this->setCursor(Qt::CursorShape::DragMoveCursor);
         return;
      }
      if (this->_mouse.is_panning) {
         this->setCursor(Qt::CursorShape::ClosedHandCursor);
         return;
      }
      if (this->_mouse.mousedown_on) {
         this->setCursor(Qt::CursorShape::ArrowCursor);
         return;
      }
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

SkillTreeVisualEditor::_geometry_update_info::_geometry_update_info() : label({ .font_metrics = QFontMetrics(QFont()) }) {}
SkillTreeVisualEditor::_geometry_update_info SkillTreeVisualEditor::_get_geometry_update_info() const {
   _geometry_update_info out;

   out.max_radius = std::max(
      this->_style.nodes.general.radius,
      this->_style.nodes.selected.radius
   );
   out.label.max_distance = std::max(
      this->_style.nodes.general.text.distance,
      this->_style.nodes.selected.text.distance
   );
   out.label.max_size = this->font().pixelSize();
   {
      QFont widget_font = this->font();
      QFont node_g_font = this->_style.nodes.general.text.font;
      QFont node_s_font = this->_style.nodes.selected.text.font;
      node_g_font = node_g_font.resolve(widget_font);
      node_s_font = node_s_font.resolve(widget_font);
      out.label.max_size = std::max(out.label.max_size, node_g_font.pixelSize());
      out.label.max_size = std::max(out.label.max_size, node_s_font.pixelSize());
   }
   out.label.font_metrics = this->fontMetrics();

   return out;
}
void SkillTreeVisualEditor::_update_geometry() {
   this->_cached = {};

   const auto geom = _get_geometry_update_info();

   int max_x = 0;
   int max_y = 0;
   for (auto& node_ptr : this->_nodes) {
      this->_update_node_geometry(*node_ptr, geom);
      if (_node_is_root(*node_ptr))
         continue;

      this->_cached.col_count = std::max(this->_cached.col_count, node_ptr->data.position.grid.x);
      this->_cached.row_count = std::max(this->_cached.row_count, node_ptr->data.position.grid.y);

      QPointF point = node_ptr->_cached.geometry.centerpoint;
      point.ry() += geom.max_radius + geom.label.max_distance + geom.label.max_size;
      max_x = std::max(max_x, (int)point.x());
      max_y = std::max(max_y, (int)point.y());

      max_x += node_ptr->_cached.geometry.text.width() / 2;
   }
   ++this->_cached.col_count;
   ++this->_cached.row_count;
   if (this->_cached.row_count < minimum_row_count) {
      this->_cached.row_count = minimum_row_count;
   }
   if (this->_cached.col_count < minimum_col_count) {
      this->_cached.col_count = minimum_col_count;
   }
   max_x = std::max<int>(max_x, this->_cached.col_count * grid_cell_w);
   max_y = std::max<int>(max_y, this->_cached.row_count * grid_cell_h);
   ++max_x; // account for the drawn borders
   ++max_y; // account for the drawn borders

   this->_cached.canvas_size = QSize(max_x, max_y);
   if (this->_style.advisory_level_labels.visible)
      this->_cached.canvas_size.rwidth() += this->_style.advisory_level_labels.width;

   this->updateGeometry();
}
void SkillTreeVisualEditor::_update_node_geometry(PerkNode& node, const _geometry_update_info& geom) {
   if (_node_is_root(node)) {
      const auto& root_style = this->_style.nodes.root;
      const auto  padding    = root_style.padding;

      const auto line_1_rect = geom.label.font_metrics.boundingRect(tr("Root"));
      const auto line_2_rect = geom.label.font_metrics.boundingRect(tr("Node"));

      auto& rect = node._cached.geometry.text;
      rect.setWidth(std::max(line_1_rect.width(), line_2_rect.width()));
      rect.setHeight(line_1_rect.height() + line_2_rect.height());

      auto& box_rect = node._cached.geometry.box;
      box_rect = rect;
      box_rect.adjust(-padding, -padding, padding, padding);

      node._cached.geometry.centerpoint = QPointF{
         (qreal) -(box_rect.width() / 2),
         (qreal) -box_rect.height() - root_style.distance
      };
      box_rect.moveCenter(node._cached.geometry.centerpoint.toPoint());

      return;
   }
   node._cached.geometry.centerpoint = _node_centerpoint(node);
   node._cached.geometry.text = geom.label.font_metrics.boundingRect(node._cached.perk_editor_id);
}
void SkillTreeVisualEditor::_update_node_geometry(PerkNode& node) {
   const auto geom = _get_geometry_update_info();
   this->_update_node_geometry(node, geom);
}
