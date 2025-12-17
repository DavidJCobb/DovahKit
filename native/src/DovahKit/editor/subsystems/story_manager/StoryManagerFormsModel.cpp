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
#include "editor/core.h"
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
      assert(qmi.model() == this);
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
      assert(qmi.model() == this);
      const node* parent_node = (const node*)qmi.internalPointer();
      if (!parent_node)
         return false;
      return (parent_node->stub.form_type == dovah::form_type::story_quest_node);
   }
   const StoryManagerFormsModel::quest_node* StoryManagerFormsModel::_quest_for_qmi(const QModelIndex& qmi) const noexcept {
      if (!qmi.isValid())
         return nullptr;
      assert(qmi.model() == this);
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
   StoryManagerFormsModel::cached_node_data* StoryManagerFormsModel::_get_cached_data(node& n) noexcept {
      return const_cast<cached_node_data*>(std::as_const(*this)._get_cached_data(n));
   }
   const StoryManagerFormsModel::cached_quest_data* StoryManagerFormsModel::_get_cached_quest_data(const node& n) const noexcept {
      const auto* cached = _get_cached_data(n);
      if (!cached)
         return nullptr;
      if (!std::holds_alternative<cached_quest_data>(cached->typed))
         return nullptr;
      return &std::get<cached_quest_data>(cached->typed);
   }
   StoryManagerFormsModel::cached_quest_data* StoryManagerFormsModel::_get_cached_quest_data(node& n) noexcept {
      return const_cast<cached_quest_data*>(std::as_const(*this)._get_cached_quest_data(n));
   }

   void StoryManagerFormsModel::_recache_node_core_properties(const node& n) {
      auto& cached = this->_cache.nodes[(node*)&n]; // containers choke on const pointer keys -_-
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
            if (cached.is_random) {
               cached.display_string = tr("Random Quest Node: %1");
            } else {
               cached.display_string = tr("Stacked Quest Node: %1");
            }
            cached.display_string = cached.display_string.arg(cached.editor_id);
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
   void StoryManagerFormsModel::_recache_quest_data(const node& n, cached_node_data& cached) {
      assert(n.stub.form_type == dovah::form_type::story_quest_node);
      if (!std::holds_alternative<cached_quest_data>(cached.typed))
         cached.typed.emplace<cached_quest_data>();
      auto& cqd    = std::get<cached_quest_data>(cached.typed);
      auto  loaded = n.stub.load().ptr_cast<dovah::loaded_forms::StoryManagerQuestNode>();
      if (!loaded)
         return;
      cqd.num_to_run     = loaded->num_quests_to_run;
      cqd.max_concurrent = loaded->max_concurrent_quests;
   }
   void StoryManagerFormsModel::_recache_quest_list(const node& n, cached_node_data& cached, bool clobber_sans_signals) {
      assert(n.stub.form_type == dovah::form_type::story_quest_node);
      if (!std::holds_alternative<cached_quest_data>(cached.typed))
         cached.typed.emplace<cached_quest_data>();
      auto& cqd    = std::get<cached_quest_data>(cached.typed);
      auto  loaded = n.stub.load().ptr_cast<dovah::loaded_forms::StoryManagerQuestNode>();
      if (!loaded)
         return;

      if (!clobber_sans_signals) {
         bool         any_quests_changed = true;
         const size_t count = cqd.quests.size();
         if (count == loaded->quests.size()) {
            any_quests_changed = false;
            for (size_t i = 0; i < cqd.quests.size(); ++i) {
               auto& src = loaded->quests[i];
               auto& dst = *cqd.quests[i];
               if (src.form.get_form_stub() != dst.stub) {
                  any_quests_changed = true;
                  break;
               }
            }
         }
         if (!any_quests_changed) {
            auto smqn_qmi = _qmi_for_node(n);
            for (size_t i = 0; i < count; ++i) {
               auto& src = loaded->quests[i];
               auto& dst = *cqd.quests[i];

               bool changed = false;
               quest_node updated;
               updated.stub                 = dst.stub;
               updated.editor_id            = QString::fromStdString(dst.stub->editorID);
               updated.reset_after_24_hours = src.flags & dovah::loaded_forms::StoryManagerQuestNode::quest_entry::flag::reset_after_24_hours;
               updated.hours_until_reset    = src.get_hours_until_reset();
               if (updated != dst) {
                  dst = updated;

                  auto qust_qmi = this->index(i, 0, smqn_qmi);
                  emit dataChanged(qust_qmi, qust_qmi);
               }
            }
            return;
         }
      }

      QModelIndex parent_qmi;
      bool emitted_removal = false;
      if (!clobber_sans_signals) {
         parent_qmi = _qmi_for_node(n);
         if (!cqd.quests.empty()) {
            this->beginRemoveRows(parent_qmi, 0, cqd.quests.size() - 1);
            emitted_removal = true;
         }
      }
      cqd.quests.clear();
      if (!clobber_sans_signals) {
         this->endRemoveRows();
      }
      cqd.quests.reserve(loaded->quests.size());
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
         dst.hours_until_reset    = src.get_hours_until_reset();
      }
      if (!clobber_sans_signals) {
         this->beginInsertRows(parent_qmi, 0, cqd.quests.size() - 1);
         this->endInsertRows();
      }
   }
   void StoryManagerFormsModel::_recache_node_from_scratch(const node& n) {
      auto loaded_base = n.stub.load();
      this->_recache_node_core_properties(n);
      auto& cached = this->_cache.nodes[(node*)&n]; // containers choke on const pointer keys -_-

      switch (n.stub.form_type) {
         case dovah::form_type::story_quest_node:
            this->_recache_quest_data(n, cached);
            this->_recache_quest_list(n, cached, true);
            break;
         case dovah::form_type::story_branch_node:
            break;
         case dovah::form_type::story_event_node:
            break;
      }
   }
   void StoryManagerFormsModel::_recache_node(const node& n) {
      auto loaded_base = n.stub.load();
      this->_recache_node_core_properties(n);
      auto& cached = this->_cache.nodes[(node*)&n]; // containers choke on const pointer keys -_-

      switch (n.stub.form_type) {
         case dovah::form_type::story_quest_node:
            this->_recache_quest_data(n, cached);
            this->_recache_quest_list(n, cached, false);
            break;
         case dovah::form_type::story_branch_node:
            break;
         case dovah::form_type::story_event_node:
            break;
      }

      auto qmi = _qmi_for_node(n);
      emit dataChanged(qmi, qmi);
   }
   void StoryManagerFormsModel::_on_form_modified(passkeys::core_controls_model, const dovah::form_stub& stub) {
      auto& ds = _get_datastore();
      switch (stub.form_type) {
         case dovah::form_type::story_quest_node:
            if (this->_callback_state.next_form_modify_is_us_changing_quest_list) {
               this->_callback_state.next_form_modify_is_us_changing_quest_list = false;
               break;
            }
            [[fallthrough]];
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_event_node:
            if (auto* node = ds.node_by_stub(stub))
               this->_recache_node(*node);
            break;
         case dovah::form_type::quest:
            for (const auto& pair : ds.nodes_by_stub) {
               auto* smqn_stub = pair.first;
               if (!smqn_stub || smqn_stub->form_type != dovah::form_type::story_quest_node)
                  continue;
               auto* cached = _get_cached_quest_data(*pair.second);
               if (!cached)
                  continue;

               auto smqn_qmi = _qmi_for_node(*pair.second);
               for (size_t i = 0; i < cached->quests.size(); ++i) {
                  auto& item = cached->quests[i];
                  if (item->stub == &stub) {
                     auto qust_qmi = this->index(i, 0, smqn_qmi);
                     emit dataChanged(qust_qmi, qust_qmi);
                  }
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
            case EditorIDRole:
               return q->editor_id;
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
         case EventTypeRole:
            if (n->stub.form_type == dovah::form_type::story_event_node) {
               if (std::holds_alternative<cached_event_data>(cached->typed)) {
                  return (int)std::get<cached_event_data>(cached->typed).event;
               }
            }
            break;
         case EditorIDRole:
            if (cached->editor_id.isEmpty()) {
               if (n->stub.form_type == dovah::form_type::story_event_node) {
                  if (std::holds_alternative<cached_event_data>(cached->typed)) {
                     return editor_helpers::story_event_name((dovah::story_event_code::type)std::get<cached_event_data>(cached->typed).event);
                  }
               }
            }
            return cached->editor_id;
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

bool StoryManagerFormsModel::canMoveUp(const QModelIndex& qmi) const noexcept {
   if (!qmi.isValid())
      return false;
   const node* n = _node_for_qmi(qmi);
   if (n) {
      if (n->parent == nullptr)
         return false;
      switch (n->stub.form_type) {
         case dovah::form_type::story_event_node:
            return false;
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_quest_node:
            break;
      }
      auto i = n->parent->index_of_child(*n);
      return i > 0;
   }
   const node* smqn = (const node*)qmi.internalPointer();
   if (!smqn)
      return false;
   return qmi.row() > 0;
}
bool StoryManagerFormsModel::canMoveDown(const QModelIndex& qmi) const noexcept {
   if (!qmi.isValid())
      return false;
   const node* n = _node_for_qmi(qmi);
   if (n) {
      if (n->parent == nullptr)
         return false;
      switch (n->stub.form_type) {
         case dovah::form_type::story_event_node:
            return false;
         case dovah::form_type::story_branch_node:
         case dovah::form_type::story_quest_node:
            break;
      }
      auto i = n->parent->index_of_child(*n);
      return i + 1 < n->parent->children.size();
   }
   const node* smqn = (const node*)qmi.internalPointer();
   if (!smqn)
      return false;
   auto* cached = _get_cached_quest_data(*smqn);
   if (!cached)
      return false;
   return qmi.row() + 1 < cached->quests.size();
}
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

std::optional<StoryManagerFormsModel::quest_properties> StoryManagerFormsModel::questProperties(const QModelIndex& quest_qmi) const noexcept {
   auto* q = _quest_for_qmi(quest_qmi);
   if (!q)
      return {};
   return *q;
}
void StoryManagerFormsModel::setQuestProperties(const QModelIndex& quest_qmi, const quest_properties& v) noexcept {
   if (!quest_qmi.isValid())
      return;
   assert(quest_qmi.model() == this);
   node* n = (node*)quest_qmi.internalPointer();
   if (!n || n->stub.form_type != dovah::form_type::story_quest_node)
      return;
   auto* cached = _get_cached_quest_data(*n);
   if (!cached || quest_qmi.row() >= cached->quests.size())
      return;

   auto& dst = *cached->quests[quest_qmi.row()];
   dst.quest_properties::operator=(v); // overwrite just the superclass properties
   {
      auto loaded = n->stub.load().ptr_cast<dovah::loaded_forms::StoryManagerQuestNode>();
      assert(!!loaded);
      if (loaded->quests.size() != cached->quests.size()) {
         //
         // This shouldn't happen, but I feel like coding defensively. We have 
         // enough indirection and layers of abstraction that it's giving me 
         // the willies a little bit.
         //
         #if _DEBUG
            __debugbreak(); // We're out of date?!
         #endif
         _update_quest_node_quest_list(*n);
      } else {
         auto& editor = DovahKitCore::get();
         emit editor.formModificationImminent(&n->stub);
         this->_callback_state.next_form_modify_is_us_changing_quest_list = true;

         auto& src_item = dst;
         auto& dst_item = loaded->quests[quest_qmi.row()];
         dst_item.set_hours_until_reset(src_item.hours_until_reset);
         cobb::edit_bit(dst_item.flags, dovah::loaded_forms::StoryManagerQuestNode::quest_entry::flag::reset_after_24_hours, src_item.reset_after_24_hours);

         n->stub.set_edited(true);
         emit editor.formModified(&n->stub);

      }
   }
   emit dataChanged(quest_qmi, quest_qmi);
}

QModelIndex StoryManagerFormsModel::_create_node_in(const QModelIndex& parent_qmi, dovah::form_type ft, QString editor_id) {
   const auto* parent_node = _node_for_qmi(parent_qmi);
   if (!parent_node)
      return {};
   const auto* parent_branch = dynamic_cast<const branch_node*>(parent_node);
   if (!parent_branch)
      return {};

   dovah::form_stub* stub = nullptr;
   {
      auto  request = DovahKitCore::get().request_form_creation(ft);
      request.editorID = editor_id.toStdString();
      stub = request.commit();
   }
   assert(stub != nullptr);
   const node* subject_node = _get_datastore().node_by_stub(*stub);
   assert(subject_node != nullptr);

   const node* prev_node = nullptr;
   if (!parent_branch->children.empty())
      prev_node = parent_branch->children.back();

   dovahkit::subsystems::story_manager::core::get().move_node(*subject_node, *parent_branch, prev_node);
   return this->index(*stub);
}
void StoryManagerFormsModel::_update_quest_node_quest_list(node& n) {
   auto& editor = DovahKitCore::get();
   assert(n.stub.form_type == dovah::form_type::story_quest_node);
   auto loaded = n.stub.load().ptr_cast<dovah::loaded_forms::StoryManagerQuestNode>();
   assert(!!loaded);

   emit editor.formModificationImminent(&n.stub);
   this->_callback_state.next_form_modify_is_us_changing_quest_list = true;

   auto& dst_list = loaded->quests;
   if (auto* cached = _get_cached_quest_data(n)) {
      const auto&  src_list = cached->quests;
      const size_t src_size = src_list.size();
      size_t       dst_size = dst_list.size();
      if (src_size > dst_size) {
         dst_list.resize(src_size);
      }
      for (size_t i = 0; i < src_size; ++i) {
         auto& src_item = *src_list[i];
         auto& dst_item = dst_list[i];
         dst_item.form.set(*loaded, src_item.stub);
         dst_item.set_hours_until_reset(src_item.hours_until_reset);
         cobb::edit_bit(dst_item.flags, dovah::loaded_forms::StoryManagerQuestNode::quest_entry::flag::reset_after_24_hours, src_item.reset_after_24_hours);
      }
      if (src_size < dst_size) {
         for (size_t i = src_size; i < dst_size; ++i) {
            auto& dst_item = dst_list[i];
            dst_item.form.set(*loaded, nullptr);
         }
         dst_list.resize(src_size);
      }
   } else {
      for (auto& item : dst_list)
         item.form.set(*loaded, nullptr);
      dst_list.clear();
   }

   n.stub.set_edited(true);
   emit editor.formModified(&n.stub);
}

QModelIndex StoryManagerFormsModel::createBranchIn(const QModelIndex& parent_qmi, QString editorID) {
   return _create_node_in(parent_qmi, dovah::form_type::story_branch_node, editorID);
}
QModelIndex StoryManagerFormsModel::createQuestListIn(const QModelIndex& parent_qmi, QString editorID) {
   return _create_node_in(parent_qmi, dovah::form_type::story_quest_node, editorID);
}
QModelIndex StoryManagerFormsModel::addQuestTo(const QModelIndex& smqn_qmi, dovah::form_stub& quest) {
   if (!smqn_qmi.isValid())
      return {};
   if (quest.form_type != dovah::form_type::quest)
      return {};
   auto* parent_node = _node_for_qmi(smqn_qmi);
   if (!parent_node)
      return {};
   if (parent_node->stub.form_type != dovah::form_type::story_quest_node)
      return {};

   auto& cached = this->_cache.nodes[parent_node];
   if (!std::holds_alternative<cached_quest_data>(cached.typed)) {
      cached.typed.emplace<cached_quest_data>();
   }
   auto& typed = std::get<cached_quest_data>(cached.typed);

   // Verify that the quest isn't already in here.
   for (auto& qust_ptr : typed.quests)
      if (qust_ptr->stub == &quest)
         return {};

   auto i = typed.quests.size();
   this->beginInsertRows(smqn_qmi, i, i);

   auto& dst_ptr = typed.quests.emplace_back();
   dst_ptr = std::make_unique<quest_node>();
   dst_ptr->stub      = &quest;
   dst_ptr->editor_id = QString::fromStdString(quest.editorID);
   this->_update_quest_node_quest_list(*parent_node);

   this->endInsertRows();
   return this->createIndex(i, 0, parent_node);
}
void StoryManagerFormsModel::removeQuestFromNode(const QModelIndex& quest_form_qmi) {
   if (!_qmi_is_quest_form(quest_form_qmi))
      return;
   auto* parent_node = (node*)quest_form_qmi.internalPointer();
   assert(parent_node != nullptr);
   auto* cached      = _get_cached_quest_data(*parent_node);
   if (cached == nullptr)
      return;

   auto i = quest_form_qmi.row();
   if (i >= cached->quests.size())
      return;

   this->beginRemoveRows(_qmi_for_node(*parent_node), i, i);
   cached->quests.erase(cached->quests.begin() + i);
   this->_update_quest_node_quest_list(*parent_node);
   this->endRemoveRows();
}
void StoryManagerFormsModel::deleteNode(const QModelIndex& sm_node_qmi) {
   auto* subject = _node_for_qmi(sm_node_qmi);
   if (!subject)
      return;
   dovahkit::subsystems::story_manager::core::get().delete_node(*subject);
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
      {
         auto rect = pixmap.rect() - qt_border_jank;
         int  rw   = rect.width();
         int  cw   = rw / 2;
         int  rh   = rect.height();
         int  ch   = rh / 2;
         painter.drawPolygon(QPolygon(QVector{
            QPoint{ cw,             0 },
            QPoint{ cw + (rw % 2),  0 },
            QPoint{ rw,            ch },
            QPoint{ rw,            ch + (rh % 2) },
            QPoint{ cw + (rw % 2), rh },
            QPoint{ cw,            rh },
            QPoint{  0,            ch + (rh % 2) },
            QPoint{  0,            ch },
         }));
      }
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
      {
         auto  rect = pixmap.rect() - qt_border_jank;
         qreal rw   = rect.width();
         qreal cw   = rw / 2;
         qreal rh   = rect.height();
         qreal ch   = rh / 2;
         painter.drawEllipse(QPointF{ cw, ch }, 2.5, 2.5);
      }
      icon = pixmap;
   }
}