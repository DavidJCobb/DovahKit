#include "./PackageDataModel.h"
#include "helpers/bitset.h"
#include "helpers/vectors/move_item_within.h"
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/forms/structs/custom_packages/package_data_declaration_map.h"
#include "dovah/forms/structs/custom_packages/package_data_value_map.h"
#include "dovah/forms/Quest.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/localize/dialogue_topic_subtype.h"
#include "editor/localize/package_data_type.h"
#include "editor/localize/package_object_type.h"

PackageDataModel::PackageDataModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->clear();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      assert(stub != nullptr);
      this->_sever_uses_of_form(*stub);
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      assert(stub != nullptr);
      this->_on_form_modified(*stub);
   });
}
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex PackageDataModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || row >= this->_data.items.size())
            return {};
         if (col < 0 || col >= this->columnCount())
            return {};
         if (parent.isValid())
            return {};
         return this->createIndex(row, col, nullptr);
      }
      /*virtual*/ QModelIndex PackageDataModel::parent(const QModelIndex& index) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex PackageDataModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int PackageDataModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_data.items.size();
      }
      /*virtual*/ int PackageDataModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant PackageDataModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.row() >= this->_data.items.size())
            return {};
         auto& src = this->_data.items[index.row()];

         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               switch (index.column()) {
                  case Column::Name:
                     return src.declaration.name;
                  case Column::Type:
                     if (src.value.has_value()) {
                        return editor::localize::package_data_type(src.value.value().type());
                     }
                     break;
                  case Column::Value:
                     return src.cached.value;
                  case Column::IsPublic:
                     return src.declaration.is_public ? tr("Yes", "boolean") : tr("No", "boolean");
               }
               break;
            case Qt::EditRole:
               switch (index.column()) {
                  case Column::Name:
                     return src.declaration.name;
                  case Column::Type:
                     return (int)src.value.value().type();
                  case Column::Value:
                     return {};
                  case Column::IsPublic:
                     return src.declaration.is_public;
               }
               break;
            case UniqueIDRole:
               return src.declaration.unique_id;
            case TypeRole:
               if (src.value.has_value()) {
                  return (int)src.value.value().type();
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags PackageDataModel::flags(const QModelIndex& index) const /*override*/ {
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
      /*virtual*/ QVariant PackageDataModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case Column::Name:
               return tr("Name");
            case Column::Type:
               return tr("Type");
            case Column::Value:
               return tr("Value");
            case Column::IsPublic:
               return tr("Public");
         }
         return {};
      }
#pragma endregion

void PackageDataModel::clear() {
   this->beginResetModel();
   this->_data.items.clear();
   this->_data.owning_quest = nullptr;
   this->_data.owns_declarations = false;
   this->endResetModel();
}

void PackageDataModel::setOwningQuest(dovah::form_stub* stub) {
   if (stub && stub->form_type != dovah::form_type::quest)
      return;
   if (stub == this->_data.owning_quest)
      return;
   this->_data.owning_quest = stub;
   this->_recache_quest_aliases();
   this->_recache_item_values_using_aliases();
}

void PackageDataModel::importDeclarations(const dovah::loaded_forms::structs::custom_packages::package_data_declaration_map& src, bool owned) {
   using frontend_decl_type = ui::types::packages::package_data_declaration;
   using backend_decl_type  = dovah::loaded_forms::structs::custom_packages::package_data_declaration_map::entry;

   this->_data.owns_declarations = owned;

   for (auto& src_decl : src.entries) {
      if (src_decl.unique_id == frontend_decl_type::no_unique_id)
         continue;
      auto prior_row = this->_row_for_unique_id(src_decl.unique_id);
      if (prior_row >= 0) {
         auto& prior = this->_data.items[prior_row];
         prior.declaration.name      = QString::fromStdString(src_decl.name);
         prior.declaration.is_public = src_decl.is_public;
         //
         auto tl = this->index(prior_row, 0, {});
         auto br = this->index(prior_row, Column::__COUNT - 1, {});
         emit dataChanged(tl, br);
         //
         continue;
      }
      this->beginInsertRows({}, this->_data.items.size(), this->_data.items.size());
      auto& dst_decl = this->_data.items.emplace_back().declaration;
      dst_decl.name      = QString::fromStdString(src_decl.name);
      dst_decl.is_public = src_decl.is_public;
      dst_decl.unique_id = src_decl.unique_id;
      this->endInsertRows();
   }
}
void PackageDataModel::importValues(const dovah::loaded_forms::structs::custom_packages::package_data_value_map& src) {
   using frontend_decl_type = ui::types::packages::package_data_declaration;
   using backend_decl_type  = dovah::loaded_forms::structs::custom_packages::package_data_value_map::entry;

   for (auto& src_pair : src.entries) {
      if (src_pair.unique_id == frontend_decl_type::no_unique_id)
         continue;
      if (!src_pair.value)
         continue;
      auto prior_row = this->_row_for_unique_id(src_pair.unique_id);
      if (prior_row >= 0) {
         auto& prior = this->_data.items[prior_row];
         prior.value.emplace().importData(*src_pair.value);
         _recache_item_value_string(prior);
         //
         auto qmi = this->index(prior_row, Column::Value, {});
         emit dataChanged(qmi, qmi);
         //
         continue;
      }
      const auto row = this->_data.items.size();
      this->beginInsertRows({}, row, row);
      //
      auto& dst_item = this->_data.items.emplace_back();
      dst_item.declaration.unique_id = src_pair.unique_id;
      dst_item.declaration.is_public = false;
      //
      dst_item.value.emplace().importData(*src_pair.value);
      _recache_item_value_string(dst_item);
      //
      this->endInsertRows();
   }

   this->_data.next_unique_id = src.next_unique_id;
}

void PackageDataModel::setDeclarationsOwned(bool v) {
   this->_data.owns_declarations = v;
}

void PackageDataModel::exportDeclarations(dovah::loaded_forms::structs::custom_packages::package_data_declaration_map& dst, dovah::loaded_forms::Form& dst_owner) {
   if (!this->_data.owns_declarations) {
      qWarning("Exporting declarations that we don't own!");
   }
   
   dst.entries.clear();
   for (auto& src_item : this->_data.items) {
      auto& dst_item = dst.entries.emplace_back();
      dst_item.unique_id = src_item.declaration.unique_id;
      dst_item.name      = src_item.declaration.name.toStdString();
      dst_item.is_public = src_item.declaration.is_public;
   }
}
void PackageDataModel::exportValues(dovah::loaded_forms::structs::custom_packages::package_data_value_map& dst, dovah::loaded_forms::Form& dst_owner) {
   dst.clear(dst_owner);
   for (auto& src_item : this->_data.items) {
      if (!src_item.value.has_value())
         continue;
      auto& dst_pair = dst.entries.emplace_back();
      dst_pair.unique_id = src_item.declaration.unique_id;
      dst_pair.value     = std::move(src_item.value.value().exportData(dst_owner));
   }
}

ui::types::packages::package_data_declaration PackageDataModel::rowDeclaration(size_t row) const {
   if (row >= this->_data.items.size())
      throw std::out_of_range("PackageDataModel::rowDeclaration argument out of range");
   return this->_data.items[row].declaration;
}
std::optional<ui::types::packages::package_data_value> PackageDataModel::rowValue(size_t row) const {
   if (row >= this->_data.items.size())
      throw std::out_of_range("PackageDataModel::rowValue argument out of range");
   return this->_data.items[row].value;
}

void PackageDataModel::setRowDeclaration(size_t row, const ui::types::packages::package_data_declaration& src) {
   if (!this->_data.owns_declarations) {
      qWarning("Editing declarations that we don't own!");
   }

   if (row >= this->_data.items.size())
      throw std::out_of_range("PackageDataModel::setRowDeclaration argument out of range");
   auto& item = this->_data.items[row];
   item.declaration = src;

   auto tl = this->index(row, Column::Name,     {});
   auto br = this->index(row, Column::IsPublic, {});
   emit dataChanged(tl, br);
}
void PackageDataModel::setRowValue(size_t row, const std::optional<ui::types::packages::package_data_value>& src) {
   if (row >= this->_data.items.size())
      throw std::out_of_range("PackageDataModel::setRowValue argument out of range");
   if (src.has_value()) {
      this->setRowValue(row, src.value());
      return;
   }
   auto& item = this->_data.items[row];
   item.value.reset();
   item.cached.value.clear();
   auto qmi = this->index(row, Column::Value, {});
   emit dataChanged(qmi, qmi);
}
void PackageDataModel::setRowValue(size_t row, const ui::types::packages::package_data_value& src) {
   if (row >= this->_data.items.size())
      throw std::out_of_range("PackageDataModel::setRowValue argument out of range");
   auto& item = this->_data.items[row];
   item.value = src;

   this->_recache_item_value_string(item);
   auto qmi = this->index(row, Column::Value, {});
   emit dataChanged(qmi, qmi);
}

QModelIndex PackageDataModel::appendRow() {
   auto id = this->_get_available_unique_id();
   if (id == ui::types::packages::package_data_declaration::no_unique_id) {
      return {};
   }
   if (this->_data.next_unique_id == id) {
      ++this->_data.next_unique_id;
   }
   size_t row = this->_data.items.size();
   this->beginInsertRows({}, row, row);
   auto& item = this->_data.items.emplace_back();
   item.declaration.unique_id = id;
   {  // Default value: bool, false
      auto& val = item.value.emplace();
      val.emplace<dovah::packages::package_data_type::boolean>() = false;
   }
   this->_recache_item_value_string(item);
   this->endInsertRows();
   return this->index(row, 0, {});
}
void PackageDataModel::moveRow(int from, int by) {
   if (from < 0 || from >= this->_data.items.size())
      return;
   int to = from + by;
   if (to < 0)
      to = 0;
   else if (to >= this->_data.items.size())
      to = this->_data.items.size() - 1;
   this->beginMoveRows(
      {},
      from,
      from,
      {},
      (to < from) ? to : to + 1 // Qt API design jank
   );
   cobb::vectors::move_item_within(this->_data.items, from, by);
   this->endMoveRows();
}
void PackageDataModel::deleteRow(size_t row) {
   if (!this->_data.owns_declarations) {
      qWarning("Editing declarations that we don't own!");
   }
   if (row >= this->_data.items.size())
      throw std::out_of_range("PackageDataModel::setRowDeclaration argument out of range");
   this->beginRemoveRows({}, row, row);
   this->_data.items.erase(this->_data.items.begin() + row);
   this->endRemoveRows();
}

QModelIndex PackageDataModel::findUniqueID(uint8_t unique_id) const {
   for (size_t i = 0; i < this->_data.items.size(); ++i) {
      auto& item = this->_data.items[i];
      if (item.declaration.unique_id == unique_id)
         return this->index(i, 0, {});
   }
   return {};
}

void PackageDataModel::_recache_item_value_string(ItemWithCaching& item) {
   auto& dst = item.cached.value;
   dst.clear();
   if (!item.value.has_value()) {
      return;
   }
   const auto& src = item.value.value();
   switch (src.type()) {
      case dovah::packages::package_data_type::boolean:
         {
            auto& casted = src.as<dovah::packages::package_data_type::boolean>();
            if (casted)
               dst = tr("True", "package data value (bool)");
            else
               dst = tr("False", "package data value (bool)");
         }
         break;
      case dovah::packages::package_data_type::float32:
         {
            auto& casted = src.as<dovah::packages::package_data_type::float32>();
            dst = QString::number(casted);
         }
         break;
      case dovah::packages::package_data_type::integer:
         {
            auto& casted = src.as<dovah::packages::package_data_type::integer>();
            dst = QString::number(casted);
         }
         break;
      case dovah::packages::package_data_type::location:
         {
            auto& casted = src.as<dovah::packages::package_data_type::location>();
            QString elaborated;
            bool    use_radius = true;
            switch (casted.get_type()) {
               case dovah::packages::location_type::at_package_location:
                  elaborated = tr("At package location", "package data value (location)");
                  break;
               case dovah::packages::location_type::interior_cell:
                  use_radius = false;
                  {
                     dovah::form_stub* cell = *casted.as_type<dovah::packages::location_type::interior_cell>();
                     if (cell) {
                        const auto& name = cell->editorID;
                        if (name.empty()) {
                           elaborated = editor_helpers::form_identifiers_to_string(cell);
                        } else {
                           elaborated = QString::fromStdString(name);
                        }
                     } else {
                        elaborated = tr("NONE", "package data value (location) (interior cell)");
                     }
                  }
                  break;
               case dovah::packages::location_type::interrupt_override_target:
                  switch (std::get<dovah::packages::interrupt_override_target>(casted.data)) {
                     case dovah::packages::interrupt_override_target::combat_target:
                        elaborated = tr("Near combat target", "package data value (location) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::corpse_to_observe:
                        elaborated = tr("Near observed corpse", "package data value (location) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::ref_to_guard:
                        elaborated = tr("Near ref to guard", "package data value (location) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::threat_to_spectate:
                        elaborated = tr("Near threat to spectate", "package data value (location) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::trespasser:
                        elaborated = tr("Near trespasser", "package data value (location) (interrupt override target)");
                        break;
                     default:
                        elaborated = tr("Near unknown interrupt override target", "package data value (location) (interrupt override target)");
                        break;
                  }
                  break;
               case dovah::packages::location_type::linked_ref:
                  {
                     dovah::form_stub* kywd = *casted.as_type<dovah::packages::location_type::linked_ref>();
                     if (kywd) {
                        const auto& name = kywd->editorID;
                        if (name.empty()) {
                           elaborated = editor_helpers::form_identifiers_to_string(kywd);
                        } else {
                           elaborated = QString::fromStdString(name);
                        }
                        elaborated = tr("Near linked ref with keyword %1", "package data value (location) (linked_ref)").arg(elaborated);
                     } else {
                        elaborated = tr("Near linked ref", "package data value (location) (linked_ref)");
                     }
                  }
                  break;
               case dovah::packages::location_type::location_alias:
                  {
                     auto alias_id = *casted.as_type<dovah::packages::location_type::location_alias>();
                     if (alias_id < 0) {
                        elaborated = tr("Near no location alias", "package data value (location) (location alias)");
                     } else {
                        elaborated = _reference_alias_name(alias_id);
                        if (elaborated.isEmpty()) {
                           elaborated = tr("Near location alias ID #%1", "package data value (location) (location alias)").arg(alias_id);
                        } else {
                           elaborated = tr("Near %1", "package data value (location) (location alias)").arg(elaborated);
                        }
                     }
                  }
                  break;
               case dovah::packages::location_type::near_editor_location:
                  elaborated = tr("Near editor location", "package data value (location)");
                  break;
               case dovah::packages::location_type::near_package_start_location:
                  elaborated = tr("Near package start location", "package data value (location)");
                  break;
               case dovah::packages::location_type::object:
                  {
                     dovah::form_stub* refr = *casted.as_type<dovah::packages::location_type::reference>();
                     if (refr) {
                        const auto& name = refr->editorID;
                        if (name.empty()) {
                           elaborated = editor_helpers::form_identifiers_to_string(refr);
                        } else {
                           elaborated = QString::fromStdString(name);
                        }
                        elaborated = tr("Near object of type %1", "package data value (location) (object)").arg(elaborated);
                     } else {
                        elaborated = tr("NONE", "package data value (location) (object)");
                     }
                  }
                  break;
               case dovah::packages::location_type::object_type:
                  {
                     elaborated = editor::localize::package_object_type(std::get<dovah::packages::object_type>(casted.data));
                     elaborated = tr("Near object of type %1", "package data value (location) (object type)").arg(elaborated);
                  }
                  break;
               case dovah::packages::location_type::reference:
                  {
                     dovah::form_stub* refr = *casted.as_type<dovah::packages::location_type::reference>();
                     if (refr) {
                        const auto& name = refr->editorID;
                        if (name.empty()) {
                           elaborated = editor_helpers::form_identifiers_to_string(refr);
                        } else {
                           elaborated = QString::fromStdString(name);
                        }
                        elaborated = tr("Near %1", "package data value (location) (reference)").arg(elaborated);
                     } else {
                        elaborated = tr("NONE", "package data value (location) (reference)");
                     }
                  }
                  break;
               case dovah::packages::location_type::reference_alias:
                  {
                     auto alias_id = *casted.as_type<dovah::packages::location_type::reference_alias>();
                     if (alias_id < 0) {
                        elaborated = tr("Near no reference alias", "package data value (location) (reference alias)");
                     } else {
                        elaborated = _reference_alias_name(alias_id);
                        if (elaborated.isEmpty()) {
                           elaborated = tr("Near reference alias ID #%1", "package data value (location) (reference alias)").arg(alias_id);
                        } else {
                           elaborated = tr("Near %1", "package data value (location) (reference alias)").arg(elaborated);
                        }
                     }
                  }
                  break;
               case dovah::packages::location_type::self:
                  elaborated = tr("Near self", "package data value (location)");
                  break;
            }
            if (use_radius)
               dst = tr("%1, radius %2").arg(elaborated).arg(casted.radius);
            else
               dst = elaborated;
         }
         break;
      case dovah::packages::package_data_type::object_list:
         {
            auto& casted = src.as<dovah::packages::package_data_type::object_list>();
            dst = QString::number(casted);
         }
         break;
      case dovah::packages::package_data_type::single_ref:
      case dovah::packages::package_data_type::target_selector:
         {
            auto& casted =
               src.is<dovah::packages::package_data_type::single_ref>() ?
                  src.as<dovah::packages::package_data_type::single_ref>()
               :
                  src.as<dovah::packages::package_data_type::target_selector>()
            ;
            switch (casted.get_type()) {
               case dovah::packages::target_type::interrupt_override_target:
                  switch (std::get<dovah::packages::interrupt_override_target>(casted.data)) {
                     case dovah::packages::interrupt_override_target::combat_target:
                        dst = tr("Combat target", "package data value (target) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::corpse_to_observe:
                        dst = tr("Observed corpse", "package data value (target) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::ref_to_guard:
                        dst = tr("Ref to guard", "package data value (target) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::threat_to_spectate:
                        dst = tr("Threat to spectate", "package data value (target) (interrupt override target)");
                        break;
                     case dovah::packages::interrupt_override_target::trespasser:
                        dst = tr("Trespasser", "package data value (target) (interrupt override target)");
                        break;
                     default:
                        dst = tr("Unknown interrupt override target", "package data value (target) (interrupt override target)");
                        break;
                  }
                  break;
               case dovah::packages::target_type::linked_ref:
                  {
                     dovah::form_stub* kywd = *casted.as_type<dovah::packages::target_type::linked_ref>();
                     if (kywd) {
                        const auto& name = kywd->editorID;
                        if (name.empty()) {
                           dst = editor_helpers::form_identifiers_to_string(kywd);
                        } else {
                           dst = QString::fromStdString(name);
                        }
                        dst = tr("Linked ref with keyword %1", "package data value (target) (linked_ref)").arg(dst);
                     } else {
                        dst = tr("Linked ref", "package data value (target) (linked_ref)");
                     }
                  }
                  break;
               case dovah::packages::target_type::object:
                  {
                     dovah::form_stub* refr = *casted.as_type<dovah::packages::target_type::reference>();
                     if (refr) {
                        const auto& name = refr->editorID;
                        if (name.empty()) {
                           dst = editor_helpers::form_identifiers_to_string(refr);
                        } else {
                           dst = QString::fromStdString(name);
                        }
                        dst = tr("Object of type %1", "package data value (location) (object)").arg(dst);
                     } else {
                        dst = tr("NONE", "package data value (location) (object)");
                     }
                  }
                  break;
               case dovah::packages::target_type::object_type:
                  {
                     dst = editor::localize::package_object_type(std::get<dovah::packages::object_type>(casted.data));
                     dst = tr("Object of type %1", "package data value (location) (object type)").arg(dst);
                  }
                  break;
               case dovah::packages::target_type::reference:
                  {
                     dovah::form_stub* refr = *casted.as_type<dovah::packages::target_type::reference>();
                     if (refr) {
                        const auto& name = refr->editorID;
                        if (name.empty()) {
                           dst = editor_helpers::form_identifiers_to_string(refr);
                        } else {
                           dst = QString::fromStdString(name);
                        }
                     } else {
                        dst = tr("NONE", "package data value (location) (reference)");
                     }
                  }
                  break;
               case dovah::packages::target_type::reference_alias:
                  {
                     auto alias_id = *casted.as_type<dovah::packages::target_type::reference_alias>();
                     if (alias_id < 0) {
                        dst = tr("No reference alias", "package data value (location) (reference alias)");
                     } else {
                        dst = _reference_alias_name(alias_id);
                        if (dst.isEmpty()) {
                           dst = tr("Reference alias ID #%1", "package data value (location) (reference alias)").arg(alias_id);
                        }
                     }
                  }
                  break;
               case dovah::packages::target_type::self:
                  dst = tr("Self", "package data value (target)");
                  break;
            }

         }
         break;
      case dovah::packages::package_data_type::topic:
         {
            auto& casted = src.as<dovah::packages::package_data_type::topic>();
            if (auto subtype = casted.get_subtype_signature()) {
               for (auto& info : dovah::dialogue::all_topic_subtypes) {
                  if (subtype == info.signature) {
                     dst = editor::localize::dialogue_topic_subtype(info);
                     dst = tr("(%1)", "package data value (topic) (subtype)").arg(dst);
                     break;
                  }
               }
            } else if (auto* topic = casted.get_topic()) {
               const auto& name = topic->editorID;
               if (name.empty()) {
                  dst = editor_helpers::form_identifiers_to_string(topic);
               } else {
                  dst = QString::fromStdString(name);
               }
            } else {
               dst = tr("NONE", "package data value (topic)");
            }
         }
         break;
   }
}
void PackageDataModel::_recache_quest_aliases() {
   if (!this->_data.owning_quest) {
      this->_cached.alias_names.location.clear();
      this->_cached.alias_names.reference.clear();
      return;
   }
   auto loaded = this->_data.owning_quest->load().ptr_cast<dovah::loaded_forms::Quest>();
   if (!loaded) {
      this->_cached.alias_names.location.clear();
      this->_cached.alias_names.reference.clear();
      return;
   }
   for (auto* alias : loaded->aliases) {
      auto name = QString::fromStdString(alias->name);
      if (alias->type == dovah::loaded_forms::Alias::alias_type::location) {
         this->_cached.alias_names.location[alias->id] = name;
      } else if (alias->type == dovah::loaded_forms::Alias::alias_type::reference) {
         this->_cached.alias_names.reference[alias->id] = name;
      }
   }
}
void PackageDataModel::_recache_item_values_using_aliases() {
   for (size_t i = 0; i < this->_data.items.size(); ++i) {
      auto& item = this->_data.items[i];
      if (!item.value.has_value())
         continue;
      auto& value = item.value.value();
      if (value.type() != dovah::packages::package_data_type::location)
         continue;
      switch (value.as<dovah::packages::package_data_type::location>().get_type()) {
         case dovah::packages::location_type::location_alias:
         case dovah::packages::location_type::reference_alias:
            this->_recache_item_value_string(item);
            {
               auto qmi = this->index(i, Column::Value, {});
               emit dataChanged(qmi, qmi);
            }
            break;
      }
   }
}

const int PackageDataModel::_row_for_unique_id(uint8_t id) const {
   for (size_t i = 0; i < this->_data.items.size(); ++i)
      if (this->_data.items[i].declaration.unique_id == id)
         return i;
   return -1;
}
const PackageDataModel::ItemWithCaching* PackageDataModel::_item_by_unique_id(uint8_t id) const {
   auto i = this->_row_for_unique_id(id);
   if (i >= 0)
      return &this->_data.items[i];
   return nullptr;
}

void PackageDataModel::_on_form_modified(dovah::form_stub& stub) {
   if (stub.form_type == dovah::form_type::quest) {
      if (&stub != this->_data.owning_quest)
         return;
      this->_recache_quest_aliases();
      this->_recache_item_values_using_aliases();
      return;
   }

   for (size_t i = 0; i < this->_data.items.size(); ++i) {
      auto& item = this->_data.items[i];
      auto& value_opt = item.value;
      if (!value_opt.has_value())
         continue;
      auto& value = value_opt.value();
      switch (value.type()) {
         case dovah::packages::package_data_type::boolean:
         case dovah::packages::package_data_type::float32:
         case dovah::packages::package_data_type::integer:
         case dovah::packages::package_data_type::object_list:
            continue;
      }
      //
      // TODO: Only react if the item's value (or stringification thereof) 
      // has actually changed.
      //
      this->_recache_item_value_string(item);
      auto qmi = this->index(i, Column::Value, {});
      emit dataChanged(qmi, qmi);
   }
}
void PackageDataModel::_sever_uses_of_form(dovah::form_stub& stub) {
   for(size_t i = 0; i < this->_data.items.size(); ++i) {
      auto& item      = this->_data.items[i];
      auto& value_opt = item.value;
      if (!value_opt.has_value())
         continue;
      auto& value = value_opt.value();
      if (value.sever_uses_of_form(stub)) {
         this->_recache_item_value_string(item);
         auto qmi = this->index(i, Column::Value, {});
         emit dataChanged(qmi, qmi);
      }
   }
}

uint8_t PackageDataModel::_get_available_unique_id() const {
   if (this->_data.next_unique_id != ui::types::packages::package_data_declaration::no_unique_id) {
      return this->_data.next_unique_id;
   }

   cobb::bitset<255> used;
   for (auto& item : this->_data.items) {
      auto id = item.declaration.unique_id;
      if (id < used.size())
         used.set(id);
   }
   auto free = used.find_first_clear();
   if (free < 0) {
      return ui::types::packages::package_data_declaration::no_unique_id;
   }
   return free;
}

QString PackageDataModel::_location_alias_name(int32_t id) const {
   auto& map = this->_cached.alias_names.location;
   auto  it = map.find(id);
   if (it != map.end()) {
      return *it;
   }
   return {};
}
QString PackageDataModel::_reference_alias_name(int32_t id) const {
   auto& map = this->_cached.alias_names.reference;
   auto  it  = map.find(id);
   if (it != map.end()) {
      return *it;
   }
   return {};
}