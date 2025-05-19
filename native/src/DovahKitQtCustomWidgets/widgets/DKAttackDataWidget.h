#pragma once
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QWidget>
#include "./DKFormPicker.h"

#if !defined(QT_PLUGIN)
namespace dovah {
   namespace loaded_forms {
      namespace components {
         class attack_data;
      }
      class Form;
   }
}
class DKAttackDataModel;
#endif

class DKAttackDataWidget : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool showsAttackRace READ showsAttackRace WRITE setShowsAttackRace DESIGNABLE true);
   public:
      DKAttackDataWidget(QWidget* parent = nullptr);

      constexpr bool showsAttackRace() const noexcept { return this->_state.show_attack_race; }
      void setShowsAttackRace(bool);

      #if !defined(QT_PLUGIN)
         void initializeFrom(const dovah::loaded_forms::components::attack_data&);
         void commitTo(dovah::loaded_forms::components::attack_data&, dovah::loaded_forms::Form& component_containing_form);
      #endif

   protected:
      #if !defined(QT_PLUGIN)
         DKAttackDataModel* _model = nullptr;
      #endif
      struct {
         struct {
            QWidget* container = nullptr;
            //
            DKFormPicker* picker = nullptr;
         } race;
         struct {
            QWidget* container = nullptr;
            //
            QTableView*  view          = nullptr;
            QPushButton* button_create = nullptr;
            QPushButton* button_delete = nullptr;
         } event_listing;
         struct {
            QWidget* container = nullptr;
            //
            QLineEdit*      name              = nullptr;
            QDoubleSpinBox* damage_mult       = nullptr;
            QDoubleSpinBox* attack_chance     = nullptr;
            QDoubleSpinBox* stagger           = nullptr;
            QDoubleSpinBox* recovery_time     = nullptr;
            QDoubleSpinBox* stamina_cost_mult = nullptr;
            DKFormPicker*   attack_spell      = nullptr; // SPEL
            DKFormPicker*   attack_type       = nullptr; // KYWD

            QDoubleSpinBox* attack_angle      = nullptr;
            QDoubleSpinBox* angle_range       = nullptr;
            QDoubleSpinBox* knockdown         = nullptr;

            struct {
               QCheckBox* power         = nullptr;
               QCheckBox* ignore_weapon = nullptr;
               QCheckBox* bash          = nullptr;
               QCheckBox* lefthanded    = nullptr;
               QCheckBox* rotating      = nullptr;
            } flags;
         } event_edit;
      } _subwidgets;
      struct {
         bool show_attack_race = true;
      } _state;

      #if !defined(QT_PLUGIN)
         void _pull_node_to_ui();
         void _push_node_from_ui();
      #endif
};