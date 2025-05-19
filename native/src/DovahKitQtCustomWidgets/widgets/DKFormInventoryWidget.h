#pragma once
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableView>
#include <QWidget>
#include "./DKFormPicker.h"

#if !defined(QT_PLUGIN)
namespace dovah {
   namespace loaded_forms {
      namespace components {
         class container_data;
      }
      class Form;
   }
   class form_stub;
}

class DKFormInventoryModel;
#endif

class DKFormInventoryWidget : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE true);
   public:
      DKFormInventoryWidget(QWidget* parent = nullptr);

      constexpr Qt::Orientation orientation() const noexcept { return this->_state.orientation; }
      void setOrientation(Qt::Orientation);

      #if !defined(QT_PLUGIN)
      void initializeFrom(const dovah::loaded_forms::components::container_data&);
      void commitTo(dovah::loaded_forms::components::container_data&, dovah::loaded_forms::Form& owner);
      #endif

   protected:
      struct {
         QTableView* view = nullptr;

         QWidget* details_pane = nullptr;

         DKFormPicker*   current_item          = nullptr;
         QSpinBox*       current_count         = nullptr;
         QDoubleSpinBox* current_health        = nullptr;
         QGroupBox*      current_owner         = nullptr;
         DKFormPicker*   current_owner_actor   = nullptr;
         DKFormPicker*   current_owner_faction = nullptr;
         DKFormPicker*   current_owner_global  = nullptr;
         QSpinBox*       current_owner_rank    = nullptr;
         //
         QRadioButton* current_owner_type_actor   = nullptr;
         QRadioButton* current_owner_type_faction = nullptr;

         QWidget*     preview_container = nullptr;
         QPushButton* preview_button    = nullptr;
         QSpinBox*    preview_level     = nullptr;
      } _subwidgets;
      struct {
         Qt::Orientation orientation = Qt::Orientation::Vertical;
      } _state;
      #if !defined(QT_PLUGIN)
      DKFormInventoryModel* _model = nullptr;
      #endif

      #if !defined(QT_PLUGIN)
      void _preview();
      void _pullEntryFromModel();
      void _writeEntryToModel();
      #endif
      void _rebuildLayout();
};