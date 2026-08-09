#include "./SoundDescriptorSoundFilesModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

SoundDescriptorSoundFilesModel::SoundDescriptorSoundFilesModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &SoundDescriptorSoundFilesModel::clear);
}

QVariant SoundDescriptorSoundFilesModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case 0:
               return node.filepath;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags SoundDescriptorSoundFilesModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant SoundDescriptorSoundFilesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Filename:
         return tr("File path");
   }
   return {};
}

QModelIndex SoundDescriptorSoundFilesModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex SoundDescriptorSoundFilesModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* dst = this->_nodes[row];
   *dst = src;
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count, {});
   emit dataChanged(tl, br);
   return tl;
}
const SoundDescriptorSoundFilesModel::node_type* SoundDescriptorSoundFilesModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void SoundDescriptorSoundFilesModel::importItems(const std::vector<std::string>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type;
         node->filepath = QString::fromStdString(src[i]);
      }
   });
}
void SoundDescriptorSoundFilesModel::exportItems(std::vector<std::string>& dst) const {
   size_t size = this->_nodes.size();
   dst.resize(this->_nodes.size());
   for (size_t i = 0; i < size; ++i) {
      dst[i] = this->_nodes[i]->filepath.toStdString();
   }
}