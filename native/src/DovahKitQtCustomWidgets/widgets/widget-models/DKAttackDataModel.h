#pragma once
#include "ui/models/DKGenericListModel.h"

namespace dovah {
   namespace loaded_forms {
      namespace components {
         class attack_data;
      }
      class Form;
   }
   class form_stub;
}

struct DKAttackDataModelNode {
   QString event_name;
   float damage_mult = 1;
   float attack_chance = 1;
   float stagger = 1;
   float recovery_time = 1;
   float stamina_cost_mult = 1;
   dovah::form_stub* spell   = nullptr;
   dovah::form_stub* keyword = nullptr;
   struct {
      float direction = 0;
      float range     = 0;
   } angles;
   float knockdown = 0;
   struct {
      bool bash          = false;
      bool ignore_weapon = false;
      bool lefthanded    = false;
      bool power         = false;
      bool rotating      = false;
      bool overridden    = true;
   } flags;

   struct {
      QString spellEditorID;
      QString keywordEditorID;
   } cached;

   void recache_editor_ids();
};

class DKAttackDataModel : public DKGenericListModel<DKAttackDataModel, DKAttackDataModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Name,
            DamageMult,
            AttackChance,
            Stagger,
            RecoveryTime,
            StaminaCostMult,
            Spell,
            Keyword,
            Angle,
            Knockdown,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      DKAttackDataModel(QObject* parent);

      using DKGenericListModel::clear;
      using DKGenericListModel::deleteItems;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      QModelIndex create();
      QModelIndex overwrite(int row, const node_type& src);
      const node_type* item(int row) const;

      void overwriteAllItems(const std::vector<node_type>& src);
};
