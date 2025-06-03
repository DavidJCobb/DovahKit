#pragma once
#include <optional>
#include <QAction>
#include <QMenu>
#include <QTableView>
#include <QWidget>
#if !defined(QT_PLUGIN)
   #include "dovah/data/magic_casting_type.h"
   #include "dovah/data/magic_delivery_type.h"
#endif

class  DKMagicEffectListModel;
struct DKMagicEffectListModelItem;
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

         dovah::form_stub* costliestMagicEffect() const;
         QList<dovah::form_stub*> magicEffects() const;

         const DKMagicEffectListModelItem* effect(size_t) const;
         size_t effectCount() const;
      #endif

   signals:
      void contentsChanged();

   protected:
      DKMagicEffectListModel* _model = nullptr;
      #if !defined(QT_PLUGIN)
         struct {
            QMenu menu;
            struct {
               QAction* add    = nullptr;
               QAction* edit   = nullptr;
               QAction* remove = nullptr;
            } actions;
         } _context_menu;
         struct {
            dovah::loaded_forms::Form* form = nullptr;
            std::optional<dovah::magic_casting_type>  casting_type;
            std::optional<dovah::magic_delivery_type> delivery_type;
            struct {
               uint32_t auto_calculated_cost = 0;
            } cached;
         } _state;
      #endif
      struct {
         QTableView* view = nullptr;
      } _subwidgets;

      #if !defined(QT_PLUGIN)
         void _new_effect();
         void _edit_effect();
         void _delete_effects();

         void _update_cached_data();
         void _update_can_add_effect();
      #endif
};