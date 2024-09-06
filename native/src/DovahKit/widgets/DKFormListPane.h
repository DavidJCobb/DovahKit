#pragma once
#include <optional>
#if !defined(QT_DESIGNER_LIB)
   #include <functional>
   #include <variant>
#endif
#include <vector>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QFrame>
#include <QPushButton>
#include <QTableView>

class DKFormListPaneModel;
#if !defined(QT_DESIGNER_LIB)
   #include "../dovah/core.h"
   #include "widget-models/DKFormListPaneModel.h"
#endif
#if !defined(QT_DESIGNER_LIB)
   class DKFormListPaneCustomFilter;
#endif

namespace dovah {
   class form_stub;
   namespace loaded_forms {
      class Form;
   }
}

class DKFormListPane : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE true USER true);
   Q_PROPERTY(bool allowDuplicates  READ allowDuplicates  WRITE setAllowDuplicates  DESIGNABLE true);
   Q_PROPERTY(bool readOnly         READ readOnly         WRITE setReadOnly         DESIGNABLE true);
   Q_PROPERTY(bool showFormTypes    READ showFormTypes    WRITE setShowFormTypes    DESIGNABLE true);
   Q_PROPERTY(bool showIndices      READ showIndices      WRITE setShowIndices      DESIGNABLE true);
   Q_PROPERTY(bool showMoveButtons  READ showMoveButtons  WRITE setShowMoveButtons  DESIGNABLE true);
   Q_PROPERTY(bool showRemoveButton READ showRemoveButton WRITE setShowRemoveButton DESIGNABLE true);
   public:
      #if !defined(QT_DESIGNER_LIB)
         //
         // You can add extra columns to the listview, with getters that take a form stub or 
         // a loaded form. Your getters will be invoked when forms are added to the listview 
         // as well as when any forms already in the listview are modified. The strings that 
         // your getters return will be cached.
         //
         using ExtraColumnHandler = std::variant<
            std::function<QString(const dovah::form_stub&)>,
            std::function<QString(const dovah::loaded_forms::Form&)>
         >;
      #endif

   protected:
      static constexpr const int ColumnType   = 0;
      static constexpr const int ColumnName   = 1;
      static constexpr const int ColumnFormID = 2;

   public:
      DKFormListPane(QWidget* parent);
      
      #if !defined(QT_DESIGNER_LIB)
         QHeaderView* horizontalHeader() const noexcept { return this->subwidgets.view->horizontalHeader(); }
      #endif
      constexpr bool allowDuplicates() const noexcept { return this->state.allow_duplicates; }
      constexpr bool allowMultiSelect() const noexcept { return this->state.allow_multi_select; }
      constexpr bool readOnly() const noexcept { return this->state.read_only; }
      constexpr Qt::Orientation orientation() const noexcept { return this->state.orientation; }
      constexpr bool showFormTypes() const noexcept { return this->state.show_form_types; }
      constexpr bool showIndices() const noexcept { return this->state.show_indices; }
      constexpr bool showMoveButtons() const noexcept { return this->state.show_move_buttons; }
      constexpr bool showRemoveButton() const noexcept { return this->state.show_remove_button; }

      #if !defined(QT_DESIGNER_LIB)
         QVector<dovah::form_stub*> stubs() const noexcept;

         // Clear the widget's current contents, and then pull all stubs in the provided list. 
         // Helper function provided for form-editing dialogs.
         void pullStubs(const std::vector<dovah::form_reference_t>&);

         // Overwrite the supplied list with the widget's current contents, modifying use info 
         // for the "owner" form as appropriate. The list should be a member on the loaded form; 
         // a form-editing dialog's "OK" button would use this to commit changes.
         void commitStubs(std::vector<dovah::form_reference_t>&, dovah::loaded_forms::Form& owner);
      #endif

   public slots:
      #if !defined(QT_DESIGNER_LIB)
         void addStub(dovah::form_stub* stub);
         void clear();
         bool contains(const dovah::form_stub*) const;
         int  indexOf(const dovah::form_stub*) const;
         void reserve(size_t);
      #endif

      void setAllowDuplicates(bool);
      void setAllowMultiSelect(bool);
      void setReadOnly(bool);
      #if !defined(QT_DESIGNER_LIB)
         void setAllowedFormTypes(QVector<dovah::form_type>);
      #endif
      void setShowFormTypes(bool);
      void setShowIndices(bool);
      void setOrientation(Qt::Orientation);
      void setShowMoveButtons(bool);
      void setShowRemoveButton(bool);
      
   public: // Ensure these are not Qt slots; slots can't have moved&& parameters
      #if !defined(QT_DESIGNER_LIB)
         void addExtraColumn(QString header, ExtraColumnHandler&&);
         void removeExtraColumn(size_t which); // the first-added extra column is index 0. indices shift as columns are removed.
      #endif

      #if !defined(QT_DESIGNER_LIB)
         DKFormListPaneCustomFilter* customFilter() const;
         void setCustomFilter(DKFormListPaneCustomFilter* v);
      #endif

      #if !defined(QT_DESIGNER_LIB)
         [[nodiscard]] std::vector<dovah::form_stub*> selectedForms() const;
         [[nodiscard]] std::vector<size_t> selectedRows() const;
      #endif

   signals:
      void formsAdded(size_t count);
      void formsRemoved(size_t count);
      void selectedFormsChanged(const std::vector<dovah::form_stub*>&);
      void selectedRowsChanged(const std::vector<size_t>&);

   protected:
      struct {
         struct {
            QWidget*     wrapper   = nullptr;
            QPushButton* move_up   = nullptr;
            QPushButton* move_down = nullptr;
            QPushButton* remove    = nullptr;
         } buttons;
         QTableView* view = nullptr;
      } subwidgets;
      struct {
         bool allow_duplicates   = false;
         bool allow_multi_select = true;
         bool read_only          = false;
         bool show_form_types    = true;
         bool show_indices       = false;
         bool show_move_buttons  = true;
         bool show_remove_button = true;
         Qt::Orientation orientation = Qt::Orientation::Horizontal;
      } state;

      DKFormListPaneModel* _model() const noexcept;

      #if !defined(QT_DESIGNER_LIB)
         void _moveSelected(int down); // negative values move up
         void _removeSelected();
      #endif
      void _updateButtonVisibility();
      void _updateOrientation();

      virtual void keyPressEvent(QKeyEvent* event) override;
};