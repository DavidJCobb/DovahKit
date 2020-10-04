#pragma once
#include <cstdint>
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
      using form_stub = dovah::form_stub;
      using form_type_set = QVector<dovah::form_type_t>;
      //
   protected:
      form_type_set last_used_form_types;
      QVector<item_type*> children;
      QVector<item_type*> pending_additions;
      QVector<form_stub*> forms_pending_use_info_update;
      //
      void doUseInfoUpdate();
      void insertItem(form_stub*, bool queued);
      //
   protected slots:
      void formCreated(form_stub*);
      void formModified(const form_stub*);
      void formModificationImminent(const form_stub*);
      //
   public slots:
      void clear();
      //
   public:
      FormTableModel(QObject* parent = nullptr);
      ~FormTableModel() {
         this->clear();
      }
      //
      QModelIndex index(dovah::form_stub*) const;
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void rebuild();
      void rebuild(const form_type_set&);
      void setFormTypes(const form_type_set&);
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
      using proxy_type      = FormTableModelProxy;
      using model_item_type = model_type::item_type;
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
      void select(dovah::form_stub*);
      //
   protected:
      BasicFormTypeTree* _source = nullptr;
      QLineEdit* _filter         = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
};