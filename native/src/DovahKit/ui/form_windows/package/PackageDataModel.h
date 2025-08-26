#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include "ui/types/packages/package_data_declaration.h"
#include "ui/types/packages/package_data_value.h"
namespace dovah::loaded_forms {
   namespace structs::custom_packages {
      class package_data_declaration_map;
      class package_data_value_map;
   }
   class Form;
}

class PackageDataModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Item {
         ui::types::packages::package_data_declaration          declaration;
         std::optional<ui::types::packages::package_data_value> value;
      };

   public:
      PackageDataModel(QObject* parent = nullptr);

      struct Column {
         Column() = delete;
         enum {
            Name,
            Type,
            Value,
            IsPublic,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      static constexpr const Qt::ItemDataRole UniqueIDRole = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const Qt::ItemDataRole TypeRole     = (Qt::ItemDataRole)(Qt::UserRole + 2);

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

      void clear();

      void setOwningQuest(dovah::form_stub*);

      void importDeclarations(const dovah::loaded_forms::structs::custom_packages::package_data_declaration_map&, bool owned);
      void importValues(const dovah::loaded_forms::structs::custom_packages::package_data_value_map&);

      constexpr bool declarationsOwned() const noexcept { return this->_data.owns_declarations; }
      void setDeclarationsOwned(bool);

      void exportDeclarations(dovah::loaded_forms::structs::custom_packages::package_data_declaration_map& dst, dovah::loaded_forms::Form& dst_owner);
      void exportValues(dovah::loaded_forms::structs::custom_packages::package_data_value_map& dst, dovah::loaded_forms::Form& dst_owner);

      ui::types::packages::package_data_declaration rowDeclaration(size_t row) const;
      std::optional<ui::types::packages::package_data_value> rowValue(size_t row) const;

      void setRowDeclaration(size_t row, const ui::types::packages::package_data_declaration&);
      void setRowValue(size_t row, const std::optional<ui::types::packages::package_data_value>&);
      void setRowValue(size_t row, const ui::types::packages::package_data_value&);

      QModelIndex appendRow();
      void moveRow(int row, int by);
      void deleteRow(size_t row);

      QModelIndex findUniqueID(uint8_t) const;

   protected:
      struct ItemWithCaching : public Item {
         struct {
            QString value;
         } cached;
      };

      struct {
         struct {
            QMap<int32_t, QString> location;
            QMap<int32_t, QString> reference;
         } alias_names;
      } _cached;
      struct {
         bool owns_declarations = false;
         dovah::form_stub* owning_quest = nullptr;
         std::vector<ItemWithCaching> items;
         uint8_t next_unique_id = 0;
      } _data;

      void _recache_item_value_string(ItemWithCaching& item);
      void _recache_quest_aliases();
      void _recache_item_values_using_aliases();

      const Item* _item_by_unique_id(uint8_t) const;
      Item* _item_by_unique_id(uint8_t id) {
         return const_cast<Item*>(std::as_const(*this)._item_by_unique_id(id));
      }

      void _on_form_modified(dovah::form_stub&);
      void _sever_uses_of_form(dovah::form_stub&);

      uint8_t _get_available_unique_id() const;

      QString _location_alias_name(int32_t id) const;
      QString _reference_alias_name(int32_t id) const;
};