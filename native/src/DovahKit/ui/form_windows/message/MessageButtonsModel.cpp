#include "./MessageButtonsModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

MessageButtonsModel::MessageButtonsModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &MessageButtonsModel::clear);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      size_t size = this->_nodes.size();
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i];
         for (auto& cnd : node->conditions)
            cnd.sever_outbound_references_to(stub);
      }
   });
}

QVariant MessageButtonsModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
      case Qt::EditRole:
         switch (column) {
            case Column::Name:
               return node.text;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags MessageButtonsModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant MessageButtonsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Name:
         return tr("Text");
   }
   return {};
}

QModelIndex MessageButtonsModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex MessageButtonsModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* node = this->_nodes[row];
   *node = src;
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count - 1, {});
   emit dataChanged(tl, br);
   return tl;
}
const MessageButtonsModel::node_type* MessageButtonsModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void MessageButtonsModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
      }
   });
}