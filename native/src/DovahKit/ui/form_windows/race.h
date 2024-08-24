#pragma once
#include <array>
#include <type_traits>
#include "./_base.h"
#include "dovah/forms/Race.h"
#include "dovah/utils/data_by_sex.h"
#include "ui_race.h" // generated

class FaceBaseHeadPartsModel;
class FaceExtraHeadPartsModel;
class HeadPartPickerFilter;

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

   protected:
      Ui::FormDialogRace ui;
      struct {
         dovah::data_by_sex< HeadPartPickerFilter*> base_head_part;
      } _filters;
      struct {
         dovah::data_by_sex<FaceBaseHeadPartsModel*>  head_parts_base;
         dovah::data_by_sex<FaceExtraHeadPartsModel*> head_parts_extra;
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
         struct {
            std::array<QComboBox*, num_skill_boosts> which = {};
            std::array<QSpinBox*,  num_skill_boosts> boost = {};
         } skills;
      } _subwidgets;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      virtual bool eventFilter(QObject* object, QEvent* event) override;
};