#include "./StoryManagerFormsModel.h"
#include "./story_manager_subsystem.h"
#include "./passkeys/core_controls_model.h"
#include "dovah/datastores/story_manager/branch_node.h"
#include "dovah/datastores/story_manager/leaf_node.h"
#include "dovah/datastores/story_manager.h"
#include "dovah/forms/StoryManagerBranchNode.h"
#include "dovah/forms/StoryManagerEventNode.h"
#include "dovah/forms/StoryManagerQuestNode.h"
#include "dovah/form_stub.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/helpers/story_event_name.h"
#include "editor/form_stub_meta_type.h"

StoryManagerFormsModel::StoryManagerFormsModel(passkeys::core_controls_model, QObject* parent) : QAbstractItemModel(parent) {
   this->_make_icons();
}

/*static*/ const StoryManagerFormsModel::datastore_type& StoryManagerFormsModel::_get_datastore() {
   return dovahkit::subsystems::story_manager::core::get().datastore();
}

#pragma region QMI-to-data mapping
   QModelIndex StoryManagerFormsModel::_qmi_for_node(const node& n) const noexcept {
      if (n.parent) {
         return createIndex(n.parent->index_of_child(n), 0, (void*)(const void*)n.parent);
      }
      if (&n == _get_datastore().root) {
         return createIndex(0, 0, nullptr);
      }
      return {};
   }

   const StoryManagerFormsModel::node* StoryManagerFormsModel::_node_for_qmi(const QModelIndex& qmi) const noexcept {
      if (!qmi.isValid())
         return nullptr;
      const node* parent_node = (const node*)qmi.internalPointer();
      if (!parent_node) {
         if (qmi.row() == 0)
            return _get_datastore().root;
         return nullptr;
      }

      const auto* parent_branch = dynamic_cast<const branch_node*>(parent_node);
      if (!parent_branch)
         return nullptr;

      auto i = qmi.row();
      if (i >= parent_branch->children.size())
         return nullptr;
      return parent_branch->children[i];
   }

   bool StoryManagerFormsModel::_qmi_is_quest_form(const QModelIndex& qmi) const noexcept {
      if (!qmi.isValid())
         return false;
      const node* parent_node = (const node*)qmi.internalPointer();
      if (!parent_node)
         return false;
      return (parent_node->stub.form_type == dovah::form_type::story_quest_node);
   }
   const StoryManagerFormsModel::quest_node* StoryManagerFormsModel::_quest_for_qmi(const QModelIndex& qmi) const noexcept {
      if (!qmi.isValid())
         return nullptr;
      const node* parent_node = (const node*)qmi.internalPointer();
      if (!parent_node)
         return nullptr;
      if (parent_node->stub.form_type != dovah::form_type::story_quest_node)
         return nullptr;

      const cached_quest_data* cached = _get_cached_quest_data(*parent_node);
      if (!cached)
         return nullptr;
      auto i = qmi.row();
      if (i >= cached->quests.size())
         return nullptr;
      return cached->quests[i].get();
   }
#pragma endregion
#pragma region Caching
   const StoryManagerFormsModel::cached_node_data* StoryManagerFormsModel::_get_cached_data(const node& n) const noexcept {
      auto& map = this->_cache.nodes;
      auto  it  = map.find((node*)&n); // containers choke on const pointer keys -_-
      if (it != map.end())
         return &it->second;
      return nullptr;
   }
   const StoryManagerFormsModel::cached_quest_data* StoryManagerFormsModel::_get_cached_quest_data(const node& n) const noexcept {
      const auto* cached = _get_cached_data(n);
      if (!cached)
         return nullptr;
      if (!std::holds_alternative<cached_quest_data>(cached->typed))
         return nullptr;
      return &std::get<cached_quest_data>(cached->typed);
   }

   void StoryManagerFormsModel::_recache_node_from_scratch(const node& n) {
      auto& cached = this->_cache.nodes[(node*)&n]; // containers choke on const pointer keys -_-
      cached = {};
      cached.editor_id      = QString::fromStdString(n.stub.editorID);
      cached.display_string = cached.editor_id;

      bool is_random   = false;
      auto loaded_base = n.stub.load();
      if (loaded_base) {
         auto* mixin = dynamic_cast<dovah::loaded_forms::mixins::StoryManagerNode*>(&*loaded_base);
         if (mixin) {
            cached.is_random            = (mixin->flags & dovah::loaded_forms::mixins::StoryManagerNode::flag::random);
            cached.warn_if_none_started = (mixin->flags & dovah::loaded_forms::mixins::StoryManagerNode::flag::warn_if_no_child_quest_started);
         }
      }

      switch (n.stub.form_type) {
         case dovah::form_type::story_quest_node:
            {
               auto loaded = loaded_base.ptr_cast<dovah::loaded_forms::StoryManagerQuestNode>();
               if (!loaded)
                  break;
               auto& cqd = cached.typed.emplace<cached_quest_data>();

               if (cached.is_random) {
                  cached.display_string = tr("Random Quest Node: %1");
               } else {
                  cached.display_string = tr("Stacked Quest Node: %1");
               }
               cached.display_string = cached.display_string.arg(cached.editor_id);

               cqd.num_to_run     = loaded->num_quests_to_run;
               cqd.max_concurrent = loaded->max_concurrent_quests;

               size_t size = loaded->quests.size();
               cqd.quests.reserve(size);
               for (auto& src : loaded->quests) {
                  if (!src.form)
                     continue;
                  if (src.form.get_form_stub()->form_type != dovah::form_type::quest)
                     continue;
                  auto  dst_ptr = std::make_unique<quest_node>();
                  auto& dst     = *dst_ptr;
                  cqd.quests.emplace_back() = std::move(dst_ptr);
                  dst.stub                 = src.form.get_form_stub();
                  dst.editor_id            = QString::fromStdString(dst.stub->editorID);
                  dst.reset_after_24_hours = src.flags & dovah::loaded_forms::StoryManagerQuestNode::quest_entry::flag::reset_after_24_hours;
                  dst.hours_until_reset    = src.hours_until_reset * 24.0F;
               }
            }
            break;
         case dovah::form_type::story_branch_node:
            if (cached.is_random) {
               cached.display_string = tr("Random Branch Node: %1");
            } else {
               cached.display_string = tr("Stacked Branch Node: %1");
            }
            cached.display_string = cached.display_string.arg(cached.editor_id);
            break;
         case dovah::form_type::story_event_node:
            {
               auto loaded = loaded_base.ptr_cast<dovah::loaded_forms::StoryManagerEventNode>();
               if (!loaded)
                  break;
               auto& cqd = cached.typed.emplace<cached_event_data>();
               cqd.event = loaded->event;
               
               auto name = editor_helpers::story_event_name((dovah::story_event_code::type)cqd.event);
               if (!name.isEmpty()) {
                  if (cached.is_random) {
                     cached.display_string = tr("Random Event Node: %1");
                  } else {
                     cached.display_string = tr("Stacked Event Node: %1");
                  }
                  cached.display_string = cached.display_string.arg(name);
               }
            }
            break;
      }
   }
#pragma endregion

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex StoryManagerFormsModel::index(int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (!parent_qmi.isValid()) {
            if (row == 0) {
               auto& ds = _get_datastore();
               if (ds.root)
                  return _qmi_for_node(*ds.root);
            }
            return {};
         }
         const auto* parent_node = _node_for_qmi(parent_qmi);
         if (!parent_node)
            return {};

         if (parent_node->stub.form_type == dovah::form_type::story_quest_node) {
            const cached_quest_data* cached = _get_cached_quest_data(*parent_node);
            if (!cached)
               return {};
            if (row >= cached->quests.size())
               return {};
         } else if (auto* parent_branch = dynamic_cast<const branch_node*>(parent_node)) {
            if (row >= parent_branch->children.size())
               return {};
         }
         return this->createIndex(row, column, (void*)parent_node);
      }
      /*virtual*/ int StoryManagerFormsModel::columnCount(const QModelIndex& qmi) const /*override*/ {
         return 1;
      }
      /*virtual*/ int StoryManagerFormsModel::rowCount(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid()) {
            if (_get_datastore().root)
               return 1;
            return 0;
         }
         const auto* parent_node = _node_for_qmi(qmi);
         if (!parent_node)
            return 0;

         if (parent_node->stub.form_type == dovah::form_type::story_quest_node) {
            const cached_quest_data* cached = _get_cached_quest_data(*parent_node);
            if (!cached)
               return {};
            return cached->quests.size();
         } else if (auto* parent_branch = dynamic_cast<const branch_node*>(parent_node)) {
            return parent_branch->children.size();
         }
         return 0;
      }
      /*virtual*/ QModelIndex StoryManagerFormsModel::parent(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return {};
         const node* parent_node = (const node*)qmi.internalPointer();
         if (!parent_node)
            return {};
         return _qmi_for_node(*parent_node);
      }
   #pragma endregion
   /*virtual*/ QVariant StoryManagerFormsModel::data(const QModelIndex& qmi, int role) const /*override*/ {
      if (!qmi.isValid())
         return {};
      const node* n = _node_for_qmi(qmi);
      if (!n) {
         const auto* q = _quest_for_qmi(qmi);
         if (!q)
            return {};
         switch (role) {
            case Qt::DisplayRole:
               return q->editor_id;
            case Qt::ToolTipRole:
               {
                  uint32_t form_id = 0;
                  if (q->stub)
                     form_id = q->stub->formID;
                  return tr("%1 (%2)")
                     .arg(q->editor_id)
                     .arg(editor_helpers::form_id_to_string(form_id));
               }
               break;
            case Qt::DecorationRole:
               return this->_icons.quest_form;
            case FormStubRole:
               return QVariant::fromValue(q->stub);
         }
         return {};
      }
      auto* cached = _get_cached_data(*n);
      switch (role) {
         case Qt::DisplayRole:
            return cached->display_string;
         case Qt::ToolTipRole:
            return tr("%1 (%2)")
               .arg(cached->editor_id)
               .arg(editor_helpers::form_id_to_string(n->stub.formID));
         case Qt::DecorationRole:
            switch (n->stub.form_type) {
               case dovah::form_type::story_event_node:
                  return this->_icons.event;
               case dovah::form_type::story_branch_node:
                  return this->_icons.branch;
               case dovah::form_type::story_quest_node:
                  return this->_icons.quest_list;
            }
            break;
         case FormStubRole:
            return QVariant::fromValue(&n->stub);
      }
      return {};
   }
   /*virtual*/ Qt::ItemFlags StoryManagerFormsModel::flags(const QModelIndex& qmi) const /*override*/ {
      auto  flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
      auto* node  = _node_for_qmi(qmi);
      if (!node) {
         return flags;
      }
      if (node != _get_datastore().root) {
         flags |= Qt::ItemIsDragEnabled;
      }
      flags |= Qt::ItemIsDropEnabled;
      return flags;
   }
   
   /*virtual*/ QVariant StoryManagerFormsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation == Qt::Orientation::Horizontal && section == 0) {
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return tr("Story Manager Nodes");
         }
      }
      return {};
   }
#pragma endregion

QModelIndex StoryManagerFormsModel::index(const dovah::form_stub& stub) const noexcept {
   switch (stub.form_type) {
      case dovah::form_type::story_branch_node:
      case dovah::form_type::story_event_node:
      case dovah::form_type::story_quest_node:
         break;
      default:
         return {};
   }
   const auto& map = _get_datastore().nodes_by_stub;
   const auto  it  = map.find((dovah::form_stub*)&stub);
   if (it == map.end())
      return {};
   return _qmi_for_node(*it->second);
}

#pragma region Puppeteering by subsystem core
   void StoryManagerFormsModel::_on_before_datastore_reset(passkeys::core_controls_model) {
      this->beginResetModel();
      this->_cache.nodes.clear();
   }
   void StoryManagerFormsModel::_on_after_datastore_reset(passkeys::core_controls_model, bool rebuilt) {
      if (rebuilt) {
         auto& ds = _get_datastore();
         for (const auto& pair : ds.nodes_by_stub) {
            assert(!!pair.second);
            this->_recache_node_from_scratch(*pair.second);
         }
      }
      this->endResetModel();
   }

   void StoryManagerFormsModel::_on_node_deletion_imminent(passkeys::core_controls_model, const node& subject) {
      if (subject.parent) {
         auto parent_qmi = _qmi_for_node(*subject.parent);
         auto i          = subject.parent->index_of_child(subject);
         assert(i != branch_node::index_of_none);
         this->beginRemoveRows(parent_qmi, i, i);
      } else if (&subject == _get_datastore().root) {
         this->beginRemoveRows({}, 0, 0);
      } else {
         this->beginRemoveRows({}, -1, -1);
      }
   }
   void StoryManagerFormsModel::_on_node_deletion_complete(passkeys::core_controls_model) {
      this->endRemoveRows();
   }

   void StoryManagerFormsModel::_on_node_placement_imminent(passkeys::core_controls_model, const node& subject, const branch_node& dst_parent, size_t dst_pos) {
      auto dst_parent_qmi = _qmi_for_node(dst_parent);
      if (subject.parent) {
         this->_callback_state.last_placement_was_insertion = false;
         auto   src_parent_qmi = _qmi_for_node(*subject.parent);
         size_t from           = subject.parent->index_of_child(subject);
         assert(from != branch_node::index_of_none);
         size_t to = dst_pos;
         if (to > from)
            --to;
         this->beginMoveRows(src_parent_qmi, from, from, dst_parent_qmi, to);
      } else {
         this->_callback_state.last_placement_was_insertion = true;
         this->beginInsertRows(dst_parent_qmi, dst_pos, dst_pos);
      }
   }
   void StoryManagerFormsModel::_on_node_placement_complete(passkeys::core_controls_model, const node& subject) {
      if (this->_callback_state.last_placement_was_insertion) {
         this->_callback_state.last_placement_was_insertion = false;
         this->endInsertRows();
      } else {
         this->endMoveRows();
      }
   }
#pragma endregion

#include <QPainter>
#include <QPixMap>
void StoryManagerFormsModel::_make_icons() {
   constexpr const auto qt_border_jank = QMargins(0, 0, 1, 1);
   {
      auto& icon    = this->_icons.event;
      auto  pixmap  = QPixmap(16, 16);
      pixmap.fill(QColor(0, 0, 0, 0));
      auto  painter = QPainter(&pixmap);
      painter.setPen(QPen(QColor(0, 0, 0), 0));
      painter.setBrush(QColor(192, 192, 192));
      painter.drawPolygon(QPolygon(QVector{
         QPoint{  8,  0 },
         QPoint{ 16,  8 },
         QPoint{  8, 16 },
         QPoint{  0,  8 },
      }));
      icon = pixmap;
   }
   {
      auto& icon    = this->_icons.branch;
      auto  pixmap  = QPixmap(16, 16);
      pixmap.fill(QColor(0, 0, 0, 0));
      auto  painter = QPainter(&pixmap);
      painter.setPen(QPen(QColor(0, 0, 0), 0));
      painter.setBrush(QColor(128, 128, 128));
      painter.drawEllipse(pixmap.rect() - qt_border_jank);
      icon = pixmap;
   }
   {
      auto& icon    = this->_icons.quest_list;
      auto  pixmap  = QPixmap(16, 16);
      pixmap.fill(QColor(0, 0, 0, 0));
      auto  painter = QPainter(&pixmap);
      painter.setPen(QPen(QColor(0, 0, 0), 0));
      painter.setBrush(QColor(255, 255, 255));
      painter.drawEllipse(pixmap.rect() - qt_border_jank);
      icon = pixmap;
   }
   {
      auto& icon    = this->_icons.quest_form;
      auto  pixmap  = QPixmap(16, 16);
      pixmap.fill(QColor(0, 0, 0, 0));
      auto  painter = QPainter(&pixmap);
      painter.setPen(QPen(QColor(128, 128, 128), 0));
      painter.setBrush(QColor(255, 255, 255));
      painter.drawEllipse(QPointF{ 8, 8 }, 2.5, 2.5);
      icon = pixmap;
   }
}