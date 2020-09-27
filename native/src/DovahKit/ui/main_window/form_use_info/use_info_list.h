#pragma once
#include <cstdint>
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
   //
   // Given a model which displays all users of a form, this item represents a 
   // user form (as opposed to the used form).
   //
   friend FormUseInfoListModel;
   public:
      using bare_form_id_t = dovah::bare_form_id_t;
      using form_stub      = dovah::form_stub;
      using form_type_t    = dovah::form_type_t;
      using data_t         = dovah::use_info_entry;
      //
      data_t::flags_t flags       = 0;
      form_type_t     otherType   = 0;
      uint32_t        countUsed   = 0;
      uint32_t        countPlaced = 0;
      bare_form_id_t  otherID     = 0;
      form_stub*      otherStub   = nullptr;
      QString signature;
      QString editorID;
      QString parentCell;
      //
      FormUseInfoListModelItem() {}
      FormUseInfoListModelItem(const data_t*);
      void updateFromStub(); // update the form's identifying information, e.g. its editor ID
      void updateUseInfo(const form_stub& used_form); // update the form's use information, e.g. the counts and flags
      void updateUseInfo(const data_t&);
      inline bool isNonUse() const noexcept { return (this->countUsed | this->countPlaced) == 0; }
};

class FormUseInfoListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = FormUseInfoListModelItem;
      using form_stub = dovah::form_stub;
      using use_info_entry = dovah::use_info_entry;
      enum class relationship_mode {
         invalid        = -1,
         general_only   = 0,
         base_form_only,
      };
   protected:
      const form_stub*    used = nullptr; // the form whose uses are being displayed
      relationship_mode   mode = relationship_mode::general_only;
      QVector<item_type*> children;
      QVector<item_type*> potential_severed_uses;
      QVector<item_type*> queued_additions;
      //
      void addUser(const use_info_entry&, bool queued); // takes care of all appropriate filtering based on (relationship_mode) and so on
      void removeUser(item_type*);
      void updateUser(item_type*);
      //
   protected slots:
      void formModificationImminent(const dovah::form_stub*);
      void formModified(const dovah::form_stub*);
      //
   public:
      FormUseInfoListModel(QObject* parent = nullptr);
      ~FormUseInfoListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      inline const item_type* row(int rowIndex) const noexcept;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void build(const dovah::form_stub* used);
      inline relationship_mode relationshipMode() const noexcept { return this->mode; }
      void setRelationshipMode(relationship_mode) noexcept; // does not rebuild the model
      void updateExistingItem(const dovah::form_stub*);
      //
   public slots:
      void clear();
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