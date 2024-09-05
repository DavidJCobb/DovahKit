#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <functional>
#include <optional>
#include <variant>
#include <vector>
#include <QAbstractItemModel>
#include <QString>
#include <QVarLengthArray>
#include "dovah/core.h"
#include "dovah/form_stub.h"

namespace dovah::loaded_forms {
   class Form;
}

class DKFormListPaneModel : public QAbstractTableModel {
   Q_OBJECT;
   public:
      struct Column {
         Column() = delete;
         enum Enum {
            Type   = 0,
            Name   = 1,
            FormID = 2,
         };
      };
      static constexpr const size_t ColumnCount = 3;

      class Item {
         friend DKFormListPaneModel;
         public:
            static constexpr const size_t cached_builtin_column_count = 2;

            static constexpr std::optional<size_t> cache_index_for_builtin_column(Column::Enum);
            static constexpr size_t cache_index_for_extra_column(size_t extra_col_index);

         public:
            dovah::form_stub* stub = nullptr;
            QVarLengthArray<QString, cached_builtin_column_count> columnText;
            
            Item();
            Item(dovah::form_stub*);

            QString computeEditorID() const;
            QString computeSignature() const;

            QString getCachedEditorID() const;
            QString getCachedSignature() const;
      };

      using extra_column_handler_by_stub = std::function<QString(const dovah::form_stub&)>;
      using extra_column_handler_by_data = std::function<QString(const dovah::loaded_forms::Form&)>;
      //
      using extra_column_handler = std::variant<extra_column_handler_by_stub, extra_column_handler_by_data>;

   protected:
      struct ExtraColumnInfo {
         extra_column_handler handler;
         QString              header_text;
      };
      
   protected:
      QVector<Item*> children;
      QVector<dovah::form_type> allowed_form_types; // if empty, then no limit
      bool allow_dupes  = false; // allow duplicate entries?
      bool allow_gaps   = true;
      bool show_indices = true;
      struct {
         QVector<ExtraColumnInfo> list;
         bool any_getters_take_loaded_form = false;
      } extra_columns;

      void _emitRowChanged(size_t row);
      void _recacheNewlyAppendedExtraColumn(Item&, extra_column_handler& handler);
      void _recacheItemText(Item&);
      
      // Remove all items for which the decider function returns true.
      void _pruneItems(std::function<bool(const Item&)> decider_function);

      void _clear(bool silent = false);
      
   protected slots:
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formModified(dovah::form_stub*);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      void formsRenumberedEnMasse();
      
   public:
      DKFormListPaneModel(QObject* parent = nullptr);
      ~DKFormListPaneModel() {
         this->_clear(true);
      }

      QVector<dovah::form_stub*> stubs() const noexcept;

      #pragma region Property getters
         constexpr bool allowGaps() const noexcept { return this->allow_gaps; }
         constexpr const QVector<dovah::form_type>& allowedFormTypes() const noexcept { return this->allowed_form_types; }
         constexpr bool showIndices() const noexcept { return this->show_indices; }
      #pragma endregion
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual int rowCount(const QModelIndex& parent) const override;
            virtual int columnCount(const QModelIndex& item) const override;
         #pragma endregion
         #pragma region Data
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            virtual QVariant data(const QModelIndex& index, int role) const override;
            virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
         #pragma endregion

         // Qt's design for this API is unintuitive, so (moveStubs) is provided as an alternative.
         virtual bool moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) override;

         #pragma region Drag-and-drop
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
            virtual QStringList mimeTypes() const override;
            virtual Qt::DropActions supportedDropActions() const override;
         #pragma endregion
      #pragma endregion
      
      
   public slots:
      void addStub(dovah::form_stub* s);
      void addStubs(const std::vector<dovah::form_stub*>&, int at = -1);
      void clear();
      int indexOfStub(const dovah::form_stub*) const;
      void moveStubs(QModelIndexList, int down);
      void removeStub(int index);
      void removeStubs(QVector<int> indices);
      void removeStubs(QModelIndexList);
      inline void reserve(int i) { this->children.reserve(i); }

      #pragma region Property setters
         void setAllowDuplicates(bool);
         void setAllowedFormTypes(QVector<dovah::form_type>);
         void setAllowGaps(bool);
         void setShowIndices(bool);
      #pragma endregion
         
   public: // Ensure these are not Qt slots; slots can't have moved&& parameters
      void addExtraColumn(QString header, extra_column_handler&&);
      void removeExtraColumn(size_t index);
};

#include "./DKFormListPaneModel.inl"