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
         struct container_data;
      }
      class Form;
   }
   class form_stub;
}

class DKFormInventoryModel;
#endif

class DKFormInventoryWidget : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool            allowsExtraData    READ allowsExtraData    WRITE setAllowsExtraData    DESIGNABLE true);
   Q_PROPERTY(bool            allowsPseudoItems  READ allowsPseudoItems  WRITE setAllowsPseudoItems  DESIGNABLE true);
   Q_PROPERTY(Qt::Orientation orientation        READ orientation        WRITE setOrientation        DESIGNABLE true USER true);
   Q_PROPERTY(bool            showPreviewWidgets READ showPreviewWidgets WRITE setShowPreviewWidgets DESIGNABLE true);
   public:
      DKFormInventoryWidget(QWidget* parent = nullptr);

      bool allowsExtraData() const noexcept;
      void setAllowsExtraData(bool);

      bool allowsPseudoItems() const noexcept;
      void setAllowsPseudoItems(bool);

      constexpr Qt::Orientation orientation() const noexcept { return this->_state.orientation; }
      void setOrientation(Qt::Orientation);

      constexpr bool showPreviewWidgets() const noexcept { return this->_state.show_preview_widgets; }
      void setShowPreviewWidgets(bool);

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
         struct {
            QLabel* current_health = nullptr;
         } labels;
      } _subwidgets;
      struct {
         #if defined(QT_PLUGIN)
         bool allow_extra_data = true;
         bool allow_pseudo_items = true;
         #endif
         Qt::Orientation orientation = Qt::Orientation::Vertical;
         bool show_preview_widgets = true;
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