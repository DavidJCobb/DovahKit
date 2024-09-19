#include "./ActorBaseCreatureSoundsModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

void ActorBaseCreatureSoundsModelNode::recache_editor_id() {
   if (auto* stub = this->sound)
      this->cached.editorID = QString::fromStdString(stub->editorID);
   else
      this->cached.editorID = "";
}

//

ActorBaseCreatureSoundsModel::ActorBaseCreatureSoundsModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ActorBaseCreatureSoundsModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      for(size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         if (node->sound == stub) {
            node->recache_editor_id();

            auto qmi = this->index(i, Column::Form, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      size_t size = this->_nodes.size();
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i];
         if (node->sound == stub) {
            node->sound = nullptr;
            node->recache_editor_id();

            auto qmi = this->index(i, Column::Form, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
}

QVariant ActorBaseCreatureSoundsModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::TextAlignmentRole:
         switch (column) {
            case Column::Chance:
               return (int)(Qt::AlignRight | Qt::AlignVCenter);
         }
         return {};

      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::Type:
               switch (node.type) {
                  using enum ActorBaseCreatureSoundsModelNode::sound_type;
                  case idle:
                     return tr("Idle");
                  case aware:
                     return tr("Aware");
                  case attack:
                     return tr("Attack");
                  case hit:
                     return tr("Hit");
                  case death:
                     return tr("Death");
                  case weapon:
                     return tr("Weapon");
                  case movement_loop:
                     return tr("Movement Loop");
                  case conscious_loop:
                     return tr("Conscious Loop");
               }
               break;
            case Column::Chance:
               return node.chance;
            case Column::Form:
               return node.cached.editorID;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags ActorBaseCreatureSoundsModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant ActorBaseCreatureSoundsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Type:
         return tr("Type");
      case Chance:
         return tr("Chance");
      case Form:
         return tr("Sound");
   }
   return {};
}

QModelIndex ActorBaseCreatureSoundsModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex ActorBaseCreatureSoundsModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* dst = this->_nodes[row];
   *dst = src;
   dst->recache_editor_id();
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count, {});
   emit dataChanged(tl, br);
   return tl; // TODO: Do we want to sort the sounds by type?
}
const ActorBaseCreatureSoundsModel::node_type* ActorBaseCreatureSoundsModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void ActorBaseCreatureSoundsModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
         node->recache_editor_id();
      }
   });
}