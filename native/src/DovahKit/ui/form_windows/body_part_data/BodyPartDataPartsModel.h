#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QPointer>
#include "dovah/data/limbs.h"
#include "dovah/forms/BodyPartData.h"
class SkeletonBonesModel;

// As a shortcut, this will be implemented as just a thin wrapper around the 
// "working copy" of the BodyPartData form. Post-launch rewrite plans will 
// make it easier to replace this with a more robust approach (i.e. one where 
// the model actually stores its own data rather than just being an interface).
//
// The model listens for form-modified signals and updates the parts as needed, 
// but any UI-side changes should be made directly, followed by a call to the 
// `refreshPart` function.
class BodyPartDataPartsModel final : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::BodyPartData;
      using loaded_item_type = loaded_form_type::unmanaged_part;

      struct Column {
         Column() = delete;
         enum {
            Name,
            Limb,
            MainNode,
            TargetNode,
            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

   protected:
      void _reset_invalid_bone_names(bool silent);

   public:
      BodyPartDataPartsModel(QObject* parent = nullptr);

      void importData(loaded_form_type&);
      void exportData(loaded_form_type&) const;

      SkeletonBonesModel* bonesModel() const;
      void setBonesModel(SkeletonBonesModel*);

      void resetInvalidBoneNames();

      const loaded_item_type* bodyPart(size_t row) const;
      void replaceBodyPart(size_t row, const loaded_item_type&);
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
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
         #pragma region Editing
            virtual bool insertRows(int row, int count, const QModelIndex& parent = {}) override;
            virtual bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
         #pragma endregion
      #pragma endregion

   protected:
      struct {
         QPointer<SkeletonBonesModel>  bones_model;
         std::vector<loaded_item_type> parts;
      } _data;
};