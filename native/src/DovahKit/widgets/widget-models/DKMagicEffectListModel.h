#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include "ui/types/conditions/condition.h"

namespace dovah {
   namespace loaded_forms {
      namespace components {
         struct magic_effect_list;
      }
      class Form;
   }
   class form_stub;
}

class DKMagicEffectListModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Item {
         dovah::form_stub* magic_effect = nullptr;
         float             magnitude    = 0.0F;
         uint32_t          area         = 0;
         uint32_t          duration     = 0;
         std::vector<ui::types::conditions::condition> conditions;

         // Cached data, to avoid doing UI-side string conversions every repaint:
         struct {
            QString  effect_name;
            QString  magic_school;
            uint32_t base_cost = 0;
         } cached;

         void _update_cache() {
            if (!this->magic_effect) {
               this->cached = {};
               return;
            }
            auto loaded = this->magic_effect->load().ptr_cast<dovah::loaded_forms::MagicEffect>();
            if (!loaded) {
               this->cached = {};
               return;
            }

            auto& editor = DovahKitCore::get();
            this->cached.base_cost   = loaded->base_cost;
            this->cached.effect_name = editor.convert_localized_string(loaded->name);
            if (this->cached.effect_name.isEmpty()) {
               this->cached.effect_name = QString::fromStdString(this->magic_effect->editorID);
            }

            {  // Magic School name
               auto i = loaded->magic_skill;
               if (i < 0) {
                  this->cached.magic_school = tr("NONE", "actor vale name");
               } else if (i >= dovah::all_actor_value_info.size()) {
                  this->cached.magic_school = tr("INVALID", "actor vale name");
               } else {
                  auto& av_info = dovah::all_actor_value_info[i];
                  auto* av_stub = editor.get_form_of_probable_type(dovah::form_type::actor_value_info, av_info.formID);
                  QString av_name;
                  if (av_stub) {
                     /*// TODO: When we can load AVIFs, use their TESFullNames.
                     auto loaded = av_stub->load().ptr_cast<dovah::loaded_forms::ActorValue>();
                     if (loaded) {
                     av_name = editor.convert_localized_string(loaded->name);
                     }
                     //*/
                  }
                  if (av_name.isEmpty()) {
                     this->cached.magic_school = QString(QLatin1String(av_info.name.data(), av_info.name.size()));
                  } else {
                     this->cached.magic_school = av_name;
                  }
               }
            }
         }
      };

      using loaded_form  = dovah::loaded_forms::Form;
      using backend_type = dovah::loaded_forms::components::magic_effect_list;

      struct Column {
         Column() = delete;
         enum {
            Index,
            Name,
            Magnitude,
            Area,
            Duration,
            Cost,
            MagicSchool,

            _COUNT
         };
      };

   public:
      DKMagicEffectListModel(QObject* parent = nullptr);
      ~DKMagicEffectListModel();
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override final;
            virtual int         columnCount(const QModelIndex& item = {}) const override final;

            virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
         #pragma region Editing
            virtual bool insertRows(int row, int count, const QModelIndex& parent = {}) override;
            virtual bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
         #pragma endregion
      #pragma endregion

      void importFrom(loaded_form& component_containing_form, const backend_type& component);
      void commitTo(loaded_form& component_containing_form, backend_type& component) const;

      constexpr const Item* data(size_t row) const noexcept {
         if (row >= this->_items.size())
            return nullptr;
         return this->_items[row];
      }

      void setData(size_t row, const Item&);

   protected:
      ui::types::conditions::context _condition_context;
      std::vector<Item*> _items;

      void _clear();
};