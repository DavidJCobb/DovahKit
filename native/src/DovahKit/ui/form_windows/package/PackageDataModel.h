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
         std::optional<ui::types::packages::package_data_value> value_default;
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

      static constexpr const Qt::ItemDataRole UniqueIDRole     = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const Qt::ItemDataRole TypeRole         = (Qt::ItemDataRole)(Qt::UserRole + 2);
      static constexpr const Qt::ItemDataRole ValueIsLocalRole = (Qt::ItemDataRole)(Qt::UserRole + 3);

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
      void importDefaultValues(const dovah::loaded_forms::structs::custom_packages::package_data_value_map&);
      void importValues(const dovah::loaded_forms::structs::custom_packages::package_data_value_map&);

      // Deletes rows that have neither a value nor a default value.
      // We do this because Skyrim.esm is... weird... about package data. Some common package 
      // templates, like Sandbox, are filled with "DELETEME" package data declarations with no 
      // corresponding value. They're hidden when editing both the template and any packages 
      // that use it.
      void hideValuelessRows();

      constexpr bool declarationsOwned() const noexcept { return this->_data.owns_declarations; }
      void setDeclarationsOwned(bool);

      void exportDeclarations(dovah::loaded_forms::structs::custom_packages::package_data_declaration_map& dst, dovah::loaded_forms::Form& dst_owner);
      void exportValues(dovah::loaded_forms::structs::custom_packages::package_data_value_map& dst, dovah::loaded_forms::Form& dst_owner);

      ui::types::packages::package_data_declaration rowDeclaration(size_t row) const;
      std::optional<ui::types::packages::package_data_value> rowValue(size_t row) const;
      std::optional<ui::types::packages::package_data_value> rowValueOrDefault(size_t row) const;

      void setRowDeclaration(size_t row, const ui::types::packages::package_data_declaration&);
      void setRowValue(size_t row, const std::optional<ui::types::packages::package_data_value>&);
      void setRowValue(size_t row, const ui::types::packages::package_data_value&);
      void resetRowValueToDefault(size_t row);

      QModelIndex appendRow();
      void moveRow(int row, int by);
      void deleteRow(size_t row);

      QModelIndex findUniqueID(uint8_t) const;

   protected:
      struct ItemWithCaching : public Item {
         struct {
            QString value;
            QString value_default;
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
         std::vector<ItemWithCaching> hidden;
         uint8_t next_unique_id = 0;
      } _data;

      QString _value_to_string(std::optional<ui::types::packages::package_data_value>& item);
      void _recache_quest_aliases();
      void _recache_item_values_using_aliases();

      const int _row_for_unique_id(uint8_t) const;
      const ItemWithCaching* _item_by_unique_id(uint8_t) const;
      ItemWithCaching* _item_by_unique_id(uint8_t id) {
         return const_cast<ItemWithCaching*>(std::as_const(*this)._item_by_unique_id(id));
      }

      void _on_form_modified(dovah::form_stub&);
      void _sever_uses_of_form(dovah::form_stub&);

      uint8_t _get_available_unique_id() const;

      QString _location_alias_name(int32_t id) const;
      QString _reference_alias_name(int32_t id) const;
};