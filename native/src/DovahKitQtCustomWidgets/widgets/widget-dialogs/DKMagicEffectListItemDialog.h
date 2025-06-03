#pragma once
#if defined(QT_PLUGIN)
   #error This dialog relies on DovahKit to run (dependency in DKBSACollectionModelBackend). Do not include it when compiling the Qt Designer plug-in.
#endif
#include "ui_DKMagicEffectListItemDialog.h" // generated
#include <cstdint>
#include <QDialog>
#include "dovah/data/magic_casting_type.h"
#include "dovah/data/magic_delivery_type.h"
#include "dovah/utils/magic_effect_list_item_cost_calculator.h"
#include "../widget-data/DKCustomFormFilter.h"
#include "../widget-models/DKMagicEffectListModel.h"

namespace dovah::loaded_forms {
   class Form;
}
class DKMagicEffectListWidget;

class DKMagicEffectListItemDialog : public QDialog {
   Q_OBJECT;
   protected:
      class EffectFilter : public DKCustomFormFilter {
         public:
            virtual bool form_matches(dovah::form_stub& stub) const noexcept override;

         public:
            void configure(std::optional<dovah::magic_casting_type>, std::optional<dovah::magic_delivery_type>);

         protected:
            std::optional<dovah::magic_casting_type>  _casting_type;
            std::optional<dovah::magic_delivery_type> _delivery_type;
      };

   public:
      struct Context {
         dovah::loaded_forms::Form& form; // form to which the effect list belongs
         std::optional<dovah::magic_casting_type>  casting_type;
         std::optional<dovah::magic_delivery_type> delivery_type;
         uint32_t spell_total_cost = 0;
         bool     not_yet_added    = false;
      };

   public:
      DKMagicEffectListItemDialog(Context&, QWidget* parent = nullptr);

      [[nodiscard]] DKMagicEffectListModel::Item data() const;
      void setData(const DKMagicEffectListModel::Item&);

   protected:
      Ui::DKMagicEffectListItemDialog ui;
      Context _context;
      float   _initial_cost = 0;
      dovah::magic_effect_list_item_cost_calculator _cost_calculator;
      struct {
         float taper_duration = 0.0F;
      } _cached_effect_info;
      EffectFilter _effect_filter;
      bool _size_corrected_on_show = false;

      void _pull_magic_effect_form_data(dovah::form_stub*);
      void _update_displayed_costs();
      void _update_displayed_duration();

      virtual void showEvent(QShowEvent*) override;
};