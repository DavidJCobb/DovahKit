#pragma once
#include <vector>
#include <QAbstractItemModel>
#include <QFrame>
#include <QPushButton>
#include <QTableView>

class DKFormListPaneModel;
#if !defined(QT_DESIGNER_LIB)
   #include "../dovah/core.h"
   #include "widget-models/DKFormListPaneModel.h"
#endif

namespace dovah {
   class form_stub;
   namespace loaded_forms {
      class Form;
   }
}

class DKFormListPane : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE true);
   Q_PROPERTY(bool showFormTypes    READ showFormTypes    WRITE setShowFormTypes    DESIGNABLE true);
   Q_PROPERTY(bool showIndices      READ showIndices      WRITE setShowIndices      DESIGNABLE true);
   Q_PROPERTY(bool showMoveButtons  READ showMoveButtons  WRITE setShowMoveButtons  DESIGNABLE true);
   Q_PROPERTY(bool showRemoveButton READ showRemoveButton WRITE setShowRemoveButton DESIGNABLE true);
   public:
      DKFormListPane(QWidget* parent);
      
      static constexpr int ColumnType   = 0;
      static constexpr int ColumnName   = 1;
      static constexpr int ColumnFormID = 2;

      inline Qt::Orientation orientation() const noexcept { return this->state.orientation; }
      inline bool showFormTypes() const noexcept { return this->state.show_form_types; }
      inline bool showIndices() const noexcept { return this->state.show_indices; }
      inline bool showMoveButtons() const noexcept { return this->state.show_move_buttons; }
      inline bool showRemoveButton() const noexcept { return this->state.show_remove_button; }

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
         void reserve(size_t);
      #endif

      #if !defined(QT_DESIGNER_LIB)
         void setAllowedFormTypes(QVector<dovah::form_type_t>);
      #endif
      void setShowFormTypes(bool);
      void setShowIndices(bool);
      void setOrientation(Qt::Orientation);
      void setShowMoveButtons(bool);
      void setShowRemoveButton(bool);

   signals:

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