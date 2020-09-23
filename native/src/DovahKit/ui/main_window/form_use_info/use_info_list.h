#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"
#include "../../../dovah/form_stub.h"

class FormUseInfoListModel;
class FormUseInfoListModelItem {
   friend FormUseInfoListModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      using data_t    = dovah::use_info_entry;
      //
      data_t::flags_t       flags       = 0;
      dovah::form_type_t    otherType   = 0;
      uint32_t              countUsed   = 0;
      uint32_t              countPlaced = 0;
      dovah::bare_form_id_t otherID     = 0;
      dovah::form_stub*     otherStub   = nullptr;
      QString signature;
      QString editorID;
      QString parentCell;
      //
      FormUseInfoListModelItem() {}
      FormUseInfoListModelItem(const data_t*);
      void updateFromStub();
};
class FormUseInfoListModelRoot : public FormUseInfoListModelItem {
   friend FormUseInfoListModel;
   public:
      using item_type = FormUseInfoListModelItem;
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

class FormUseInfoListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = FormUseInfoListModelItem;
      using root_type = FormUseInfoListModelRoot;
      enum class relationship_mode {
         invalid        = -1,
         general_only   = 0,
         base_form_only,
      };
   protected:
      root_type* root = nullptr;
      relationship_mode mode = relationship_mode::general_only;
      //
   public:
      FormUseInfoListModel() {
         this->root = new root_type;
      }
      ~FormUseInfoListModel() {
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
      inline const item_type* row(int rowIndex) const noexcept;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void clear();
      void build(const dovah::form_stub* used);
      inline relationship_mode relationshipMode() const noexcept { return this->mode; }
      void setRelationshipMode(relationship_mode) noexcept; // does not rebuild the model
      void updateExistingItem(const dovah::form_stub*);
};

class FormUseInfoListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      FormUseInfoListModelProxy(QObject* parent = nullptr);
      using model_type      = FormUseInfoListModel;
      using model_item_type = model_type::item_type;
      //
      bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
      //
      inline dovah::form_type_t formType() const noexcept { return this->_formType; };
      void setFormType(dovah::form_type_t);
      //
   protected:
      dovah::form_type_t _formType = dovah::form_type::none;
};

class FormUseInfoList : public QTableView {
   Q_OBJECT
   public:
      FormUseInfoList(QWidget* parent);
      using model_type        = FormUseInfoListModel;
      using model_item_type   = model_type::item_type;
      using relationship_mode = model_type::relationship_mode;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      relationship_mode relationshipMode() const noexcept;
      void setRelationshipMode(relationship_mode) noexcept; // rebuilds the model
      void setTarget(const dovah::form_stub*);
      void setTextFilter(QLineEdit*);
      void setFormTypeFilter(dovah::form_type_t);
      //
   public slots:
      void build();
      void refilterModelByText(const QString&);
      void textFilterChanged();
      void textFilterFinished();
      //
   protected:
      const dovah::form_stub* target = nullptr;
      QLineEdit* _filter         = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
};