#include "./TopicInfoLinkedTopicsModel.h"
#include "dovah/form_reference_t.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

void TopicInfoLinkedTopicsModelNode::recache() {
   if (!this->stub) {
      this->cached.topic_editor_id = TopicInfoLinkedTopicsModel::tr("<DELETED>");
      this->cached.branch_editor_id.clear();
      return;
   }
   this->cached.topic_editor_id = QString::fromStdString(this->stub->editorID);

   auto* branch = dovah::form_stub_helpers::get_dialogue_topic_branch(*this->stub);
   if (branch)
      this->cached.branch_editor_id = QString::fromStdString(branch->editorID);
   else
      this->cached.branch_editor_id.clear();
}

TopicInfoLinkedTopicsModel::TopicInfoLinkedTopicsModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &TopicInfoLinkedTopicsModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (stub->form_type != dovah::form_type::topic)
         return;
      for (size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         if (node->stub == stub) {
            node->recache();

            auto tl = this->index(i, 0, {});
            auto br = this->index(i, column_count - 1, {});
            emit dataChanged(tl, br);

            return;
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      if (stub->form_type != dovah::form_type::topic)
         return;
      auto&  list = this->_nodes;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* node = list[i];
         if (node->stub == stub) {
            if (node->locked) {
               node->stub = nullptr;
               node->recache();
               this->emitNodeChanged(this->index(i, 0, {}));
               return;
            }
            this->beginRemoveRows({}, i, i);
            list.erase(list.begin() + i);
            delete node;
            this->endRemoveRows();
            return;
         }
      }
   });
}

QVariant TopicInfoLinkedTopicsModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::Branch:
               return node.cached.branch_editor_id;
            case Column::Topic:
               return node.cached.topic_editor_id;
         }
         break;
   }
   return {};
}
Qt::ItemFlags TopicInfoLinkedTopicsModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant TopicInfoLinkedTopicsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   if (role != Qt::DisplayRole)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Branch:
         return tr("Branch");
      case Topic:
         return tr("Topic");
   }
   return {};
}

void TopicInfoLinkedTopicsModel::addTopic(dovah::form_stub* stub) {
   if (!stub)
      return;
   if (stub->form_type != dovah::form_type::topic)
      return;
   if (this->containsTopic(stub))
      return;

   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   auto* node = new node_type;
   this->_nodes.push_back(node);
   {
      node->stub = stub;
      node->recache();
   }
   this->endInsertRows();
}
void TopicInfoLinkedTopicsModel::reorderTopic(size_t row, int by) {
   if (by == 0)
      return;
   if (row >= this->_nodes.size())
      return;
   if (by < 0) {
      if (row < -by)
         return;
   } else {
      if (row + by >= this->_nodes.size())
         return;
   }
   if (this->_nodes[row]->locked)
      //
      // Can't move a locked entry.
      //
      return;
   if (by < 0) {
      if (this->_nodes[row + by]->locked)
         //
         // Can't move an unlocked entry between the locked entries.
         //
         return;
   }
   DKGenericListModel::moveItem(this->index(row, 0, {}), by);
}
void TopicInfoLinkedTopicsModel::removeTopic(dovah::form_stub* stub) {
   if (!stub)
      return;
   if (stub->form_type != dovah::form_type::topic)
      return;

   auto&  list = this->_nodes;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* node = list[i];
      if (node->stub != stub)
         continue;
      this->beginRemoveRows({}, i, i);
      list.erase(list.begin() + i);
      delete node;
      --i;
      --size;
      this->endRemoveRows();
      return;
   }
}
void TopicInfoLinkedTopicsModel::removeTopic(size_t row) {
   auto&  list = this->_nodes;
   size_t size = list.size();
   if (row >= size)
      return;
   auto* node = list[row];
   if (node->locked)
      return;
   this->beginRemoveRows({}, row, row);
   list.erase(list.begin() + row);
   delete node;
   this->endRemoveRows();
}

void TopicInfoLinkedTopicsModel::importFrom(
   const std::vector<dovah::form_reference_t>& src_locked,
   const std::vector<dovah::form_reference_t>& src_normal
) {
   this->performReset([this, &src_locked, &src_normal]() {
      auto _insert = [this](const dovah::form_reference_t& use, bool locked) {
         auto* stub = use.get_form_stub();
         if (!stub || stub->form_type != dovah::form_type::topic)
            return;
         auto* node = new node_type;
         this->_nodes.push_back(node);
         node->stub = stub;
         node->locked = locked;
         node->recache();
      };

      for (auto& use : src_locked)
         _insert(use, true);
      for (auto& use : src_normal)
         _insert(use, false);
   });
}
void TopicInfoLinkedTopicsModel::exportTo(std::vector<dovah::form_reference_t>& dst, dovah::loaded_forms::Form& dst_form) {
   dovah::clear_form_reference_list(dst, dst_form);
   for (auto* node : this->_nodes) {
      if (node->locked)
         continue;
      auto& use = dst.emplace_back();
      use.set(dst_form, node->stub);
   }
}