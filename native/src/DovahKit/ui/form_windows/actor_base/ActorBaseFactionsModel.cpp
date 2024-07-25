#include "./ActorBaseFactionsModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

ActorBaseFactionsModel::ActorBaseFactionsModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ActorBaseFactionsModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      for(size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         if (node->faction == stub) {
            node->cached.editorID = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, Column::Faction, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      size_t size = this->_nodes.size();
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i];
         if (node->faction == stub) {
            this->beginRemoveRows({}, i, i);
            this->_nodes.erase(this->_nodes.begin() + i);
            delete node;
            --i;
            --size;
            this->endRemoveRows();
         }
      }
   });
}

QVariant ActorBaseFactionsModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::TextAlignmentRole:
         switch (column) {
            case Column::Rank:
               return (int)(Qt::AlignRight | Qt::AlignVCenter);
         }
         return {};

      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::Faction:
               return node.cached.editorID;
            case Column::Rank:
               return node.rank;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags ActorBaseFactionsModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant ActorBaseFactionsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Faction:
         return tr("Faction Name");
      case Rank:
         return tr("Rank");
   }
   return {};
}

QModelIndex ActorBaseFactionsModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex ActorBaseFactionsModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   *this->_nodes[row] = src;
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count, {});
   emit dataChanged(tl, br);
}
const ActorBaseFactionsModel::node_type* ActorBaseFactionsModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void ActorBaseFactionsModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
         if (auto* stub = node->faction)
            node->cached.editorID = QString::fromStdString(stub->editorID);
      }
   });
}