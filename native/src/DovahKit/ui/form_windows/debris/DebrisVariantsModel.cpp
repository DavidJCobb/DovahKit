#include "./DebrisVariantsModel.h"
#include "editor/core.h"

DebrisVariantsModel::DebrisVariantsModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &DebrisVariantsModel::clear);
}

QVariant DebrisVariantsModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::Path:
               return node.path.model_path.c_str();
            case Column::Chance:
               return tr("%1%%").arg(node.chance);
            case Column::HasCollision:
               return node.has_collision ? tr("Yes") : tr("No");
         }
         return {};
      case Qt::EditRole:
         switch (column) {
            case Column::Path:
               return node.path.model_path.c_str();
            case Column::Chance:
               return node.chance;
            case Column::HasCollision:
               return node.has_collision;
         }
         return {};
      case Qt::TextAlignmentRole:
         if (column == Column::Chance) {
            return Qt::AlignmentFlag::AlignRight;
         }
         break;
   }
   return {};
}
Qt::ItemFlags DebrisVariantsModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant DebrisVariantsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Path:
         return tr("Path");
      case Chance:
         return tr("Chance");
      case HasCollision:
         return tr("Collision?");
   }
   return {};
}

QModelIndex DebrisVariantsModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex DebrisVariantsModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* node = this->_nodes[row];
   *node = src;
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count - 1, {});
   emit dataChanged(tl, br);
   return tl;
}
const DebrisVariantsModel::node_type* DebrisVariantsModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void DebrisVariantsModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
      }
   });
}