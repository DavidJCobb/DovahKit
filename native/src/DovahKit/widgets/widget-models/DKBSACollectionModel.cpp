#include "DKBSACollectionModel.h"
#include <QApplication>
#include <QIcon>
#include <QMimeDatabase>
#include <QStyle>
#include "../../dovah/files/bsa/bsa_archive.h"
#include "../../dovah/files/bsa/bsa_load_order.h"

#ifdef _WIN32
   #include "helpers/windows.h"
   #include <QtWin>
#endif

namespace {
   using Node   = DKBSACollectionModelBackend::Node;
   using File   = DKBSACollectionModelBackend::File;
   using Folder = DKBSACollectionModelBackend::Folder;
   using node_type = Node::node_type;

   #ifdef _WIN32
   QIcon _get_icon_from_windows(const QString& extension) {
      constexpr std::array sizes      = { 0, SHGFI_SMALLICON, SHGFI_LARGEICON, SHGFI_SHELLICONSIZE };
      constexpr std::array win_states = { 0,             SHGFI_SELECTED };
      constexpr std::array qt_states  = { QIcon::Normal, QIcon::Selected };
      static_assert(win_states.size() == qt_states.size());
      //
      QIcon icon;
      //
      #ifdef UNICODE
         auto extension_win = extension.toStdWString();
      #else
         auto extension_win = extension.toStdString();
      #endif
      extension_win.insert(extension_win.begin(), decltype(extension_win)::value_type('.'));
      //
      SHFILEINFO info = {};
      for (auto s : sizes) {
         for (size_t i = 0; i < win_states.size(); ++i) {
            #ifdef UNICODE
            HRESULT hr = SHGetFileInfoW(extension_win.c_str(), FILE_ATTRIBUTE_NORMAL, &info, sizeof(info), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | s | win_states[i]);
            #else
            HRESULT hr = SHGetFileInfoA(extension_win.c_str(), FILE_ATTRIBUTE_NORMAL, &info, sizeof(info), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | s | win_states[i]);
            #endif
            if (!SUCCEEDED(hr))
               continue;
            if (info.hIcon == NULL)
               continue;
            auto pm = QtWin::fromHICON(info.hIcon);
            DestroyIcon(info.hIcon);
            if (pm.isNull())
               continue;
            icon.addPixmap(pm, qt_states[i]);
         }
      }
      //
      return icon;
   }
   #endif

   class _Icons {
      private:
         _Icons() {
            auto* style = QApplication::style();
            this->generic_icons.file   = style->standardIcon(QStyle::SP_FileIcon);
            this->generic_icons.folder = style->standardIcon(QStyle::SP_DirIcon);
            //
            // Preload common extensions:
            //
            this->iconForExtension("dds");
            this->iconForExtension("hkx");
            this->iconForExtension("nif");
            this->iconForExtension("pex");
            this->iconForExtension("psc");
            this->iconForExtension("txt");
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
            #ifdef _WIN32
               if (icon.isNull()) {
                  icon = _get_icon_from_windows(extension);
               }
            #endif
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
   return this->parent->indexOf(this);
}

dovah::bsa_archived_file* File::load() const noexcept {
   if (!this->source || !this->raw_folder)
      return nullptr;
   auto full = this->raw_folder->name;
   full += '/';
   full += (const char*)this->name.toLatin1();
   return this->source->lookup_file(full);
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
      if (name == node->name)
         return node;
   return nullptr;
}
File* Folder::file(const QString& name, int stop_at) const noexcept {
   size_t size = std::min(std::max(0, stop_at), this->files.size());
   for (size_t i = 0; i < size; ++i)
      if (name == this->files[i]->name)
         return this->files[i];
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
Node* Folder::node(int i) const noexcept {
   if (i < 0)
      return nullptr;
   auto s = this->subfolders.size();
   if (i >= s) {
      i -= s;
      if (i >= this->files.size())
         return nullptr;
      return this->files[i];
   }
   return this->subfolders[i];
}
Folder* Folder::subfolder(const QStringView& name) const noexcept {
   for (auto* node : this->subfolders)
      if (name == node->name)
         return node;
   return nullptr;
}

void Folder::absorb(Folder& other) {
   if (this->subfolders.empty()) {
      std::swap(this->subfolders, other.subfolders);
      for (auto* f : this->subfolders)
         f->parent = this;
   } else if (!other.subfolders.empty()) {
      QVector<Folder*> sorted;
      //
      size_t i  = 0;
      size_t as = this->subfolders.size();
      for (auto* folder : other.subfolders) {
         if (i < as) {
            auto* a = this->subfolders[i];
            auto* b = folder;
            while (a->name < b->name && i < as) {
               sorted.push_back(a);
               if (++i >= as)
                  break;
               a = this->subfolders[i];
            }
         }
         sorted.push_back(folder);
      }
      for (auto* f : sorted)
         f->parent = this;
      other.subfolders.clear();
      std::swap(this->subfolders, sorted);
   }
   //
   if (this->files.empty()) {
      std::swap(this->files, other.files);
      for (auto* f : this->files)
         f->parent = this;
   } else if (!other.files.empty()) {
      QVector<File*> sorted;
      //
      size_t i  = 0;
      size_t as = this->files.size();
      for (auto* file : other.files) {
         if (i < as) {
            auto* a = this->files[i];
            auto* b = file;
            while (a->name < b->name && i < as) {
               sorted.push_back(a);
               if (++i >= as)
                  break;
               a = this->files[i];
            }
         }
         sorted.push_back(file);
      }
      for (auto* f : sorted)
         f->parent = this;
      other.files.clear();
      std::swap(this->files, sorted);
   }
}
void Folder::recursiveSort() {
   qSort(this->subfolders.begin(), this->subfolders.end(), [](const Folder* a, const Folder* b) { return a->name < b->name; });
   qSort(this->files.begin(), this->files.end(), [](const File* a, const File* b) { return a->name < b->name; });
   //
   for (auto* sf : this->subfolders)
      sf->recursiveSort();
}
void Folder::sort() {
   qSort(this->subfolders.begin(), this->subfolders.end(), [](const Folder* a, const Folder* b) { return a->name < b->name; });
   qSort(this->files.begin(), this->files.end(), [](const File* a, const File* b) { return a->name < b->name; });
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
   const auto& folders = bsa->folder_list();
   for (auto& raw_folder : folders) {
      auto  folder_path = QString::fromLatin1(raw_folder.name.c_str());
      auto* our_folder  = this->_folderByPath(folder_path);
      //
      auto prior_count = our_folder->files.size();
      our_folder->files.reserve(raw_folder.files.size());
      //
      for (const auto& raw_file : raw_folder.files) {
         auto  fn = QString::fromLatin1(raw_file.name.c_str());
         auto* mf = our_folder->file(fn, prior_count);
         if (mf) {
            mf->source = bsa;
            emit this->fileSourceChanged(mf);
         } else {
            mf = new File;
            mf->setName(fn);
            our_folder->appendFile(mf);
         }
         mf->source     = bsa;
         mf->raw_folder = &raw_folder;
      }
      our_folder->sort();
   }
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
   this->_root.recursiveSort();
   emit this->replaced();
}

namespace {
   int _indexOfSlash(const QString& path, int from = 0) {
      return path.indexOf(dovah::bsa_archive::path_separator, from);
   }
}
Folder* DKBSACollectionModelBackend::_folderByPath(const QString& path) {
   Folder* basis = &this->_root;
   bool    prior = true;
   int     last  = 0;
   int     next  = _indexOfSlash(path);
   for (; next >= 0; last = next + 1, next = _indexOfSlash(path, last)) {
      auto name = QStringView(path.data() + last, next - last);
      if (!name.size())
         continue;
      if (prior) {
         if (auto* sub = basis->subfolder(name)) {
            basis = sub;
            continue;
         }
         prior = false;
      }
      auto* sub = new Folder;
      sub->name = name.toString();
      basis->appendSubfolder(sub);
      basis = sub;
   }
   if (last >= 0 && last < path.size()) {
      auto name = QStringView(path.data() + last, path.size() - last);
      if (name.size()) {
         Folder* sub = nullptr;
         if (prior)
            sub = basis->subfolder(name);
         if (!sub) {
            sub = new Folder;
            sub->name = name.toString();
            basis->appendSubfolder(sub);
         }
         basis = sub;
      }
   }
   return basis;
}
void DKBSACollectionModelBackend::_importFromArchiveInList(const dovah::bsa_archive* bsa) {
   const auto& folders = bsa->folder_list();
   for (auto& raw_folder : folders) {
      auto  folder_path = QString::fromLatin1(raw_folder.name.c_str());
      auto* our_folder  = this->_folderByPath(folder_path);
      //
      auto prior_count = our_folder->files.size();
      our_folder->files.reserve(raw_folder.files.size());
      //
      for (const auto& raw_file : raw_folder.files) {
         auto  fn = QString::fromLatin1(raw_file.name.c_str());
         auto* mf = our_folder->file(fn, prior_count);
         if (!mf) {
            mf = new File;
            mf->setName(fn);
            our_folder->appendFile(mf);
         }
         mf->source     = bsa;
         mf->raw_folder = &raw_folder;
      }
   }
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
   auto i = index.row();
   if (!index.isValid()) {
      if (this->_backend)
         return this->_backend->root();
      return nullptr;
   }
   const Folder* folder = (const Folder*)index.internalPointer();
   if (!folder)
      return nullptr;
   assert(folder->type == node_type::folder);
   return folder->node(i);
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
   auto* parent = node->parent;
   if (!parent)
      return QModelIndex();
   return this->createIndex(parent->indexOf(node), col, parent);
}
QModelIndex DKBSACollectionModel::indexOfFile(const QString& name, const QModelIndex& inFolder) const noexcept {
   if (!this->_backend)
      return QModelIndex();
   const Folder* folder = nullptr;
   {
      int i = name.lastIndexOf('/');
      int j = name.lastIndexOf('\\');
      i = std::max(i, j);
      if (i >= 0) {
         auto instead = this->indexOfFolder(name.mid(0, i), inFolder);
         folder = this->_folderFromIndex(instead);
      } else {
         folder = this->_folderFromIndex(inFolder);
      }
   }
   if (!folder)
      return QModelIndex();
   auto* file = folder->file(name);
   if (file)
      return this->index(file, inFolder.column());
   return QModelIndex();
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
         name.clear();
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
QModelIndex DKBSACollectionModel::indexOfFolder(const QString& path, const QModelIndex& relativeTo) const noexcept {
   if (!this->_backend)
      return QModelIndex();
   const Folder* basis = this->_backend->root();
   if (!path.startsWith('/')) {
      if (auto* node = this->_nodeFromIndex(relativeTo)) {
         if (node->type != node_type::folder)
            basis = node->parent;
         else
            basis = (Folder*)node;
      }
   }
   if (!basis)
      basis = this->_backend->root();
   //
   const Folder* target = basis;
   QString name;
   for (QChar c : path) {
      if (c == '/' || c == '\\') {
         if (name.isEmpty())
            continue;
         if (name == '.')
            continue;
         if (name == "..") {
            target = target->parent;
         } else {
            target = target->subfolder(name);
         }
         name.clear();
         if (!target)
            return QModelIndex();
         continue;
      }
      name += c;
   }
   if (!name.isEmpty() && name != '.') {
      if (name == "..") {
         target = target->parent;
      } else {
         target = target->subfolder(name);
      }
   }
   if (!target || target == this->_backend->root())
      return QModelIndex();
   return this->index(target);
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
         return this->createIndex(row, column, (void*)folder);
      }
      if (!this->_properties.foldersOnly) {
         if (row - sfc < folder->files.size())
            return this->createIndex(row, column, (void*)folder);
      }
      return QModelIndex();
   }
   QModelIndex DKBSACollectionModel::parent(const QModelIndex& index) const {
      if (!index.isValid())
         return QModelIndex();
      auto* node = this->_nodeFromIndex(index);
      return this->index(node->parent, index.column());
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
      return 1;
   }
   Qt::ItemFlags DKBSACollectionModel::flags(const QModelIndex& index) const {
      Qt::ItemFlags flags = {};
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
            case Qt::ItemDataRole::ToolTipRole:
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
                  auto* file = (File*)node;
                  if (!file->raw_folder)
                     break;
                  return QString::fromLatin1(file->raw_folder->name.c_str()).replace('\\', '/') + '/' + node->name;
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
      QObject::connect(backend, &DKBSACollectionModelBackend::aboutToClear, this, [this]() { this->beginResetModel(); });
      QObject::connect(backend, &DKBSACollectionModelBackend::aboutToReplace, this, [this]() { this->beginResetModel(); });
      QObject::connect(backend, &DKBSACollectionModelBackend::cleared, this, [this]() { this->endResetModel(); });
      QObject::connect(backend, &DKBSACollectionModelBackend::replaced, this, [this]() { this->endResetModel(); });
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