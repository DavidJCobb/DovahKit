#include "./ClimateWeathersModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"
#include "editor/helpers/form_stub_drag_drop.h"

ClimateWeathersModel::ClimateWeathersModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ClimateWeathersModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      for(size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         if (node->weather == stub) {
            node->cached.weather_editor_id = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, Column::Weather, {});
            emit this->dataChanged(qmi, qmi);
         }
         if (node->global == stub) {
            node->cached.global_editor_id = QString::fromStdString(stub->editorID);

            auto qmi = this->index(i, Column::Global, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      size_t size = this->_nodes.size();
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i];
         if (node->weather == stub) {
            this->beginRemoveRows({}, i, i);
            this->_nodes.erase(this->_nodes.begin() + i);
            delete node;
            --i;
            --size;
            this->endRemoveRows();
            continue;
         }
         if (node->global == stub) {
            node->global = nullptr;
            node->cached.global_editor_id.clear();
            //
            auto qmi = this->index(i, Column::Global, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
}

QVariant ClimateWeathersModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
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
            case Column::Weather:
               return node.cached.weather_editor_id;
            case Column::Chance:
               return node.chance;
            case Column::Global:
               return node.cached.global_editor_id;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags ClimateWeathersModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant ClimateWeathersModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case Weather:
         return tr("Weather");
      case Chance:
         return tr("Chance");
      case Global:
         return tr("Global");
   }
   return {};
}


#pragma region Drag-and-drop
   bool ClimateWeathersModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
      if (action != Qt::DropAction::CopyAction)
         return false;
      if (!data)
         return false;
      if (!data->hasFormat(editor_helpers::form_stub_array_mime_type))
         return false;
      return true;
   }
   bool ClimateWeathersModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
      if (!this->canDropMimeData(data, action, row, column, parent))
         return false;
      if (action == Qt::IgnoreAction)
         return true;
         
      auto stubs = editor_helpers::form_stubs_from_mime_data(*data);
      if (stubs.empty())
         return true;

      for (auto* stub : stubs) {
         if (stub->form_type != dovah::form_type::weather)
            continue;
         if (this->containsWeather(stub))
            continue;

         size_t i = this->_nodes.size();
         this->beginInsertRows({}, i, i);
         this->_nodes.push_back(new node_type{});
         {
            auto* node = this->_nodes.back();
            node->weather = stub;
            node->chance  = 0;
            node->cached.weather_editor_id = QString::fromStdString(stub->editorID);
         }
         this->endInsertRows();
      }
      return true;
   }
   QStringList ClimateWeathersModel::mimeTypes() const {
      return QStringList(QString(editor_helpers::form_stub_array_mime_type));
   }
   Qt::DropActions ClimateWeathersModel::supportedDropActions() const {
      return Qt::CopyAction;
   }
#pragma endregion

QModelIndex ClimateWeathersModel::create() {
   auto i = this->_nodes.size();
   this->beginInsertRows({}, i, i);
   this->_nodes.push_back(new node_type{});
   this->endInsertRows();
   return this->index(i, 0, {});
}
QModelIndex ClimateWeathersModel::overwrite(int row, const node_type& src) {
   if (row < 0 || row >= this->_nodes.size())
      return {};
   auto* node = this->_nodes[row];
   *node = src;

   if (auto* stub = node->weather)
      node->cached.weather_editor_id = QString::fromStdString(stub->editorID);
   else
      node->cached.weather_editor_id = "";

   if (auto* stub = node->global)
      node->cached.global_editor_id = QString::fromStdString(stub->editorID);
   else
      node->cached.global_editor_id = "";
   
   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count, {});
   emit dataChanged(tl, br);
   return tl;
}
const ClimateWeathersModel::node_type* ClimateWeathersModel::item(int row) const {
   if (row < 0 || row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}

void ClimateWeathersModel::overwriteAllItems(const std::vector<node_type>& src) {
   this->performReset([this, &src]() {
      size_t size = src.size();
      this->_nodes.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i] = new node_type{ src[i] };
         if (auto* stub = node->weather)
            node->cached.weather_editor_id = QString::fromStdString(stub->editorID);
         if (auto* stub = node->global)
            node->cached.global_editor_id = QString::fromStdString(stub->editorID);
      }
   });
}