#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <QAbstractItemModel>
#include <QString>
#include <QVector>

namespace dovah {
   class form_stub;
}

class DKRefsInCellModel : public QAbstractListModel {
   Q_OBJECT
   public:
      static constexpr const Qt::ItemDataRole ForcedPrependedRole = Qt::UserRole;

   protected:
      class Item {
         public:
            dovah::form_stub* stub = nullptr;
            struct {
               QString ref;
               QString base;
            } editor_ids;

            bool is_prepended = false;
            QString cached_text;
            
            Item() {}
            Item(dovah::form_stub*);
            void updateFromStub(); // update the form's identifying information, e.g. its editor ID
      };

      QVector<Item*> children;
      dovah::form_stub* parent_cell = nullptr;

      void _clear();
      void _insert_sorted_stub(dovah::form_stub&, bool prepended);
      void _sort();
      
   protected slots:
      void formCreated(dovah::form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formModified(dovah::form_stub*);
      
   public:
      DKRefsInCellModel(QObject* parent = nullptr);
      ~DKRefsInCellModel();

      void addPrependedRef(dovah::form_stub&);
      void removePrependedRef(dovah::form_stub&);
      void clearAllPrependedRefs();

      dovah::form_stub* parentCell() const;
      void setParentCell(dovah::form_stub*);
      
      #pragma region QAbstractItemModel overrides
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion
};