#pragma once
#include <cstdint>
#include <set>
#include <vector>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"

namespace dovah {
   class form_stub;
}
class BasicFormTypeTree;

class FormTableModel;
class FormTableModelItem {
   friend FormTableModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      //
      dovah::form_stub* stub = nullptr;
      QString   editorID;
      form_id_t formID    = 0;
      uint32_t  userCount = 0;
      //
      FormTableModelItem() {}
      FormTableModelItem(dovah::form_stub*);
      //
      inline const QString& name() const noexcept { return this->editorID; }
      void update();
      bool updateUserCount(); // returns true if any changes were made
};
class FormTableModelRoot : public FormTableModelItem {
   friend FormTableModel;
   protected:
      std::vector<FormTableModelItem*> _children;
   public:
      inline const std::vector<FormTableModelItem*>& children() const noexcept { return this->_children; }
      inline FormTableModelItem* child(size_t i) const noexcept {
         if (i < 0 || i >= this->_children.size())
            return nullptr;
         return this->_children[i];
      }
      inline size_t childCount() const noexcept { return this->_children.size(); }
      //
      void clear() {
         for (auto* p : this->_children)
            delete p;
         this->_children.clear();
      }
};

class FormTableModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = FormTableModelItem;
      using root_type = FormTableModelRoot;
      using form_id_t = item_type::form_id_t;
      using form_type_set = std::set<dovah::form_type_t>;
   protected:
      root_type* root = nullptr;
      QVector<dovah::form_stub*> forms_pending_use_info_update;
      //
   public:
      FormTableModel() {
         this->root = new root_type;
      }
      ~FormTableModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      inline item_type* invisibleRootItem() const noexcept { return this->root; }
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void insertItem(dovah::form_stub*);
      void updateExistingItem(const dovah::form_stub*);
      void prepToUpdateUseInfo(const dovah::form_stub* user);
      void doUseInfoUpdate();
      //
      void clear();
      //
      void rebuild(const form_type_set&);
};

class FormTableModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      FormTableModelProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
         this->setFilterCaseSensitivity(Qt::CaseInsensitive);
         this->setFilterRole(Qt::UserRole + 1);
         this->setFilterKeyColumn(-1);
         this->setSortCaseSensitivity(Qt::CaseInsensitive);
         this->setSortRole(Qt::UserRole);
      }
};

class FormTable : public QTableView {
   Q_OBJECT
   public:
      FormTable(QWidget* parent);
      using model_type      = FormTableModel;
      using model_item_type = model_type::item_type;
      using form_type_set   = std::set<dovah::form_type_t>;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      //
      void setFilter(QLineEdit*);
      void setSource(BasicFormTypeTree*);
      //
   public slots:
      void recheckFormTypes();
      void rebuildModel();
      void refilterModel(const QString&);
      void clear();
      //
      void filterChanged();
      void filterFinished();
      //
   protected:
      BasicFormTypeTree* _source = nullptr;
      QLineEdit* _filter         = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
      form_type_set _currentFormTypes;
};