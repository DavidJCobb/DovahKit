#pragma once
#include <optional>
#include <QActions>
#include <QTableView>
#include <QWidget>
#if !defined(QT_PLUGIN)
   #include "dovah/data/magic_casting_type.h"
   #include "dovah/data/magic_delivery_type.h"
#endif

class DKMagicEffectListModel;
#if !defined(QT_PLUGIN)
   namespace dovah {
      namespace loaded_forms {
         namespace components {
            class magic_effect_list;
         }
         class Form;
      }
      class form_stub;
   }
#endif

class DKMagicEffectListWidget : public QWidget {
   Q_OBJECT;
   public:
      DKMagicEffectListWidget(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         constexpr const std::optional<dovah::magic_casting_type>& castingType() const noexcept { return this->_state.casting_type; }
         void setCastingType(const std::optional<dovah::magic_casting_type>&);

         constexpr const std::optional<dovah::magic_delivery_type>& deliveryType() const noexcept { return this->_state.delivery_type; }
         void setDeliveryType(const std::optional<dovah::magic_delivery_type>&);

         void importFrom(dovah::loaded_forms::Form& owner, const dovah::loaded_forms::components::magic_effect_list& target);
         void exportTo(dovah::loaded_forms::Form& owner, dovah::loaded_forms::components::magic_effect_list& target);

         void openCreateEffectModal();
         void openEditEffectModal();
      #endif

   protected:
      struct {
         QAction* create = nullptr;
         QAction* edit   = nullptr;
         QAction* remove = nullptr;
      } _actions;
      DKMagicEffectListModel* _model = nullptr;
      #if !defined(QT_PLUGIN)
      struct {
         std::optional<dovah::magic_casting_type>  casting_type;
         std::optional<dovah::magic_delivery_type> delivery_type;
      } _state;
      #endif
      struct {
         QTableView* view = nullptr;
      } _subwidgets;
};