#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include "../../../dovah/core.h"
#include "../../../dovah/form_stub.h"

class FormUseInfoListModel;
class FormUseInfoListModelItem {
   friend FormUseInfoListModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      using data_t    = dovah::use_info_entry;
      //
      data_t::flags_t            flags       = 0;
      dovah::form_type_t         otherType   = 0;
      decltype(data_t::refcount) countUsed   = 0;
      decltype(data_t::refcount) countPlaced = 0;
      dovah::bare_form_id_t      otherID     = 0;
      dovah::form_stub*          otherStub   = nullptr;
      QString signature;
      QString editorID;
      QString parentCell;
      //
      FormUseInfoListModelItem() {}
      FormUseInfoListModelItem(const data_t*);
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
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void clear();
      void build(const dovah::form_stub* used);
      inline relationship_mode relationshipMode() const noexcept { return this->mode; }
      void setRelationshipMode(relationship_mode) noexcept; // does not rebuild the model
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
         return (model_type*)this->model();
      }
      relationship_mode relationshipMode() const noexcept;
      void setRelationshipMode(relationship_mode) noexcept; // rebuilds the model
      void setTarget(const dovah::form_stub*);
      //
   public slots:
      void build();
      //
   protected:
      const dovah::form_stub* target = nullptr;
};