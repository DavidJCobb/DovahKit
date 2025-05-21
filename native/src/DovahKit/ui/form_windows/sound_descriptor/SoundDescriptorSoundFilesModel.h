#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"
#include "dovah/forms/structs/actor_creature_sounds.h"

struct SoundDescriptorSoundFilesModelNode {
   QString filepath;
};

class SoundDescriptorSoundFilesModel : public DKGenericListModel<SoundDescriptorSoundFilesModel, SoundDescriptorSoundFilesModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Filename,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   public:
      SoundDescriptorSoundFilesModel(QObject* parent);

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

      void importItems(const std::vector<std::string>& src);
      void exportItems(std::vector<std::string>& dst) const;
};

