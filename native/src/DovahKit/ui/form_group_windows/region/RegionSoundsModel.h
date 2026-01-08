#pragma once
#include <vector>
#include <QAbstractItemModel>
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class audio;
      }
      class Region;
   }
   class form_stub;
}

class RegionSoundsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::audio;
      using loaded_form_type        = dovah::loaded_forms::Region;
      struct Column {
         Column() = delete;
         enum {
            SoundName,
            Chance,
            WeatherIsPleasant,
            WeahterIsCloudy,
            WeatherIsRainy,
            WeatherIsSnowy,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      static constexpr const Qt::ItemDataRole FormStubRole = Qt::UserRole;

   public:
      RegionSoundsModel(QObject* parent = nullptr);
      ~RegionSoundsModel();

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex&) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
            #pragma region Editing
               virtual bool insertRows(int row, int count, const QModelIndex& parent = {}) override;
               virtual bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
            #pragma endregion
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex&, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex&) const override;
            virtual bool          setData(const QModelIndex&, const QVariant& value, int role) override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
         #pragma region Drag and drop
            #pragma region Whole-model queries
               virtual QStringList mimeTypes() const override;
               virtual Qt::DropActions supportedDropActions() const override;
            #pragma endregion
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
         #pragma endregion
      #pragma endregion

      void importData(const loaded_form_type&);
      void exportData(loaded_form_type&);
      void clear();

   protected:
      struct Item {
         dovah::form_stub* form = nullptr;
         float chance = 0.0F;
         struct {
            bool pleasant = false;
            bool cloudy   = false;
            bool rainy    = false;
            bool snowy    = false;
         } weather;
         struct {
            QString editor_id;
         } cached;
      };
      std::vector<Item> _items;

      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);
};