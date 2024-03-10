#include "./DKFormVMADModel.h"
#include <cassert>
#include <QIcon>
#include "helpers/type_traits/is_std_vector.h"
#include "helpers/type_containers/fixed_map.h"
#include "helpers/function_traits.h"
#include "dovah/forms/components/papyrus.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/ObjectReference.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/core.h"

// For loading PEXs:
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/papyrus/compiled_script.h"
#include "editor/subsystems/assets.h"

// Config
namespace {
   constexpr const bool show_raw_statuses = false;

   constexpr const bool allow_incomplete_polishing = true;
}

// Type aliases
namespace {
   namespace model {
      using property_value = DKFormVMADModel::property_value;
   }
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
      using object_property_value = property_object_value;
   }
}

namespace {
   // This is such a stupid thing for me to even have to do... Basically, Qt's treeviews 
   // can display icons in cells, BUT if only some cells in a column have icons, the others 
   // don't reserve space for an icon -- even if you set an icon size of the treeview, and 
   // even if you give those other cells default-constructed QIcons.
   static QIcon& _blank_icon() {
      static QIcon icon([]() {
         QPixmap pixmap(16, 16); // also, QTreeView::setIconSize doesn't enlarge icons to match the desired size. what *does* it do?
         pixmap.fill(QColorConstants::Transparent);
         return pixmap;
      }());
      return icon;
   }
}

#pragma region DKFormVMADModel
DKFormVMADModel::DKFormVMADModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &DKFormVMADModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formModified,           this, &DKFormVMADModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,         this, &DKFormVMADModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &DKFormVMADModel::unsetWorkingVMAD);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &DKFormVMADModel::formsRenumberedEnMasse);
}

DKFormVMADModel::~DKFormVMADModel() {
   for (auto* item : this->scripts) {
      assert(item);
      delete item;
   }
   this->scripts.clear();
}

bool DKFormVMADModel::indexIsScript(const QModelIndex qmi) const {
   if (!qmi.isValid())
      return false;
   if (qmi.model() != this)
      return false;

   if (qmi.row() >= this->scripts.size())
      return false;
   for (auto* script : this->scripts)
      if (qmi.internalPointer() == script)
         return true;
   return false;
}
bool DKFormVMADModel::indexIsProperty(const QModelIndex qmi) const {
   if (!qmi.isValid())
      return false;
   if (qmi.model() != this)
      return false;

   if (qmi.row() < this->scripts.size()) {
      for (auto* script : this->scripts)
         if (qmi.internalPointer() == script)
            return false;
   }
   for (const auto* script : this->scripts) {
      for (const auto* prop : script->properties)
         if (qmi.internalPointer() == prop)
            return true;
   }
   return false;
}

dovah::form_stub* DKFormVMADModel::_getBaseForm() const {
   if (!this->vmads.parent)
      return nullptr;
   if (!dovah::form_type_is_reference(this->attached_to->stub.form_type))
      return nullptr;
   auto* base = ((dovah::loaded_forms::ObjectReference*)this->attached_to)->base_form.get_form_stub();
   return base;
}
DKFormVMADModel::Script* DKFormVMADModel::_getContainingScript(QModelIndex prop_qmi) {
   if (!prop_qmi.isValid())
      return nullptr;
   for (auto* script : this->scripts)
      for (auto* prop : script->properties)
         if (prop == prop_qmi.internalPointer())
            return script;
   return nullptr;
}

#pragma region Editor core hooks
void DKFormVMADModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (this->attached_to == nullptr) {
      assert(this->scripts.isEmpty());
      return;
   }
   if (stub == &this->attached_to->stub && !is_just_flagged) {
      this->unsetWorkingVMAD();
      return;
   }

   bool base_form_removed_under_us = this->_getBaseForm() == stub;
   std::vector<size_t> scripts_to_remove;

   for (size_t i = 0; i < this->scripts.size(); ++i) {
      auto* script = this->scripts[i];
      for (size_t j = 0; j < script->properties.size(); ++j) {
         auto* prop = script->properties[j];

         bool changed = false;
         //
         if (base_form_removed_under_us) {
            if (prop->bindings.parent.has_value()) {
               //
               // We don't call `prop->clearParentBinding()` here because that recaches the value 
               // string, which may be redundant if `onFormDeletionImminent` below also changes 
               // the property's effective value.
               //
               prop->bindings.parent.reset();
               changed = true;
            }
         }
         if (prop->on_form_deletion_imminent(*stub)) {
            changed = true;
         }
         //
         if (changed) {
            prop->recache_value_string();

            QModelIndex qmi_script = this->index(i, 0, {});
            QModelIndex qmi_l      = this->index(j, 0, qmi_script);
            QModelIndex qmi_r      = this->index(j, PropertyColumnCount - 1, qmi_script);

            emit dataChanged(qmi_l, qmi_r);
         }
      }

      if (base_form_removed_under_us) {
         if (!script->statuses.target.has_value()) {
            //
            // If we make it here, then the script was only attached on the parent; it should be removed 
            // from the model entirely. We shouldn't remove it right this second, because we're still 
            // iterating the list and knocking elements out of it might mess with iterators.
            // 
            // We intentionally do not do this kind of cleanup for properties that are only defined on 
            // the base: properties loaded from the PEX should always remain in the model.
            //
            scripts_to_remove.push_back(i);
         }
      }
   }

   if (base_form_removed_under_us) {
      //
      // Iterate the `scripts_to_remove` list in reverse order. This reduces the number of elements that 
      // are displaced with each removal, and it means that the indices in `scripts_to_remove` are not 
      // themselves displaced as we carry out removals. (If we iterated `scripts_to_remove` in forward 
      // order, then each index we remove would come after the last index we removed, and so each index 
      // would displace all successive ones by -1.)
      //
      for (auto it = scripts_to_remove.rbegin(); it != scripts_to_remove.rend(); ++it) {
         size_t i = *it;
         
         this->beginRemoveRows({}, i, i);
         {
            auto* script = this->scripts[i];
            this->scripts[i] = nullptr;
            delete script;
         }
         this->scripts.erase(this->scripts.begin() + i);
         this->endRemoveRows();
      }
   }
}
void DKFormVMADModel::formModified(dovah::form_stub* stub) {
   if (auto* base = this->_getBaseForm(); base == stub) {
      // Handle removed scripts:
      {
         std::vector<size_t> scripts_to_remove;
         
         for (size_t i = 0; i < this->scripts.size(); ++i) {
            auto* script = this->scripts[i];
            if (!script->statuses.parent.has_value())
               continue;

            std::string scriptname = script->name.toStdString();
            auto* src = this->vmads.parent->lookup_script(scriptname);
            if (!src) {
               script->statuses.parent.reset();
               if (!script->statuses.target.has_value()) {
                  scripts_to_remove.push_back(i);
               }
            }
         }
         
         for (auto it = scripts_to_remove.rbegin(); it != scripts_to_remove.rend(); ++it) {
            size_t i = *it;
         
            this->beginRemoveRows({}, i, i);
            {
               auto* script = this->scripts[i];
               this->scripts[i] = nullptr;
               delete script;
            }
            this->scripts.erase(this->scripts.begin() + i);
            this->endRemoveRows();
         }
      }

      // Handle added or edited scripts:
      for (const auto& src : this->vmads.parent->scripts) {
         QString scriptname = QString::fromStdString(src.name);

         Script* dst = nullptr;
         for (auto* script : this->scripts) {
            if (script->name_matches(scriptname)) {
               dst = script;
               break;
            }
         }
         
         bool is_new_script = !dst;
         if (is_new_script) {
            size_t at;
            for (at = 0; at < this->scripts.size(); ++at) {
               if (this->scripts[at]->name < scriptname) {
                  break;
               }
            }
            this->beginInsertRows({}, at, at);

            dst = new Script;
            this->scripts.insert(at, dst);
            dst->name = scriptname;
            dst->load_property_definitions();
         }
         dst->statuses.parent = src.status;

         dst->properties.reserve(src.properties.size());
         for (const auto& prop : src.properties) {
            QString prop_name = QString::fromStdString(src.name);

            auto* dst_prop        = dst->lookup_property(prop_name);
            bool  is_new_property = !dst_prop;
            if (is_new_property) {
               if (!is_new_script) {
                  this->beginInsertRows({}, dst->properties.size(), dst->properties.size());
               }
               dst_prop = new Property;
               dst->properties.push_back(dst_prop);
               dst_prop->name = prop_name;
            }
            dst->load_parent_property_value(prop);
            if (!is_new_script && is_new_property) {
               this->endInsertRows();
            }
         }

         if (is_new_script) {
            this->endInsertRows();
         }
      }
   }

   for (size_t i = 0; i < this->scripts.size(); ++i) {
      auto* script = this->scripts[i];
      for (size_t j = 0; j < script->properties.size(); ++j) {
         auto* prop = script->properties[j];

         if (prop->refers_to_form(*stub)) {
            prop->recache_value_string();
            auto index = this->index(j, PropertyColumn::Value, this->index(i, 0, {}));
            emit dataChanged(index, index);
         }
      }
   }
}
void DKFormVMADModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   for (size_t i = 0; i < this->scripts.size(); ++i) {
      auto* script = this->scripts[i];
      for (size_t j = 0; j < script->properties.size(); ++j) {
         auto* prop = script->properties[j];

         if (prop->refers_to_form(*stub)) {
            prop->recache_value_string();
            auto index = this->index(j, PropertyColumn::Value, this->index(i, 0, {}));
            emit dataChanged(index, index);
         }
      }
   }
}
void DKFormVMADModel::formsRenumberedEnMasse() {
   //
   // We don't store enough information to check which list items have had their 
   // form IDs changed, so just blindly update the form IDs for all list items.
   //
   for (size_t i = 0; i < this->scripts.size(); ++i) {
      auto* script = this->scripts[i];

      bool any = false;
      for (auto* prop : script->properties) {
         if (prop->is_object_or_object_array()) {
            prop->recache_value_string();
            any = true;
         }
      }
      if (any) {
         QModelIndex script_qmi = this->index(i, 0, {});

         QModelIndex upper_left  = this->index(0, PropertyColumn::Value, script_qmi);
         QModelIndex lower_right = this->index(script->properties.size() - 1, PropertyColumn::Value, script_qmi);
         emit dataChanged(upper_left, lower_right);
      }
   }
}
#pragma endregion


void DKFormVMADModel::setWorkingVMAD(working_copy_type& working_copy, vmad_data& target) {
   if (this->attached_to == &working_copy && this->vmads.target == &target && this->vmads.parent == nullptr)
      return;

   this->beginResetModel();

   this->attached_to  = &working_copy;
   this->vmads.target = &target;
   this->vmads.parent = nullptr;

   this->scripts.clear();
   //
   QMap<QString, Script*> working;
   for (const auto& src : target.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& ptr = working[name_lower];
      if (!ptr) {
         ptr = new Script;
         ptr->name = name;
         ptr->load_property_definitions();
      }
      ptr->statuses.target = src.status;

      ptr->properties.reserve(src.properties.size());
      for (const auto& prop : src.properties) {
         ptr->load_target_property_value(prop);
      }
   }
   //
   this->scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->scripts.push_back(it->second);
      for (auto* prop : it->second->properties)
         prop->recache_value_string();
   }

   this->endResetModel();
}
void DKFormVMADModel::setWorkingVMAD(working_copy_type& working_copy, vmad_data& target, vmad_data& parent) {
   if (this->attached_to == &working_copy && this->vmads.target == &target && this->vmads.parent == &parent)
      return;

   this->beginResetModel();

   this->attached_to  = &working_copy;
   this->vmads.target = &target;
   this->vmads.parent = &parent;

   this->scripts.clear();
   //
   QMap<QString, Script*> working;
   for (const auto& src : parent.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& ptr = working[name_lower];
      if (!ptr) {
         ptr = new Script;
         ptr->name = name;
         ptr->load_property_definitions();
      }
      ptr->statuses.parent = src.status;

      ptr->properties.reserve(src.properties.size());
      for (const auto& prop : src.properties) {
         ptr->load_parent_property_value(prop);
      }
   }
   for (const auto& src : target.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& ptr = working[name_lower];
      if (!ptr) {
         ptr = new Script;
         ptr->name = name;
         ptr->load_property_definitions();
      }
      ptr->statuses.target = src.status;

      ptr->properties.reserve(ptr->properties.size() + src.properties.size());
      for (const auto& prop : src.properties) {
         ptr->load_target_property_value(prop);
      }
      ptr->properties.shrink_to_fit();
   }
   //
   this->scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->scripts.push_back(it->second);
      for (auto* prop : it->second->properties)
         prop->recache_value_string();
   }

   this->endResetModel();
}
void DKFormVMADModel::commitToWorkingVMAD() {
   if (this->attached_to == nullptr)
      return;
   if (this->vmads.target == nullptr)
      return;

   auto* loaded = this->attached_to;
   assert(loaded);

   this->vmads.target->clear_scripts(*loaded);
   for (const auto* script : this->scripts) {
      auto& target_status_opt = script->statuses.target;
      auto& parent_status_opt = script->statuses.parent;
      if (target_status_opt.has_value()) {
         auto& dst = this->vmads.target->scripts.emplace_back();
         dst.name   = script->name.toStdString();
         dst.status = target_status_opt.value();
         
         // Commit properties:
         for (const auto* property : script->properties) {
            auto& target_bind_opt = property->bindings.target;
            auto& parent_bind_opt = property->bindings.parent;

            if (target_bind_opt.has_value()) {
               auto& dst_prop = dst.properties.emplace_back();

               auto& target_bind = target_bind_opt.value();
               if (target_bind.status == property_status::inherited_and_removed) {
                  dst_prop.status = target_bind.status;
                  dst_prop.value  = {};
               } else {
                  dst_prop.status = target_bind.status;
                  DKFormVMADModelObjects::property_value_to_vmad(target_bind.value, dst_prop.value, *loaded);
               }
               continue;
            }
         }
      }
   }
}
void DKFormVMADModel::unsetWorkingVMAD() {
   this->beginResetModel();
   this->attached_to = nullptr;
   this->vmads = {};
   this->scripts.clear();
   this->endResetModel();
}

#pragma region QAbstractItemModel overrides
QModelIndex DKFormVMADModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return {};
   if (row < 0)
      return {};
   if (this->indexIsScript(parent)) {
      auto* script = (Script*)parent.internalPointer();
      if (row >= 0 && row < script->properties.size())
         return this->createIndex(row, column, (void*)script->properties[row]);
   } else {
      if (row >= 0 && row < this->scripts.size())
         return this->createIndex(row, column, (void*)this->scripts[row]);
   }
   return {};
}
QModelIndex DKFormVMADModel::parent(const QModelIndex& qmi) const {
   if (!qmi.isValid() || qmi.model() != this)
      return {};

   const void* ptr = qmi.internalPointer();
   for (size_t i = 0; i < this->scripts.size(); ++i) {
      const auto* script = this->scripts[i];
      if (script == ptr)
         return {};
      for (const auto* prop : script->properties) {
         if (prop == ptr)
            return this->createIndex(i, 0, (void*)script);
      }
   }

   return {};
}
int DKFormVMADModel::rowCount(const QModelIndex& qmi) const {
   if (!qmi.isValid()) {
      if (qmi.column() > 0)
         return 0;
      return this->scripts.size();
   }
   if (qmi.model() != this)
      return 0;

   int count = 0;
   this->handleIndexByType(
      qmi,
      [&count](QModelIndex, const Script* script) {
         count = script->properties.size();
      },
      [&count](QModelIndex, const Property* prop) {
         count = 0; // TODO: Do we want to represent property values as child nodes? Could help with arrays, maybe?
      }
   );
   return count;
}
int DKFormVMADModel::columnCount(const QModelIndex& qmi) const {
   if (!qmi.isValid())
      return 1;
   if (this->indexIsScript(qmi))
      return PropertyColumnCount;
   return 0;
}
Qt::ItemFlags DKFormVMADModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return {};
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DKFormVMADModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return {};
   if (index.internalPointer() == nullptr)
      return {};

   QVariant out;
   this->handleIndexByType(
      index,
      [&out, role](QModelIndex qmi, const Script* script) -> void {
         switch (role) {
            case Qt::DisplayRole:
               out = script->name;
               return;
            case Qt::ToolTipRole:
               {
                  auto status_opt = script->get_computed_status();
                  if (!status_opt.has_value()) {
                     out = tr("Status: <unknown>");
                     return;
                  }
                  switch (status_opt.value()) {
                     case script_status::defined_locally:
                        if (script->properties_set_on_target) {
                           out = tr("Status: Script added and edited locally");
                        } else {
                           out = tr("Status: Script added locally");
                        }
                        return;
                     case script_status::overrides_base:
                        out = tr("Status: Script inherited and edited locally");
                        return;
                     case script_status::defined_on_base:
                        out = tr("Status: Script inherited from parent");
                        return;
                     case script_status::removed:
                        out = tr("Status: Script inherited and deleted locally");
                        return;
                  }
               }
               break;
            case Qt::DecorationRole:
               {
                  auto status_opt = script->get_computed_status();
                  if (!status_opt.has_value()) {
                     out = _blank_icon(); // Call QTableView::setIconSize to ensure there's always space reserved for icons.
                     return;
                  }
                  switch (status_opt.value()) {
                     case script_status::defined_locally:
                     default:
                        if (script->properties_set_on_target) {
                           out = QIcon(":/icons/papyrus-status-icons/added-edited.png");
                        } else {
                           out = QIcon(":/icons/papyrus-status-icons/added.png");
                        }
                        return;
                     case script_status::overrides_base:
                     case script_status::defined_on_base:
                        if (script->properties_set_on_target) {
                           out = QIcon(":/icons/papyrus-status-icons/inherited-edited.png");
                        } else {
                           out = QIcon(":/icons/papyrus-status-icons/inherited.png");
                        }
                        return;
                     case script_status::removed:
                        out = QIcon(":/icons/papyrus-status-icons/removed.png");
                        return;
                  }
               }
               break;
         }
      },
      [&out, role](QModelIndex qmi, const Property* prop) -> void {
         switch (qmi.column()) {
            case PropertyColumn::Name:
               switch (role) {
                  case Qt::DisplayRole:
                     out = prop->name;
                     return;
                  case Qt::ToolTipRole:
                     {
                        QString tooltip;

                        auto status_opt = prop->get_computed_status();
                        if (!status_opt.has_value()) {
                           tooltip = tr("Status: Property unmodified");
                        } else {
                           switch (status_opt.value()) {
                              case property_status::defined_locally:
                                 if (prop->bindings.parent.has_value()) {
                                    tooltip = tr("Status: Property inherited and edited locally");
                                 } else {
                                    tooltip = tr("Status: Property edited locally");
                                 }
                                 break;
                              case property_status::defined_only_on_base:
                                 tooltip = tr("Status: Property inherited from parent");
                                 break;
                              case property_status::inherited_and_removed:
                                 tooltip = tr("Status: Property inherited and cleared locally");
                                 break;
                              default:
                                 tooltip = tr("Status: <<unknown>>");
                                 break;
                           }
                        }
                        if (!prop->docstring.isEmpty()) {
                           tooltip += "\n\n" + prop->docstring;
                        }
                        out = tooltip;
                        return;
                     }
                     break;
                  case Qt::DecorationRole:
                     {
                        auto status_opt = prop->get_computed_status();
                        if (!status_opt.has_value()) {
                           out = _blank_icon(); // Call QTableView::setIconSize to ensure there's always space reserved for icons.
                           return;
                        }
                        switch (status_opt.value()) {
                           case property_status::defined_locally:
                              if (prop->bindings.parent.has_value()) {
                                 out = QIcon(":/icons/papyrus-status-icons/inherited-edited.png");
                              } else {
                                 out = QIcon(":/icons/papyrus-status-icons/added-edited.png");
                              }
                              return;
                           case property_status::defined_only_on_base:
                              out = QIcon(":/icons/papyrus-status-icons/inherited.png");
                              return;
                           case property_status::inherited_and_removed:
                              out = QIcon(":/icons/papyrus-status-icons/removed.png");
                              return;
                        }
                     }
                     break;
               }
               return;
            case PropertyColumn::Type:
               if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
                  out = prop->type_string();
               }
               return;
            case PropertyColumn::Value:
               if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
                  out = prop->value_string;
               }
               return;
         }

      }
   );
   return out;
}
QVariant DKFormVMADModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal) {
      return {};
   }
   if (role == Qt::DisplayRole) {
      switch (section) {
         case ScriptColumn::Name:
            return tr("Name");

         case PropertyColumn::Type:
            return tr("Type");
         case PropertyColumn::Value:
            return tr("Value");
      }
   }
   if (role == Qt::TextAlignmentRole) {
      return (int)(Qt::AlignBaseline | Qt::AlignLeading);
   }
   return {};
}
#pragma endregion

const DKFormVMADModel::ScriptMetadata DKFormVMADModel::getScriptMetadata(QModelIndex qmi) const noexcept {
   if (!this->indexIsScript(qmi))
      return {};
   auto* script       = (const Script*)qmi.internalPointer();
   bool  is_on_parent = script->statuses.parent.has_value();
   bool  is_on_target = script->statuses.target.has_value();
   return {
      .attached_on_parent    = is_on_parent,
      .attached_on_target    = is_on_target,
      .inherited_and_removed = is_on_parent && is_on_target && script->get_computed_status() != script_status::removed,
   };
}
QModelIndex DKFormVMADModel::scriptIndex(QString scriptname) const {
   size_t size = this->scripts.size();
   for (size_t i = 0; i < size; ++i) {
      const auto* item = this->scripts[i];
      if (item->name_matches(scriptname))
         return this->index(i, 0, {});
   }
   return {};
}
QModelIndex DKFormVMADModel::propertyIndex(QModelIndex script_qmi, QString name) const {
   if (!script_qmi.isValid())
      return {};

   assert(this->indexIsScript(script_qmi));
   const auto* script = this->scripts[script_qmi.row()];
   const auto& list   = script->properties;

   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      const auto* prop = list[i];
      if (prop->name_matches(name))
         return this->index(i, 0, script_qmi);
   }
   return {};
}

QModelIndex DKFormVMADModel::addScript(QString scriptname) {
   auto qmi = this->scriptIndex(scriptname);
   if (qmi.isValid())
      return qmi;

   size_t insert_at;
   for (insert_at = 0; insert_at < this->scripts.size(); ++insert_at) {
      auto* item = this->scripts[insert_at];
      if (item->name < scriptname)
         break;
   }
   this->beginInsertRows({}, insert_at, insert_at);
   {
      auto* item = new Script;
      this->scripts.insert(insert_at, item);
      item->name = scriptname;
      item->statuses.target = script_status::defined_locally;
   }
   this->endInsertRows();
   return this->index(insert_at, 0, {});
}
void DKFormVMADModel::removeScript(QModelIndex qmi) {
   assert(this->indexIsScript(qmi));

   auto i = qmi.row();
   if (this->vmads.parent) {
      auto* script = this->scripts[i];
      if (script->statuses.parent.has_value()) {
         script->statuses.target = script_status::removed;
         //
         auto qmi = this->index(i, 0, {});
         {
            auto tl = this->index(0, 0, qmi);
            auto br = this->index(script->properties.size() - 1, PropertyColumnCount, qmi);
            for (auto* prop : script->properties) {
               prop->erase_non_inherited_value();
            }
            emit dataChanged(tl, br);
         }
         emit dataChanged(qmi, qmi);
         //
         return;
      }
   }
   this->beginRemoveRows({}, i, i);
   delete this->scripts[i];
   this->scripts.removeAt(i);
   this->endRemoveRows();
}
void DKFormVMADModel::undeleteInheritedScript(QModelIndex qmi) {
   assert(this->indexIsScript(qmi));

   auto* script = this->scripts[qmi.row()];
   if (!script->statuses.parent.has_value())
      return;
   auto& s_target = script->statuses.target;
   if (!s_target.has_value())
      return;
   if (s_target.value() != script_status::removed)
      return;

   s_target = script_status::defined_on_base;
   //
   qmi = qmi.siblingAtColumn(0);
   {
      auto tl = this->index(0, 0, qmi);
      auto br = this->index(script->properties.size() - 1, PropertyColumnCount, qmi);
      for (auto* prop : script->properties) {
         prop->erase_non_inherited_value();
      }
      emit dataChanged(tl, br);
   }
   emit dataChanged(qmi, qmi);
}

void DKFormVMADModel::clearPropertyValue(QModelIndex qmi) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return;
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);

   {
      auto& v_opt = prop->bindings.target;
      if (!v_opt.has_value())
         v_opt.emplace();
      auto& v_bind = v_opt.value();
      //
      v_bind.value  = {};
      v_bind.status = vmad::property_status::inherited_and_removed;
   }
   prop->recache_value_string();

   QModelIndex qmi_l = qmi.siblingAtColumn(PropertyColumn::Name);
   QModelIndex qmi_r = qmi.siblingAtColumn(PropertyColumn::Value);
   emit dataChanged(qmi_l, qmi_r);
}
void DKFormVMADModel::revertPropertyValue(QModelIndex qmi) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return;
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);

   prop->bindings.target.reset();
   prop->recache_value_string();

   QModelIndex qmi_l = qmi.siblingAtColumn(PropertyColumn::Name);
   QModelIndex qmi_r = qmi.siblingAtColumn(PropertyColumn::Value);
   emit dataChanged(qmi_l, qmi_r);
}
void DKFormVMADModel::setPropertyValue(QModelIndex qmi, const property_value& data) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return;
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);
   prop->set_non_inherited_value(data, true);
   
   QModelIndex qmi_l = qmi.siblingAtColumn(PropertyColumn::Name);
   QModelIndex qmi_r = qmi.siblingAtColumn(PropertyColumn::Value);
   emit dataChanged(qmi_l, qmi_r);
}


std::optional<DKFormVMADModel::property_status> DKFormVMADModel::getPropertyWorkingStatus(QModelIndex qmi) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return {};
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);

   if (prop->bindings.edited.has_value()) {
      return prop->bindings.edited.value().status;
   }
   return prop->get_computed_status();
}
DKFormVMADModel::property_value DKFormVMADModel::getPropertyWorkingValue(QModelIndex qmi) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return {};
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);

   if (prop->bindings.edited.has_value()) {
      return prop->bindings.edited.value().value;
   }
   if (prop->bindings.target.has_value()) {
      return prop->bindings.target.value().value;
   }
   if (prop->bindings.parent.has_value()) {
      return prop->bindings.parent.value().value;
   }

   return prop->make_empty_value();
}
void DKFormVMADModel::clearPropertyWorkingValue(QModelIndex qmi) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return;
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);
   prop->clear_non_inherited_value();

   QModelIndex qmi_l = qmi.siblingAtColumn(PropertyColumn::Name);
   QModelIndex qmi_r = qmi.siblingAtColumn(PropertyColumn::Value);
   emit dataChanged(qmi_l, qmi_r);
}
void DKFormVMADModel::revertPropertyWorkingValue(QModelIndex qmi) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return;
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);
   prop->bindings.edited.reset();
   prop->recache_value_string();

   QModelIndex qmi_l = qmi.siblingAtColumn(PropertyColumn::Name);
   QModelIndex qmi_r = qmi.siblingAtColumn(PropertyColumn::Value);
   emit dataChanged(qmi_l, qmi_r);
}
void DKFormVMADModel::setPropertyWorkingValue(QModelIndex qmi, const property_value& data) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return;
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);
   prop->set_non_inherited_value(data);
   
   QModelIndex qmi_l = qmi.siblingAtColumn(PropertyColumn::Name);
   QModelIndex qmi_r = qmi.siblingAtColumn(PropertyColumn::Value);
   emit dataChanged(qmi_l, qmi_r);
}

void DKFormVMADModel::commitScriptWorkingProperties(QModelIndex script_qmi) {
   assert(this->indexIsScript(script_qmi));

   auto* script = this->scripts[script_qmi.row()];
   assert(script != nullptr);

   for (size_t i = 0; i < script->properties.size(); ++i) {
      auto* prop = script->properties[i];
      prop->bindings.target = prop->bindings.edited;
      prop->bindings.edited.reset();
      prop->recache_value_string();

      auto tl = this->index(i, PropertyColumn::Name,  script_qmi); // to refresh status icon
      auto br = this->index(i, PropertyColumn::Value, script_qmi);
      emit dataChanged(tl, br);
   }
}
void DKFormVMADModel::discardScriptWorkingProperties(QModelIndex script_qmi) {
   assert(this->indexIsScript(script_qmi));

   auto* script = this->scripts[script_qmi.row()];
   assert(script != nullptr);

   for (size_t i = 0; i < script->properties.size(); ++i) {
      auto* prop = script->properties[i];
      prop->bindings.edited.reset();
      
      auto tl = this->index(i, PropertyColumn::Name,  script_qmi); // to refresh status icon
      auto br = this->index(i, PropertyColumn::Value, script_qmi);
      emit dataChanged(tl, br);
   }
}

QString DKFormVMADModel::getPropertyWorkingValueStringified(QModelIndex qmi, size_t array_index) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return {};
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);

   auto _stringify = [](const property_value& value, size_t array_index = 0) -> QString {
      QString out;
      std::visit(
         [&out, array_index](const auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               if (array_index <= v.size())
                  out = DKFormVMADModelObjects::stringify(v[array_index]);
            } else {
               out = DKFormVMADModelObjects::stringify(v);
            }
         },
         value
      );
      return out;
   };
   
   if (auto& bind_opt = prop->bindings.edited; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      switch (bind.status) {
         case property_status::unknown:
         case property_status::defined_locally:
            return _stringify(bind.value, array_index);
         case property_status::inherited_and_removed:
            return "None";
      }
   }
   if (auto& bind_opt = prop->bindings.target; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      switch (bind.status) {
         case property_status::unknown:
         case property_status::defined_locally:
            return _stringify(bind.value, array_index);
         case property_status::inherited_and_removed:
            return "None";
      }
   }
   if (auto& bind_opt = prop->bindings.parent; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      switch (bind.status) {
         case property_status::defined_locally:
         case property_status::defined_only_on_base:
            return _stringify(bind.value, array_index);
      }
   }
   return "<<Default>>";
}

DKFormVMADModel::PropertyMetadata DKFormVMADModel::getPropertyWorkingMetadata(QModelIndex qmi) {
   Property* prop;
   {
      auto* script = this->_getContainingScript(qmi);
      if (!script)
         return {};
      prop = (Property*)qmi.internalPointer();
      assert(prop != nullptr);
   }

   PropertyMetadata out;

   if (prop->bindings.parent.has_value()) {
      out.is_inherited = true;
   }

   if (prop->bindings.edited.has_value()) {
      out.status = prop->bindings.edited.value().status;
   } else {
      out.status = prop->get_computed_status();
   }

   if (prop->is_object_or_object_array()) {
      auto& nt = prop->typeinfo.object_info.native_type;
      if (nt.has_value()) {
         out.underlying_form_typeinfo.type = nt.value();
      } else {
         out.underlying_form_typeinfo.not_a_form = true;
      }
      if (prop->typeinfo.object_info_available())
         out.underlying_form_typeinfo.unidentified = false;

      if (prop->typeinfo.object_info.definition || !nt.has_value())
         out.typeinfo.scriptname = prop->typeinfo.name;

      if (prop->is_quest_alias_or_array_thereof()) {
         out.typeinfo.underlying.base = ui::types::papyrus::single_value_type::alias;
         out.underlying_form_typeinfo.not_a_form = true;
      } else {
         out.typeinfo.underlying.base = ui::types::papyrus::single_value_type::form;
      }
   } else {
      switch (vmad::scalar_property_type_for(prop->typeinfo.raw_type)) {
         case vmad::property_type::boolean:
            out.typeinfo.underlying.base = ui::types::papyrus::single_value_type::boolean;
            break;
         case vmad::property_type::float32:
            out.typeinfo.underlying.base = ui::types::papyrus::single_value_type::float32;
            break;
         case vmad::property_type::integer:
            out.typeinfo.underlying.base = ui::types::papyrus::single_value_type::integer;
            break;
         case vmad::property_type::string:
            out.typeinfo.underlying.base = ui::types::papyrus::single_value_type::string;
            break;
      }
   }
   out.typeinfo.underlying.is_array = vmad::scalar_property_type_for(prop->typeinfo.raw_type) != prop->typeinfo.raw_type;

   out.typeinfo.display_typename = prop->type_string();

   return out;
}

std::vector<std::string> DKFormVMADModel::getAllBoundScripts() const {
   std::vector<std::string> names;
   for (const auto* script : this->scripts) {
      names.push_back(script->name.toUtf8().toStdString());
   }
   return names;
}
#pragma endregion