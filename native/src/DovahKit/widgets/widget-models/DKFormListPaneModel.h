#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <QAbstractItemModel>
#include <QString>
#include "../../dovah/core.h"
#include "../../dovah/form_stub.h"

class DKFormListPaneModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      class Item {
         friend DKFormListPaneModel;
         public:
            using bare_form_id_t = dovah::bare_form_id_t;
            using form_stub      = dovah::form_stub;
            
            form_stub* stub = nullptr;
            QString signature;
            QString editorID;
            
            Item() {}
            Item(form_stub*);
            void updateFromStub(); // update the form's identifying information, e.g. its editor ID
      };

      using form_stub = dovah::form_stub;
      using form_type = dovah::form_type;
      
      static constexpr int ColumnType   = 0;
      static constexpr int ColumnName   = 1;
      static constexpr int ColumnFormID = 2;
      
   protected:
      QVector<Item*> children;
      QVector<Item*> queued_additions;
      QVector<form_type> allowed_form_types; // if empty, then no limit
      bool allow_gaps   = true;
      bool show_indices = true;
      
      void _addStub(dovah::form_stub*, bool queued);
      void _removeStub(Item*);
      void _updateStub(Item*);
      void _pruneItems(std::function<bool(const Item&)>);
      
   protected slots:
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      void formsRenumberedEnMasse();
      
   public:
      DKFormListPaneModel(QObject* parent = nullptr);
      ~DKFormListPaneModel() {
         this->clear();
      }

      QVector<dovah::form_stub*> stubs() const noexcept;

      #pragma region Property getters
         inline bool allowGaps() const noexcept { return this->allow_gaps; }
         inline QVector<form_type> allowedFormTypes() const noexcept { return this->allowed_form_types; }
         inline bool showIndices() const noexcept { return this->show_indices; }

         inline const QVector<form_type>& constAllowedFormTypes() const noexcept { return this->allowed_form_types; }
      #pragma endregion
      
      #pragma region QAbstractItemModel overrides
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
         inline const Item* row(int rowIndex) const noexcept;

         // Qt's design for this API is unintuitive, so (moveStubs) is provided as an alternative.
         virtual bool moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) override;
      #pragma endregion
      
      #pragma region Drag-and-drop
         bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
         bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
         virtual QStringList mimeTypes() const override;
         Qt::DropActions supportedDropActions() const;
      #pragma endregion
      
   public slots:
      inline void addStub(dovah::form_stub* s) { this->_addStub(s, false); }
      void clear();
      void moveStubs(QModelIndexList, int down);
      void removeStub(int index);
      void removeStubs(QVector<int> indices);
      void removeStubs(QModelIndexList);
      inline void reserve(int i) { this->children.reserve(i); }

      #pragma region Property setters
         void setAllowedFormTypes(QVector<form_type>);
         void setAllowGaps(bool);
         void setShowIndices(bool);
      #pragma endregion
};