#pragma once
#include <QAbstractItemModel>
namespace dovah::loaded_forms {
   class Alias;
   class Quest;
}

class QuestAliasesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Column {
         Column() = delete;
         enum {
            Name,
            ID,
            Optional,
            Type,
            Fill,
            Flags,
            Allow,
            Papyrus,
            Packages,
            Inventory,
            Factions,
            Spells,
            Keywords,
            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

   protected:
      struct cached_alias_data {
         QString factions;
         QString fill;
         QString inventory;
         QString keywords;
         QString packages;
         QString papyrus;
         QString spells;

         void recache(const dovah::loaded_forms::Alias&);
      };

   public:
      QuestAliasesModel(dovah::loaded_forms::Quest&, QObject* parent = nullptr);

      //
      // we can't cram all of an alias's data into the model (post-sustain that'll be 
      // achievable but it isn't now), so just keep the data lightweight, remember the 
      // working-copy Quest we pulled it from, and make changes directly to the Quest 
      // in tandem with what we're doing here
      // 
      // or, better yet, don't even store nodes directly and just make the model serve 
      // as a paper-thin wrapper around the working-copy Quest
      //

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      const dovah::loaded_forms::Alias* alias(const QModelIndex&) const;
      dovah::loaded_forms::Alias* alias(const QModelIndex&);
      const dovah::loaded_forms::Alias* aliasByID(int32_t) const;
      dovah::loaded_forms::Alias* aliasByID(int32_t);

      QModelIndex createRefAlias();
      QModelIndex createLocAlias();

      void deleteAlias(const QModelIndex&);

      void onAliasChanged(const QModelIndex&);

   protected:
      dovah::loaded_forms::Quest& _quest;
      std::unordered_map<uint32_t, cached_alias_data> _cache;

      std::optional<uint32_t> _next_alias_id();

      void _recacheAllAliases();
};