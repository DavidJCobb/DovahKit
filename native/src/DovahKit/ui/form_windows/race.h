#pragma once
#include <array>
#include <type_traits>
#include "./_base.h"
#include "dovah/forms/Race.h"
#include "dovah/utils/data_by_sex.h"
#include "ui_race.h" // generated

class FaceBaseHeadPartsModel;
class FaceExtraHeadPartsModel;
class FormPickerFromFormListPaneFilter;
class HeadPartPickerFilter;
class RaceAvailableFaceMorphsModel;
class RaceBaseMovementDefaultsModel;
class RaceBipedObjectSlotsModel;
class RaceEquipSlotsModel;
class RaceEquipTypesModel;
class RaceTintDefaultColorPickerFilter;
class RaceTintLayerModel;
class RaceTintLayerPresetsModel;

class FormDialogRace :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Race, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogRace(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      static constexpr const size_t num_skill_boosts = std::tuple_size_v<decltype(std::declval<loaded_form_type>().stats.skill_boosts)>;

      struct tints_tab_subwidgets {
         struct {
            struct {
               QPushButton* create    = nullptr;
               QPushButton* move_up   = nullptr;
               QPushButton* move_down = nullptr;
               QPushButton* remove    = nullptr;
            } buttons;
            QTableView* view = nullptr;
            struct {
               QWidget*          container = nullptr;
               DKGameFilePicker* texture   = nullptr;
               QComboBox*        type      = nullptr;
               DKFormPicker*     default_color = nullptr;
            } edit;
         } layers;
         struct {
            struct {
               QPushButton* create    = nullptr;
               QPushButton* move_up   = nullptr;
               QPushButton* move_down = nullptr;
               QPushButton* remove    = nullptr;
            } buttons;
            QTableView* view = nullptr;
            struct {
               DKFormPicker* color = nullptr;
               struct {
                  DKFloatSlider*  slider  = nullptr;
                  QDoubleSpinBox* spinbox = nullptr;
               } alpha;
            } edit;
         } presets;
      };

   protected:
      Ui::FormDialogRace ui;
      struct {
         dovah::data_by_sex<HeadPartPickerFilter*>             base_head_part;
         dovah::data_by_sex<FormPickerFromFormListPaneFilter*> default_complexion;
         dovah::data_by_sex<FormPickerFromFormListPaneFilter*> default_hair_color;
         dovah::data_by_sex<RaceTintDefaultColorPickerFilter*> tint_layer_default_color;
      } _filters;
      struct {
         RaceBipedObjectSlotsModel* biped_objects = nullptr;
         RaceEquipSlotsModel*       equip_slots   = nullptr;
         RaceEquipTypesModel*       equip_types   = nullptr;

         RaceBaseMovementDefaultsModel* base_movement_types = nullptr;

         dovah::data_by_sex<FaceBaseHeadPartsModel*>  head_parts_base;
         dovah::data_by_sex<FaceExtraHeadPartsModel*> head_parts_extra;

         dovah::data_by_sex<RaceAvailableFaceMorphsModel*> available_face_morphs;

         dovah::data_by_sex<RaceTintLayerModel*>        tint_layer_model;
         dovah::data_by_sex<RaceTintLayerPresetsModel*> tint_preset_model; // NOTE: the preset model is a child/proxy of the layer model
      } _models;
      struct {
         union {
            std::array<QRadioButton*, 3> all = {};
            struct {
               QRadioButton* overlay;
               QRadioButton* use_own;
               QRadioButton* inherit;
            };
         } head_part_inheritance;
         struct {
            dovah::data_by_sex<QTableView*> extra;
         } head_parts;
         union _ {
            ~_() { all.~array(); }

            std::array<QTabWidget*, 3> all = {};
            struct {
               QTabWidget* body;
               QTabWidget* face_data;
               QTabWidget* face_tints;
            };
         } sex_tabboxes;
         struct {
            std::array<QComboBox*, num_skill_boosts> which = {};
            std::array<QSpinBox*,  num_skill_boosts> boost = {};
         } skills;
         dovah::data_by_sex<tints_tab_subwidgets> tints;
      } _subwidgets;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_slot_dropdowns();

      void _add_new_tint_layer(dovah::sex);
      void _add_new_tint_preset(dovah::sex);
      void _on_tint_layer_selection_changed(dovah::sex, const QItemSelection&);
      void _on_tint_preset_selection_changed(dovah::sex, const QItemSelection&);
      QModelIndex _selected_tint_layer(dovah::sex) const;
      QModelIndex _selected_tint_preset(dovah::sex) const;
      //
      void _push_tint_layer_to_model(dovah::sex);
      void _push_tint_preset_to_model(dovah::sex);

      virtual bool eventFilter(QObject* object, QEvent* event) override;
};