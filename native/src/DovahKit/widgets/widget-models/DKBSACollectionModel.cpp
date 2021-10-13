#include "DKBSACollectionModel.h"
#include <QApplication>
#include <QIcon>
#include <QMimeDatabase>
#include <QStyle>
#include "../../dovah/files/bsa/bsa_archive.h"
#include "../../dovah/files/bsa/bsa_load_order.h"

namespace {
   using Node   = DKBSACollectionModelBackend::Node;
   using File   = DKBSACollectionModelBackend::File;
   using Folder = DKBSACollectionModelBackend::Folder;
   using node_type = Node::node_type;

   class _Icons {
      private:
         _Icons() {
            auto* style = QApplication::style();
            this->generic_icons.file   = style->standardIcon(QStyle::SP_FileIcon);
            this->generic_icons.folder = style->standardIcon(QStyle::SP_DirIcon);
         }

         QMimeDatabase database;
         QHash<QString, QIcon> icons_by_extension;
         struct {
            QIcon file;
            QIcon folder;
         } generic_icons;

      public:
         static _Icons& get() {
            static _Icons instance;
            return instance;
         }

         QIcon iconForExtension(QString extension) {
            extension = extension.toLower();
            auto& map = this->icons_by_extension;
            auto  it  = map.find(extension);
            if (it != map.end()) {
               auto icon = *it;
               if (icon.isNull())
                  return this->generic_icons.file;
               return icon;
            }
            auto  types = this->database.mimeTypesForFileName(QString("x.%1").arg(extension));
            QIcon icon;
            for (auto& type : types) {
               icon = QIcon::fromTheme(type.iconName());
               if (!icon.isNull())
                  break;
            }
            map[extension] = icon;
            return icon;
         }
         QIcon iconForFilename(const QString& filename) {
            int i = filename.lastIndexOf('.');
            int j = filename.lastIndexOf('/');
            int k = filename.lastIndexOf('\\');
            j = std::max(j, k);
            if (i > j) {
               return this->iconForExtension(filename.mid(i + 1));
            }
            return this->generic_icons.file;
         }
         
         inline QIcon folderIcon() const noexcept { return this->generic_icons.folder; }
   };
}

#pragma region Nodes
int Node::indexInParent() const noexcept {
   if (!this->parent)
      return -1;
   assert(this->parent->type == node_type::folder);
   return ((Folder*)this->parent)->indexOf(this);
}

dovah::bsa_archived_file* File::load() const noexcept {
   if (!this->source)
      return nullptr;
   auto full = this->path + '/'  + this->name;
   return this->source->lookup_file(full.toLatin1().constData());
}
void File::setName(const QString& n) {
   this->name = n;
   int i = n.lastIndexOf('.');
   if (i >= 0) {
      this->extension = n.mid(i + 1).toLower();
   } else {
      this->extension.clear();
   }
}

Folder::~Folder() {
   this->clear();
}
void Folder::appendFile(File* file) {
   assert(file->parent == nullptr);
   file->parent = this;
   this->files.push_back(file);
}
void Folder::appendSubfolder(Folder* child) {
   assert(child->parent == nullptr);
   child->parent = this;
   this->subfolders.push_back(child);
}
void Folder::clear() {
   for (auto* node : this->files)
      if (node)
         delete node;
   this->files.clear();
   for (auto* node : this->subfolders)
      if (node)
         delete node;
   this->subfolders.clear();
}
File* Folder::file(const QString& name) const noexcept {
   for (auto* node : this->files)
      if (node->name.compare(name, Qt::CaseInsensitive) == 0)
         return node;
   return nullptr;
}
int Folder::indexOf(const Node* n) const noexcept {
   if (n->type == node_type::file) {
      int i = this->files.indexOf((File*)n);
      if (i >= 0)
         return i + this->subfolders.size();
      return -1;
   }
   if (n->type == node_type::folder) {
      return this->subfolders.indexOf((Folder*)n);
   }
   return -1;
}
Folder* Folder::subfolder(const QString& name) const noexcept {
   for (auto* node : this->subfolders)
      if (node->name.compare(name, Qt::CaseInsensitive) == 0)
         return node;
   return nullptr;
}
#pragma endregion

#pragma region DKBSACollectionModelBackend
DKBSACollectionModelBackend::DKBSACollectionModelBackend(QObject* parent) : QObject(parent) {
}

void DKBSACollectionModelBackend::clear() {
   emit this->aboutToClear();
   this->_root.clear();
   this->_archives.clear();
   emit this->cleared();
}
void DKBSACollectionModelBackend::importFromArchive(const dovah::bsa_archive* bsa) {
   bsa->for_each_folder([this, bsa](const dovah::bsa_archive::folder_entry& folder) {
      auto& name         = folder.name;
      auto* known_folder = this->_folderByPath(name);
      bsa->for_each_file_in_folder(folder, [this, known_folder, bsa](const dovah::bsa_archive::folder_entry& folder, const dovah::bsa_archive::file_entry& file) {
         auto  fn = QString::fromLatin1(file.name.c_str());
         auto* mf = known_folder->file(fn);
         if (mf) {
            mf->source = bsa;
            emit this->fileSourceChanged(mf);
         } else {
            mf = new File;
            mf->setName(fn);
            mf->path   = QString::fromLatin1(folder.name.c_str());
            mf->source = bsa;
            known_folder->appendFile(mf);
         }
         return false;
      });
      return false;
   });
   emit this->archiveImported();
}
void DKBSACollectionModelBackend::setArchives(const dovah::bsa_load_order& lo) {
   emit this->aboutToReplace();
   this->_root.clear();
   this->_archives.clear();
   {
      const auto& list = lo.get_archive_list();
      this->_archives.reserve(list.size());
      for (const auto* file : list) {
         this->_archives.push_back(file);
         this->_importFromArchiveInList(file);
      }
   }
   emit this->replaced();
}

Folder* DKBSACollectionModelBackend::_folderByPath(const std::string& path) {
   Folder* target = &this->_root;
   QString name;
   for (const char c : path) {
      if (c == '/' || c == '\\') {
         if (name.isEmpty())
            continue;
         auto* sub = target->subfolder(name);
         if (!sub) {
            sub = new Folder;
            sub->name = name;
            target->appendSubfolder(sub);
         }
         target = sub;
      }
   }
   if (!name.isEmpty()) {
      auto* sub = target->subfolder(name);
      if (!sub) {
         sub = new Folder;
         sub->name = name;
         target->appendSubfolder(sub);
      }
      target = sub;
   }
   return target;
}
void DKBSACollectionModelBackend::_importFromArchiveInList(const dovah::bsa_archive* bsa) {
   bsa->for_each_folder([this, bsa](const dovah::bsa_archive::folder_entry& folder) {
      const auto& name = folder.name;
      auto* modeled = this->_folderByPath(name);
      bsa->for_each_file_in_folder(folder, [modeled, bsa](const dovah::bsa_archive::folder_entry& folder, const dovah::bsa_archive::file_entry& file) {
         auto  fn = QString::fromLatin1(file.name.c_str());
         auto* mf = modeled->file(fn);
         if (!mf) {
            mf = new File;
            mf->setName(fn);
            mf->path = QString::fromLatin1(folder.name.c_str());
            modeled->appendFile(mf);
         }
         mf->source = bsa;
         return false;
      });
      return false;
   });
}
#pragma endregion

#pragma region DKBSACollectionModel
DKBSACollectionModel::DKBSACollectionModel(QObject* parent) : QAbstractItemModel(parent) {
   if (_defaultBackend)
      this->setBackend(_defaultBackend);
}

/*static*/ DKBSACollectionModelBackend* DKBSACollectionModel::_defaultBackend = nullptr;
/*static*/ void DKBSACollectionModel::setDefaultBackend(DKBSACollectionModelBackend* b) {
   _defaultBackend = b;
   if (b) {
      QObject::connect(b, &QObject::destroyed, b, [b]() {
         if (_defaultBackend == b)
            _defaultBackend = nullptr;
      });
   }
}

const Node* DKBSACollectionModel::_nodeFromIndex(const QModelIndex& index) const noexcept {
   if (!index.isValid())
      if (this->_backend)
         return this->_backend->root();
   return (const Node*)index.internalPointer();
}
const Folder* DKBSACollectionModel::_folderFromIndex(const QModelIndex& index) const noexcept {
   if (auto* node = this->_nodeFromIndex(index)) {
      if (node->type != node_type::folder)
         return nullptr;
      return (const Folder*)node;
   }
   return nullptr;
}

QModelIndex DKBSACollectionModel::index(const Node* node, int col) const noexcept {
   if (!node || !this->_backend || node == this->_backend->root())
      return QModelIndex();
   auto* parent = (Folder*)node->parent;
   if (!parent)
      return QModelIndex();
   assert(parent->type == Node::node_type::folder);
   auto j = parent->indexInParent();
   if (j < 0)
      return this->createIndex(0, col, parent);
   return this->createIndex(j, col, parent);
}
QModelIndex DKBSACollectionModel::indexOfFolder(const QString& path) const noexcept {
   if (!this->_backend)
      return QModelIndex();
   const auto* parent = this->_backend->root();
   QString     name;
   for (QChar c : path) {
      if (c == '/' || c == '\\') {
         if (name.isEmpty())
            continue;
         parent = parent->subfolder(name);
         if (!parent)
            return QModelIndex();
         continue;
      }
      name += c;
   }
   if (!name.isEmpty())
      parent = parent->subfolder(name);
   if (!parent || parent == this->_backend->root())
      return QModelIndex();
   return this->index(parent);
}

bool DKBSACollectionModel::isFile(const QModelIndex& index) const noexcept {
   auto* node = this->_nodeFromIndex(index);
   return node && (node->type == node_type::file);
}
bool DKBSACollectionModel::isFolder(const QModelIndex& index) const noexcept {
   auto* node = this->_nodeFromIndex(index);
   return node && (node->type == node_type::folder);
}

#pragma region QAbstractItemModel overrides
   QModelIndex DKBSACollectionModel::index(int row, int column, const QModelIndex& parent) const {
      if (!this->_backend)
         return QModelIndex();
      if (row < 0)
         return QModelIndex();
      const Folder* folder = nullptr;
      if (!parent.isValid()) {
         folder = this->_backend->root();
      } else {
         folder = this->_folderFromIndex(parent);
         if (!folder)
            return QModelIndex();
      }
      auto& subs = folder->subfolders;
      int   sfc  = subs.size();
      if (row < sfc) {
         return this->createIndex(row, column, subs[row]);
      }
      if (!this->_properties.foldersOnly) {
         auto& files = folder->files;
         int   fr    = row - sfc;
         if (fr < files.size()) {
            return this->createIndex(row, column, files[fr]);
         }
      }
      return QModelIndex();
   }
   QModelIndex DKBSACollectionModel::parent(const QModelIndex& index) const {
      if (!index.isValid())
         return QModelIndex();
      auto* node = this->_nodeFromIndex(index);
      return this->index(node, index.column());
   }
   int DKBSACollectionModel::rowCount(const QModelIndex& parent) const {
      auto* folder = this->_folderFromIndex(parent);
      if (!folder)
         return -1;
      if (this->_properties.foldersOnly)
         return folder->subfolders.size();
      return folder->subfolders.size() + folder->files.size();
   }
   int DKBSACollectionModel::columnCount(const QModelIndex& item) const {
      if (!item.isValid())
         return 0;
      return 1;
   }
   Qt::ItemFlags DKBSACollectionModel::flags(const QModelIndex& index) const {
      Qt::ItemFlags flags = 0;
      if (auto* node = this->_nodeFromIndex(index)) {
         flags |= Qt::ItemFlag::ItemIsEnabled;
         flags |= Qt::ItemFlag::ItemIsSelectable;
         if (node->type != node_type::folder) {
            flags |= Qt::ItemFlag::ItemNeverHasChildren;
         }
      }
      return flags;
   }
   QVariant DKBSACollectionModel::data(const QModelIndex& index, int role) const {
      if (!index.isValid())
         return QVariant();
      auto* node = this->_nodeFromIndex(index);
      if (!node)
         return QVariant();
      auto col = index.column();
      if (col == 0) {
         switch (role) {
            case Qt::ItemDataRole::DisplayRole:
               return node->name;
            case Qt::ItemDataRole::DecorationRole:
               if (node->type == node_type::folder) {
                  return _Icons::get().folderIcon();
               } else if (node->type == node_type::file) {
                  return _Icons::get().iconForExtension(((File*)node)->extension);
               }
               break;
            case FullPathRole:
               if (node->type == node_type::file) {
                  return ((File*)node)->path + '/' + node->name;
               } else {
                  auto*   root = this->_backend->root();
                  QString path = node->name;
                  for (node = node->parent; node && node != root; node = node->parent)
                     path.insert(0, node->name + '/');
                  return path;
               }
               break;
         }
      }
      return QVariant();
   }
   QVariant DKBSACollectionModel::headerData(int section, Qt::Orientation orientation, int role) const {
      return QVariant();
   }
#pragma endregion

dovah::bsa_archived_file* DKBSACollectionModel::loadFileContents(const QModelIndex& index) {
   auto* node = this->_nodeFromIndex(index);
   if (!node || node->type != node_type::file)
      return nullptr;
   return ((File*)node)->load();
}

void DKBSACollectionModel::setBackend(DKBSACollectionModelBackend* backend) {
   if (backend == this->_backend)
      return;
   this->beginResetModel();
   if (this->_backend) {
      QObject::disconnect(this->_backend, nullptr, this, nullptr);
   }
   this->_backend = backend;
   if (backend) {
      QObject::connect(backend, &DKBSACollectionModelBackend::aboutToClear, this, &QAbstractItemModel::beginResetModel);
      QObject::connect(backend, &DKBSACollectionModelBackend::aboutToReplace, this, &QAbstractItemModel::beginResetModel);
      QObject::connect(backend, &DKBSACollectionModelBackend::cleared, this, &QAbstractItemModel::endResetModel);
      QObject::connect(backend, &DKBSACollectionModelBackend::replaced, this, &QAbstractItemModel::endResetModel);
      QObject::connect(backend, &DKBSACollectionModelBackend::fileSourceChanged, this, [this](File* file) {
         auto index = this->index(file);
         if (index.isValid())
            emit this->dataChanged(index, index);
      });
   }
   this->endResetModel();
}
void DKBSACollectionModel::setFoldersOnly(bool b) {
   if (b == this->_properties.foldersOnly)
      return;
   if (this->_backend != nullptr)
      this->beginResetModel();
   this->_properties.foldersOnly = b;
   if (this->_backend != nullptr)
      this->endResetModel();
}
#pragma endregion