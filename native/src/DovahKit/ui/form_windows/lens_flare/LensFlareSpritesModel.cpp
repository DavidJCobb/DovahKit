#include "./LensFlareSpritesModel.h"
#include <QColor>

LensFlareSpritesModel::LensFlareSpritesModel(QObject* parent) : DKGenericListModel(parent) {
}

QVariant LensFlareSpritesModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
      case Qt::EditRole:
         switch (column) {
            case Column::Name:
               return node._cached.id;
            case Column::Opacity:
               return node.data.opacity;
            case Column::Tint:
               {
                  const auto& src = node.data.tint;
                  QColor      dst;
                  dst.setRedF(src.r);
                  dst.setGreenF(src.g);
                  dst.setBlueF(src.b);
                  return dst;
               }
               break;
            case Column::Size:
               return node._cached.size;
            case Column::Position:
               return node.data.position;
            case Column::Texture:
               return node._cached.texture;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags LensFlareSpritesModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant LensFlareSpritesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Name:
         return tr("Sprite ID");
      case Texture:
         return tr("Texture");
      case Opacity:
         return tr("Opacity");
      case Tint:
         return tr("Tint");
      case Size:
         return tr("Size");
      case Position:
         return tr("Position");
   }
   return {};
}

QModelIndex LensFlareSpritesModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->_recache_item(*this->_nodes.back());
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex LensFlareSpritesModel::overwrite(int row, const dovah::loaded_forms::LensFlare::sprite& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* node = this->_nodes[row];
   *static_cast<dovah::loaded_forms::LensFlare::sprite*>(node) = src;
   this->_recache_item(*node);
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count - 1, {});
   emit dataChanged(tl, br);
   return tl;
}
const dovah::loaded_forms::LensFlare::sprite* LensFlareSpritesModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void LensFlareSpritesModel::overwriteAllItems(const std::vector<dovah::loaded_forms::LensFlare::sprite>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
         this->_recache_item(*node);
      }
   });
}

void LensFlareSpritesModel::_recache_item(node_type& node) {
   node._cached = {};
   node._cached.id = QString::fromStdString(node.id);
   node._cached.texture = QString::fromStdString(node.texture);
   node._cached.size = tr("%1 x %2").arg(node.data.width).arg(node.data.height);
}