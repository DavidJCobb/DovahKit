#pragma once
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include <QPointer>
#include "helpers/enum_flags.h"
#include "dovah/data/furniture/animation_type.h"
#include "dovah/data/furniture/entry_point.h"
#include "editor/form_stub_meta_type.h"
namespace dovah {
   namespace loaded_forms {
      class Furniture;
   }
   class form_stub;
}

class FurnitureMarkersModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      static constexpr const Qt::ItemDataRole EntryPointsEnabledRole   = (Qt::ItemDataRole)(Qt::UserRole);
      static constexpr const Qt::ItemDataRole EntryPointsSupportedRole = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const Qt::ItemDataRole KeywordRole              = (Qt::ItemDataRole)(Qt::UserRole + 2);
      static constexpr const Qt::ItemDataRole AnimationTypeRole        = (Qt::ItemDataRole)(Qt::UserRole + 3);

      using AnimationType   = dovah::furniture::animation_type;
      using EntryPoint      = dovah::furniture::entry_point;
      using EntryPointFlags = dovah::furniture::entry_point_flags;

   protected:
      struct Node {
         AnimationType     animation_type = AnimationType::sit;
         struct {
            EntryPointFlags enabled;
            EntryPointFlags supported;
         } entry_points;
         dovah::form_stub* keyword = nullptr;
         struct {
            QString keyword_editor_id;
         } cached;
      };

   public:
      FurnitureMarkersModel(QObject* parent = nullptr);
      
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            #pragma region Write-access
               virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
            #pragma endregion
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void importData(const dovah::loaded_forms::Furniture&);
      void exportData(dovah::loaded_forms::Furniture&) const;

   protected:
      std::vector<Node> _nodes;

      void _pull_marker_info_from_nif(const std::string& nif_path);
};

class FurnitureMarkerEntryPointsProxyModel : public QAbstractItemModel {
   public:
      using source_model_type = FurnitureMarkersModel;

      using EntryPoint      = source_model_type::EntryPoint;
      using EntryPointFlags = source_model_type::EntryPointFlags;

      static constexpr const Qt::ItemDataRole KeywordRole = source_model_type::KeywordRole;
      
      static constexpr const size_t valid_entry_point_count = 5;

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            #pragma region Write-access
               virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
            #pragma endregion
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      QModelIndex source() const;
      void setSource(const QModelIndex&);

   protected:
      QPersistentModelIndex _source_qmi;
      struct {
         struct {
            EntryPointFlags supported;
            EntryPointFlags enabled;
            uint8_t supported_count = 0;
         } entry_points;
      } _cache;

      std::optional<EntryPoint> _map_row_to_entry_point(int row) const;
      void _recache();
};