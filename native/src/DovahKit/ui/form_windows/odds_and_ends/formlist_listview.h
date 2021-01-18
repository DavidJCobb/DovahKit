#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QTableView>
#include <QString>
#include "../../../dovah/core.h"
#include "../../../dovah/form_stub.h"

//
// Designed for showing the contents of a FormList, but built to be generic. I may go all 
// the way and make this a generic widget. We'll see.
//

class FormListListviewModel;
class FormListListviewModelItem {
   friend FormListListviewModel;
   public:
      using bare_form_id_t = dovah::bare_form_id_t;
      using form_stub      = dovah::form_stub;
      //
      form_stub* stub = nullptr;
      QString signature;
      QString editorID;
      //
      FormListListviewModelItem() {}
      FormListListviewModelItem(form_stub*);
      void updateFromStub(); // update the form's identifying information, e.g. its editor ID
};

class FormListListviewModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type   = FormListListviewModelItem;
      using form_stub   = dovah::form_stub;
      using form_type_t = dovah::form_type_t;
   protected:
      QVector<item_type*>  children;
      QVector<item_type*>  queued_additions;
      QVector<form_type_t> allowed_form_types; // if empty, then no limit
      bool allow_gaps = true;
      //
      void addStub(dovah::form_stub*, bool queued);
      void removeStub(item_type*);
      void updateStub(item_type*);
      void _pruneItems(std::function<bool(const item_type&)>);
      //
   protected slots:
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      void formsRenumberedEnMasse();
      //
   public:
      FormListListviewModel(QObject* parent = nullptr);
      ~FormListListviewModel() {
         this->clear();
      }
      
      inline void addStub(dovah::form_stub* s) { this->addStub(s, false); }
      inline bool allowGaps() const noexcept { return this->allow_gaps; }
      inline const QVector<form_type_t>& allowedFormTypes() const noexcept { return this->allowed_form_types; }
      void moveStubs(QModelIndexList, int down);
      void removeStub(int index);
      void removeStubs(QVector<int> indices);
      void removeStubs(QModelIndexList);
      inline void reserve(int i) { this->children.reserve(i); }
      void setAllowedFormTypes(QVector<form_type_t>);
      void setAllowGaps(bool);
      QVector<dovah::form_stub*> stubs() const noexcept;
      
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      inline const item_type* row(int rowIndex) const noexcept;
      
      //
      // don't call this directly; Qt's design for this API is unintuitive; you WILL screw up
      //
      // use (moveStubs) instead
      //
      virtual bool moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) override;
      
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
      bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
      virtual QStringList mimeTypes() const override;
      Qt::DropActions supportedDropActions() const;
      
   public slots:
      void clear();
};

class FormListListview : public QTableView {
   Q_OBJECT
   public:
      FormListListview(QWidget* parent);
      using model_type        = FormListListviewModel;
      using model_item_type   = model_type::item_type;
      //
   public slots:
      void addStub(dovah::form_stub* stub);
      void clear();
      void reserve(size_t);
      void setTarget(const dovah::form_stub* stub);
      QVector<dovah::form_stub*> stubs() const noexcept;
      //
      void moveSelected(int down); // negative values move up
      void removeSelected();
      //
      inline model_type* fullModel() {
         return (model_type*)this->model();
      }
      //
   protected:
      const dovah::form_stub* target = nullptr;
      //
      virtual void keyPressEvent(QKeyEvent* event) override;
};