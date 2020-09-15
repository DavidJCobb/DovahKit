#pragma once
#include <cstdint>
#include <set>
#include <vector>
#include <QAbstractItemModel>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"

namespace dovah {
   class file_header;
}

class LoadOrderFileListModel;
class LoadOrderFileListModelItem {
   friend LoadOrderFileListModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      //
      QString filename;
      bool    is_master;
      bool    selected = false;
      QString author;
      QString description;
      QVector<QString> dependencies;
      QDateTime created;
      QDateTime modified;
      //
      LoadOrderFileListModelItem() {}
      LoadOrderFileListModelItem(const dovah::file_header& header, const QDateTime& created, const QDateTime& modified);
      //
      inline const QString& name() const noexcept { return this->filename; }
};
class LoadOrderFileListModelRoot : public LoadOrderFileListModelItem {
   friend LoadOrderFileListModel;
   public:
      using item_type = LoadOrderFileListModelItem;
   protected:
      std::vector<item_type*> _children;
      void clear() {
         for (auto* p : this->_children)
            delete p;
         this->_children.clear();
      }
   public:
      inline const std::vector<item_type*>& children() const noexcept { return this->_children; }
      inline item_type* child(size_t i) const noexcept {
         if (i < 0 || i >= this->_children.size())
            return nullptr;
         return this->_children[i];
      }
      inline size_t childCount() const noexcept { return this->_children.size(); }
      inline int indexOf(item_type*) const noexcept;
};

class LoadOrderFileListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = LoadOrderFileListModelItem;
      using root_type = LoadOrderFileListModelRoot;
   protected:
      root_type* root   = nullptr;
      item_type* active = nullptr;
      //
   public:
      LoadOrderFileListModel() {
         this->root = new root_type;
      }
      ~LoadOrderFileListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex index(item_type*) const;
      inline item_type* invisibleRootItem() const noexcept { return this->root; }
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      bool setData(const QModelIndex& index, const QVariant& value, int role) override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void clear();
      void insert(const dovah::file_header&, const QDateTime& created, const QDateTime& modified);
      void build(QString game_install_path);
      //
      inline const item_type* activeFile() const noexcept { return this->active; }
      inline const std::vector<item_type*>& files() const noexcept { return this->root->children(); }
      //
      void setActiveFile(item_type*) noexcept;
};

class LoadOrderFileList : public QTableView {
   Q_OBJECT
   public:
      LoadOrderFileList(QWidget* parent);
      using model_type      = LoadOrderFileListModel;
      using model_item_type = model_type::item_type;
      using form_type_set   = std::set<dovah::form_type_t>;
      //
      inline model_type* unwrappedModel() const noexcept {
         return (model_type*)this->model();
      }
      //
   public slots:
      //
};