#include "./file_list.h"
#include <filesystem>
#include <QDirIterator>
#include <QHeaderView>
#include <QLineEdit>
#include <QPainter>
#include <QTextOption>
#include "helpers/windows.h"
#include "dovah/exceptions/file_load_failed.h"
#include "dovah/files/tes_file_reading/file_header.h"
#include "editor/core.h"

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
   //
   // The `canonically_ordered_filenames` list is a list of all possible official plug-ins, 
   // followed by non-official plug-ins listed in their `plugins.txt`. We don't verify that 
   // any of these plug-ins are actually in the Data directory, because we really only use 
   // this list to enforce a "canonical" ordering on those files that we've already found 
   // in the Data directory.
   // 
   // Probably I should rename the member function on `get_game_plugins`, but I don't want 
   // to have to recompile the entire program just for this. Something for the post-launch 
   // refactor, then.
   //
   std::vector<QString> canonically_ordered_filenames;
   DovahKitCore::get().get_game_plugins(canonically_ordered_filenames, this->current_game);
   if (canonically_ordered_filenames.empty())
      return;
   
   emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
   
   {
      const auto children_count = this->children.size();

      std::vector<std::pair<size_t, int>> indices;
      indices.resize(children_count);
      for (size_t i = 0; i < children_count; ++i) {
         auto& pair = indices[i];
         pair.first  = i;
         pair.second = -1;
         //
         auto fit = std::find_if(canonically_ordered_filenames.begin(), canonically_ordered_filenames.end(), [this, i](const QString canonical_name) {
            auto known_name = this->children[i]->name();
            return known_name.compare(canonical_name, Qt::CaseInsensitive) == 0;
         });
         if (fit != canonically_ordered_filenames.end())
            pair.second = std::distance(canonically_ordered_filenames.begin(), fit);
      }
      std::sort(indices.begin(), indices.end(), [](const auto& a, const auto& b) {
         if (a.second == -1) {
            if (b.second == -1)
               return a.first < b.first;
            return false;
         }
         if (b.second == -1)
            return true;
         return a.second < b.second;
      });

      size_t first_unknown = children_count;

      decltype(this->children) sorted;
      sorted.resize(children_count);
      for (size_t i = 0; i < children_count; ++i) {
         const auto& pair = indices[i];
         sorted[i] = this->children[pair.first];
      }
      std::swap(this->children, sorted);

      if (first_unknown < children_count) {
         std::sort(this->children.begin() + first_unknown, this->children.end(), [](const item_type* a, const item_type* b) {
            return a->modified < b->modified;
         });
      }
   }
   
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
      try {
         fh.load(path.toStdString().c_str());
      } catch (const dovah::exceptions::file_load_failed& ex) {
         //
         // We don't actually care about error details here, since we're just checking what 
         // files in the directory are available and seem valid.
         //
         fh.clear();
         continue;
      }
      created  = info.birthTime();
      modified = info.lastModified();
      model->insert(fh, created, modified);
      fh.clear();
   }
   //
   model->sortByPluginsTxt();
}
void LoadOrderFileList::paintEvent(QPaintEvent* e) {
   QTableView::paintEvent(e);

   auto* model = (model_type*)this->model();
   if (!model)
      return;
   if (model->rowCount({}) == 0) {
      QPainter painter(this->viewport());

      QPen pen(QColor(128, 128, 128));
      pen.setCosmetic(true);
      painter.setPen(pen);

      QTextOption options;
      options.setWrapMode(QTextOption::WordWrap);
      options.setAlignment(Qt::AlignCenter);
      painter.drawText(this->rect().adjusted(3, 3, -3, -3), tr("No files found. You might want to set your game path manually, in DovahKit's options."), options);
   }
}
#pragma endregion