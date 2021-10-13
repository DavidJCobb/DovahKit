#pragma once
#include <QAbstractItemModel>
#include <string>
#include "../../dovah/files/bsa/bsa_archive.h"

namespace dovah {
   class bsa_load_order;
   class bsa_archived_file;
}

//
// This class should be used to cache the contents of all loaded BSAs in a form that 
// can most efficiently be worked with by models and by Qt. You should create one of 
// these for the whole program, and share it among all models.
//
class DKBSACollectionModelBackend : public QObject {
   Q_OBJECT;
   public:
      DKBSACollectionModelBackend(QObject* parent = nullptr);
      
      class Node {
         public:
            enum class node_type {
               file,
               folder,
            };
         public:
            Node(node_type t) : type(t) {}

            const node_type type;
            Node*   parent = nullptr;
            QString name;

            int indexInParent() const noexcept;
      };
      class File : public Node {
         public:
            File() : Node(node_type::file) {}

            const dovah::bsa_archive* source = nullptr; // last-imported archive to provide this file
            const dovah::bsa_archive::folder_entry* raw_folder = nullptr;
            QString extension;

            dovah::bsa_archived_file* load() const noexcept;
            void setName(const QString&);
      };
      class Folder : public Node {
         public:
            Folder() : Node(node_type::folder) {}
            ~Folder();

            QVector<Folder*> subfolders;
            QVector<File*>   files;

            void appendSubfolder(Folder*);
            void appendFile(File*);
            void clear();
            File* file(const QString& name) const noexcept;
            int indexOf(const Node*) const noexcept;
            Folder* subfolder(const QStringView& name) const noexcept;

            void absorb(Folder&); // assumes both folders are already sorted; exists in case we wanna try multithreading
            void recursiveSort();
            void sort();
      };

      inline const Folder* root() const noexcept { return &this->_root; }

   signals:
      void aboutToClear();
      void aboutToReplace();
      void cleared();
      void replaced();

      void archiveImported();
      void fileSourceChanged(File*);

   public slots:
      void clear();
      void importFromArchive(const dovah::bsa_archive*);
      void setArchives(const dovah::bsa_load_order&);

   protected:
      QVector<const dovah::bsa_archive*> _archives;
      Folder _root;

      void _importFromArchiveInList(const dovah::bsa_archive*);

      // get or create folder; used when importing an archive's contents
      Folder* _folderByPath(const QString&);
};

//
// This class wraps DKBSACollectionModelBackend and makes it available within Qt's 
// model/view system. This model type contains very basic filtering features, though 
// those features rely on begin/endResetModel.
// 
// This class is lightweight, storing no data other than its own configuration and 
// a pointer to its backend: it pulls all data from DKBSACollectionModelBackend. You 
// can set a single DKBSACollectionModelBackend to serve as the "default instance," 
// used for all DKBSACollectionModels created in the future, and then you cna create 
// a single model for each widget (file picker, file browser, etc.) that needs one.
//
class DKBSACollectionModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      DKBSACollectionModel(QObject* parent = nullptr);

      static constexpr auto FullPathRole = (Qt::ItemDataRole)(Qt::UserRole + 1); // only fast for files

      using Node   = DKBSACollectionModelBackend::Node;
      using File   = DKBSACollectionModelBackend::File;
      using Folder = DKBSACollectionModelBackend::Folder;

      static void setDefaultBackend(DKBSACollectionModelBackend*);

      inline DKBSACollectionModelBackend* backend() const noexcept { return this->_backend; }
      QModelIndex index(const Node*, int col = 0) const noexcept;
      QModelIndex indexOfFile(const QString&, const QModelIndex& inFolder) const noexcept;
      QModelIndex indexOfFolder(const QString&) const noexcept; // if path starts with "../", fails
      QModelIndex indexOfFolder(const QString&, const QModelIndex& relativeTo) const noexcept;

      bool isFile(const QModelIndex&) const noexcept;
      bool isFolder(const QModelIndex&) const noexcept;
      
      #pragma region QAbstractItemModel overrides
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      dovah::bsa_archived_file* loadFileContents(const QModelIndex&);

   public slots:
      void setBackend(DKBSACollectionModelBackend*);
      void setFoldersOnly(bool);

   protected:
      DKBSACollectionModelBackend* _backend = nullptr;
      struct {
         bool foldersOnly = false;
      } _properties;

      const Node*   _nodeFromIndex(const QModelIndex& index) const noexcept;
      const Folder* _folderFromIndex(const QModelIndex& index) const noexcept;

      static DKBSACollectionModelBackend* _defaultBackend;
};