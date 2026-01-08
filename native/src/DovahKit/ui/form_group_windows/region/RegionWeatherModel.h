#pragma once
#include <vector>
#include <QAbstractItemModel>
namespace dovah {
   namespace loaded_forms {
      namespace structs::region::generable_content {
         class weather_collection;
      }
      class Region;
   }
   class form_stub;
}

class RegionWeatherModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using backend_collection_type = dovah::loaded_forms::structs::region::generable_content::weather_collection;
      using loaded_form_type        = dovah::loaded_forms::Region;
      struct Column {
         Column() = delete;
         enum {
            WeatherName,
            Chance,
            GlobalName,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      static constexpr const Qt::ItemDataRole WeatherStubRole = Qt::UserRole;
      static constexpr const Qt::ItemDataRole GlobalStubRole  = (Qt::ItemDataRole)(Qt::UserRole + 1);

   public:
      RegionWeatherModel(QObject* parent = nullptr);
      ~RegionWeatherModel();

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex&) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
            #pragma region Editing
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

      QModelIndex addWeather(dovah::form_stub& weather, float chance, dovah::form_stub* global);
      bool containsWeather(const dovah::form_stub&) const;

   protected:
      struct Item {
         dovah::form_stub* weather = nullptr;
         float chance = 0.0F;
         dovah::form_stub* global = nullptr;
         struct {
            QString weather_id;
            QString global_id;
         } cached;
      };
      std::vector<Item> _items;

      void _on_form_deleted(dovah::form_stub&);
      void _on_form_modified(dovah::form_stub&);
};