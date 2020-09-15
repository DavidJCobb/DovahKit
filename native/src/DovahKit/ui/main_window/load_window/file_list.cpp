#include "file_list.h"
#include <filesystem>
#include <QDirIterator>
#include <QHeaderView>
#include <QLineEdit>
#include "../../../dovah/files/file_header.h"
#include "../../../editor/core.h"

#include "windows.h"
#include "../../../helpers/intrusive_windows_defines.h"

#pragma region LoadOrderFileListModel
LoadOrderFileListModelItem::LoadOrderFileListModelItem(const dovah::file_header& header, const QDateTime& created, const QDateTime& modified) {
   this->filename  = QString::fromStdString(header.name);
   this->is_master = header.is_master();
   this->dependencies.reserve(header.masters.size());
   for (auto& m : header.masters)
      this->dependencies.push_back(m.c_str());
   this->author      = QString::fromStdString(header.author); // TODO: locale support
   this->description = QString::fromStdString(header.description); // TODO: locale support
   this->created  = created;
   this->modified = modified;
}
inline int LoadOrderFileListModelRoot::indexOf(item_type* item) const noexcept {
   int size = this->_children.size();
   for (int i = 0; i < size; ++i)
      if (this->_children[i] == item)
         return i;
   return -1;
}

QModelIndex LoadOrderFileListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->root->child(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex LoadOrderFileListModel::index(item_type* item) const {
   int row = this->root->indexOf(item);
   return this->index(row, 1, QModelIndex());
}
QModelIndex LoadOrderFileListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int LoadOrderFileListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->root->childCount();
}
int LoadOrderFileListModel::columnCount(const QModelIndex& item) const {
   return 2;
}
Qt::ItemFlags LoadOrderFileListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   int flags = Qt::ItemFlag::ItemIsSelectable;
   if (index.column() == 0) {
      flags |= Qt::ItemFlag::ItemIsUserCheckable;
      auto item = (item_type*)index.internalPointer();
      if (item && item != this->active)
         flags |= Qt::ItemFlag::ItemIsEnabled;
   } else {
      flags |= Qt::ItemFlag::ItemIsEnabled;
   }
   return flags;
}
QVariant LoadOrderFileListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (item_type*)index.internalPointer();
   auto column = index.column();
   switch (role) {
      case Qt::CheckStateRole:
         if (column == 0) {
            if (item == this->active)
               return Qt::Checked;
            return item->selected ? Qt::Checked : Qt::Unchecked;
         }
         break;
      case Qt::DisplayRole:
         switch (column) {
            case 0:
               return item->name();
            case 1:
               if (item->is_master) {
                  if (this->active == item)
                     return tr("Active Master", "load order file list");
                  return tr("Master", "load order file list");
               }
               if (this->active == item)
                  return tr("Active Plugin", "load order file list");
               return tr("Plugin", "load order file list");
         }
         break;
   }
   return QVariant();
}
bool LoadOrderFileListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
   if (role != Qt::CheckStateRole || !index.isValid())
      return false;
   if (index.column() != 0)
      return false;
   auto item  = static_cast<item_type*>(index.internalPointer());
   auto state = static_cast<Qt::CheckState>(value.toInt());
   if (item == this->active) // do not allow the user to uncheck the active file
      state = Qt::Checked;
   item->selected = state == Qt::Checked;
   emit dataChanged(index, index);
   return true;
}
//
QVariant LoadOrderFileListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case 0: return tr("Filename", "load order file list");
            case 1: return tr("Type",     "load order file list");
         }
         break;
   }
   return QVariant();
}

void LoadOrderFileListModel::clear() {
   this->beginResetModel();
   this->active = nullptr;
   this->root->clear();
   this->endResetModel();
}
void LoadOrderFileListModel::insert(const dovah::file_header& header, const QDateTime& created, const QDateTime& modified) {
   auto item = new item_type(header, created, modified);
   //
   auto first_inserted = this->root->childCount();
   auto last_inserted  = first_inserted + 1;
   //
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
   this->root->_children.push_back(item);
   this->endInsertRows();
}

void LoadOrderFileListModel::setActiveFile(item_type* item) noexcept {
   auto previous = this->active;
   this->active = item;
   if (previous) {
      auto index = this->index(item);
      emit dataChanged(index, index);
   }
   if (item) {
      item->selected = true;
      auto index = this->index(item);
      emit dataChanged(index, index);
   }
}
#pragma endregion

#pragma region LoadOrderFileList
LoadOrderFileList::LoadOrderFileList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->setSectionResizeMode(0, QHeaderView::Stretch);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   //
   auto& editor = DovahKitCore::get();
   std::filesystem::path install_path;
   if (!editor.get_game_path(install_path)) {
      //
      // TODO: display error
      //
      return;
   }
   auto model = (model_type*)this->model();
   dovah::file_header fh;
   QDateTime created;
   QDateTime modified;
   QDirIterator it(QString::fromStdWString(install_path.c_str()) + "\\Data\\");
   while (it.hasNext()) {
      auto path = it.next();
      auto info = QFileInfo(path);
      auto ext  = info.completeSuffix().toLower(); // what the hell kind of name is this?
      if (ext != "esl" && ext != "esm" && ext != "esp")
         continue;
      if (fh.load(path.toStdString().c_str())) { // TODO: use std::filesystem::path within the internal "dovah" library
         created  = info.created();
         modified = info.lastModified();
         model->insert(fh, created, modified);
      }
      fh.clear();
   }
   //
   // TODO: Can we sort the model by the user's load order?
   //
};
#pragma endregion