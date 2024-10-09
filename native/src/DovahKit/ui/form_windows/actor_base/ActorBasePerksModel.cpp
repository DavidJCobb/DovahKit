#include "./ActorBasePerksModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/helpers/form_stub_drag_drop.h"

ActorBasePerksModel::ActorBasePerksModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ActorBasePerksModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      for(size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         if (node->perk == stub) {
            node->cached.editorID = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, Column::Perk, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      size_t size = this->_nodes.size();
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i];
         if (node->perk == stub) {
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

QVariant ActorBasePerksModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
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
            case Column::Perk:
               return node.cached.editorID;
            case Column::Rank:
               return node.rank;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags ActorBasePerksModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant ActorBasePerksModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Perk:
         return tr("Perk");
      case Rank:
         return tr("Rank");
   }
   return {};
}

#pragma region Drag-and-drop
   bool ActorBasePerksModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
      if (action != Qt::DropAction::CopyAction)
         return false;
      if (!data)
         return false;
      if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
         return false;
      return true;
   }
   bool ActorBasePerksModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
      if (!this->canDropMimeData(data, action, row, column, parent))
         return false;
      if (action == Qt::IgnoreAction)
         return true;
         
      auto stubs = editor_helpers::form_stubs_from_mime_data(*data);
      if (stubs.empty())
         return true;

      for (auto* stub : stubs) {
         if (stub->form_type != dovah::form_type::perk)
            continue;
         if (this->containsPerk(stub))
            continue;

         size_t i = this->_nodes.size();
         this->beginInsertRows({}, i, i);
         this->_nodes.push_back(new node_type{});
         {
            auto* node = this->_nodes.back();
            node->perk = stub;
            node->rank = 0;
            node->cached.editorID = QString::fromStdString(stub->editorID);
         }
         this->endInsertRows();
      }
      return true;
   }
   QStringList ActorBasePerksModel::mimeTypes() const {
      return QStringList(QString(editor_helpers::form_stub_array_mime_type));
   }
   Qt::DropActions ActorBasePerksModel::supportedDropActions() const {
      return Qt::CopyAction;
   }
#pragma endregion

QModelIndex ActorBasePerksModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex ActorBasePerksModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* node = this->_nodes[row];
   *node = src;
   if (auto* stub = node->perk)
      node->cached.editorID = QString::fromStdString(stub->editorID);
   else
      node->cached.editorID = "";
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count, {});
   emit dataChanged(tl, br);
   return tl;
}
const ActorBasePerksModel::node_type* ActorBasePerksModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void ActorBasePerksModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
         if (auto* stub = node->perk)
            node->cached.editorID = QString::fromStdString(stub->editorID);
      }
   });
}