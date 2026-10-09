#pragma once
#include <QAbstractItemModel>
#include "dovah/bare_form_id_t.h"
#include "dovah/form_types.h"
namespace dovah {
   class form_stub;
}
namespace ui::object_window {
   class form_model_item;
}

namespace ui::object_window {
   class form_table_source_model final : public QAbstractTableModel {
      Q_OBJECT
      public:
         using item_type     = form_model_item;
         using form_stub     = dovah::form_stub;
         using form_type_set = QVector<dovah::form_type>;

         struct Column {
            Column() = delete;
            enum {
               EditorID,
               FormID,
               UserCount,

               _COUNT
            };
         };
         static constexpr const size_t ColumnCount = Column::_COUNT;
         
         static constexpr const Qt::ItemDataRole RawDataRole        = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole);
         static constexpr const Qt::ItemDataRole FilterableTextRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1);
         
      protected:
         form_type_set       form_types; // list of all form types that the Object Window should be capable of displaying under any circumstance
         QVector<item_type*> children;
         QVector<form_stub*> forms_pending_use_info_update;
         
         void doUseInfoUpdate();
         
      protected slots:
         void formCreated(form_stub*);
         void formModified(const form_stub*);
         void formModificationImminent(const form_stub*);
         void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
         void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);

         void _emit_data_changed_on(const dovah::form_stub&);
      
      public slots:
         void clear();
         
      public:
         form_table_source_model(QObject* parent = nullptr);
         ~form_table_source_model() {
            this->clear();
         }

         static const item_type* data_for_qmi(const QModelIndex& qmi) noexcept {
            return (item_type*) qmi.internalPointer();
         }

         QModelIndex index(dovah::form_stub*) const;

         #pragma region QAbstractItemModel overrides
            #pragma region Hierarchy
               virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
               virtual QModelIndex parent(const QModelIndex& index) const;
               virtual int rowCount(const QModelIndex& parent) const override;
               virtual int columnCount(const QModelIndex& item) const override;
            #pragma endregion
            #pragma region Item data
               virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
               virtual QVariant data(const QModelIndex& index, int role) const override;
            #pragma endregion
            virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
            #pragma region Drag and drop
               virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
               virtual QStringList mimeTypes() const override;
            #pragma endregion
         #pragma endregion
         
         void rebuild();
         void setBaseFormTypes(const form_type_set&);
         
         const item_type* dataAtRow(int) const noexcept;
   };
}