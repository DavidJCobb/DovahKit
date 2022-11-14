#include "file_list.h"
#include <filesystem>
#include <QDirIterator>
#include <QHeaderView>
#include <QLineEdit>
#include "../../../dovah/files/tes_file_reading/file_header.h"
#include "../../../editor/core.h"

#include "helpers/windows.h"

#pragma region LoadOrderFileListModel
LoadOrderFileListModelItem::LoadOrderFileListModelItem(const dovah::tes_file_reading::file_header_reader& header, const QDateTime& created, const QDateTime& modified) {
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

QModelIndex LoadOrderFileListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   item_type* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return QModelIndex();
}
QModelIndex LoadOrderFileListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int LoadOrderFileListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int LoadOrderFileListModel::columnCount(const QModelIndex& item) const {
   return 2;
}
Qt::ItemFlags LoadOrderFileListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   int flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
   if (index.column() == 0) {
      flags |= Qt::ItemFlag::ItemIsUserCheckable;
      //
      // It's tempting to disable the checkbox for the active file, so the user can't 
      // uncheck it, but since the checkbox and the filename are the same column, doing 
      // that greys out the filename and makes it so you can't select the row by clicking 
      // the filename. Not ideal.
      //
      // The (setData) override blocks unchecking active files anyway, so I guess it's 
      // fine. We mainly just lose out on greying out the checkbox.
      //
   }
   return Qt::ItemFlags(QFlag(flags));
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
      case Qt::ToolTipRole:
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
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endResetModel();
}
void LoadOrderFileListModel::setGame(dovah::game g) {
   if (this->current_game == g)
      return;
   this->clear();
   this->current_game = g;
}
void LoadOrderFileListModel::insert(const dovah::tes_file_reading::file_header_reader& header, const QDateTime& created, const QDateTime& modified) {
   auto item = new item_type(header, created, modified);
   //
   auto first_inserted = this->children.size();
   auto last_inserted  = first_inserted + 1;
   //
   this->beginInsertRows(QModelIndex(), first_inserted, last_inserted); // we're not passing the count, we're passing the index of the last row. how annoying.
   this->children.push_back(item);
   this->endInsertRows();
}
void LoadOrderFileListModel::sortByPluginsTxt() {
   std::vector<QString> files;
   DovahKitCore::get().get_game_plugins(files, this->current_game);
   if (files.empty())
      return;
   //
   emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
   //
   uint8_t i    = 0;
   uint8_t j    = 0;
   int     max  = std::min<int>(255, files.size());
   auto&   list = this->children;
   auto    size = list.size();
   for (; i < max; ++i) {
      auto& name = files[i];
      for (int k = j; k < size; ++k) {
         auto* item = list[k];
         if (item->name() == name) {
            if (k == i) // item is already in position
               break;
            std::swap(list[j], list[k]);
            ++j;
            break;
         }
      }
   }
   if (j < size) {
      std::sort(list.begin() + j, list.end(), [](const item_type* a, const item_type* b) {
         return a->modified < b->modified;
      });
   }
   //
   emit layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
}

void LoadOrderFileListModel::setActiveFile(item_type* item) noexcept {
   auto previous = this->active;
   this->active = item;
   if (previous) {
      auto index = this->index(this->children.indexOf(item), 1, QModelIndex());
      emit dataChanged(index, index);
   }
   if (item) {
      bool selected = item->selected;
      auto row      = this->children.indexOf(item);
      item->selected = true;
      auto index0 = this->index(row, selected ? 1 : 0, QModelIndex());
      auto index1 = this->index(row, 1, QModelIndex());
      emit dataChanged(index0, index1);
   }
}
void LoadOrderFileListModel::setSelected(item_type* data, bool state) noexcept {
   data->selected = state;
   auto index = this->index(this->children.indexOf(data), 0, QModelIndex());
   emit dataChanged(index, index);
}
void LoadOrderFileListModel::toggleSelected(item_type* data) noexcept {
   data->selected = !data->selected;
   auto index = this->index(this->children.indexOf(data), 0, QModelIndex());
   emit dataChanged(index, index);
}
#pragma endregion

#pragma region LoadOrderFileList
LoadOrderFileList::LoadOrderFileList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type(this));
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->setSectionResizeMode(0, QHeaderView::Interactive);
   header->setSectionResizeMode(1, QHeaderView::Interactive);
   QObject::connect(this, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      if (!index.isValid())
         return;
      auto data = (model_item_type*)index.internalPointer();
      if (data) {
         auto* model = (model_type*)this->model();
         model->toggleSelected(data);
      }
   });
};
void LoadOrderFileList::listFiles(dovah::game g) {
   auto* model = (model_type*)this->model();
   model->clear();
   //
   model->setGame(g);
   //
   auto& editor = DovahKitCore::get();
   std::filesystem::path install_path;
   if (!editor.get_game_path(install_path, g)) {
      //
      // TODO: display error
      //
      return;
   }
   dovah::tes_file_reading::file_header_reader fh;
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
   model->sortByPluginsTxt();
}
#pragma endregion