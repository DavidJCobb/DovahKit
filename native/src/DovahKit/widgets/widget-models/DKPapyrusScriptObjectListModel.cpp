#include "DKPapyrusScriptObjectListModel.h"
#include <cassert>
#include <string>
#include <vector>
#include <QIcon>
#include "helpers/qt/strings.h"
#include "editor/core.h"

/*

   TODO:

    - When no properties, grey out entire pane and show "<no properties>" in first row's name column
 
*/

namespace {
   constexpr const bool show_raw_statuses = false;

   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

std::optional<vmad::script_status> DKPapyrusScriptObjectListModel::Script::getComputedStatus() const {
   auto& p_opt = this->statuses.parent;
   auto& t_opt = this->statuses.target;
   if constexpr (show_raw_statuses) {
      if (t_opt.has_value())
         return t_opt.value();
      if (p_opt.has_value())
         return p_opt.value();
   } else {
      if (p_opt.has_value()) {
         if (t_opt.has_value())
            return t_opt.value();
      
         switch (p_opt.value()) {
            case script_status::removed:
               return script_status::removed;
            default:
               return script_status::defined_on_base;
         }
      }
      if (t_opt.has_value()) {
         switch (t_opt.value()) {
            case script_status::removed:
               return script_status::removed;
            default:
               return script_status::defined_locally;
         }
      }
   }
   return {};
}
bool DKPapyrusScriptObjectListModel::Script::nameMatches(QString s) const {
   //
   // This is not a straightforward QString::toLower check because Bethesda's string table is 
   // only case-insensitive within the C locale.
   //
   size_t size = this->name.size();
   if (size != s.size())
      return false;
   for (size_t i = 0; i < size; ++i) {
      auto a = this->name[(uint)i].unicode(); // it's very annoying that QString was implemented in such a manner as to make this cast necessary.
      auto b = s[(uint)i].unicode();
      if (a == b)
         continue;
      if (a >= 'a' && a <= 'z')
         a -= 0x20;
      if (b >= 'a' && b <= 'z')
         b -= 0x20;
      if (a != b)
         return false;
   }
   return true;
}

DKPapyrusScriptObjectListModel::DKPapyrusScriptObjectListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &DKPapyrusScriptObjectListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &DKPapyrusScriptObjectListModel::clearTarget);
}

void DKPapyrusScriptObjectListModel::setTarget(dovah::form_stub& target_stub, vmad_data& target) {
   if (this->attached_to == &target_stub && this->vmads.target == &target && this->vmads.parent == nullptr)
      return;

   this->beginResetModel();

   this->attached_to  = &target_stub;
   this->vmads.target = &target;
   this->vmads.parent = nullptr;

   this->scripts.clear();
   //
   QMap<QString, Script> working;
   for (const auto& src : target.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& data = working[name_lower];
      if (data.name.isEmpty()) {
         data.name = name;
      }
      data.statuses.target = src.status;
      for (const auto& prop : src.properties) {
         if (prop.status != vmad::property_status::defined_only_on_base) {
            data.properties_set_on_target = true;
            break;
         }
      }
   }
   //
   this->scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->scripts.push_back(std::move(it->second));
   }

   this->endResetModel();
}
void DKPapyrusScriptObjectListModel::setTarget(dovah::form_stub& target_stub, vmad_data& target, vmad_data& parent) {
   if (this->attached_to == &target_stub && this->vmads.target == &target && this->vmads.parent == &parent)
      return;

   this->beginResetModel();

   this->attached_to  = &target_stub;
   this->vmads.target = &target;
   this->vmads.parent = &parent;

   this->scripts.clear();
   //
   QMap<QString, Script> working;
   for (const auto& src : parent.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& data = working[name_lower];
      if (data.name.isEmpty()) {
         data.name = name;
      }
      data.statuses.parent = src.status;
   }
   for (const auto& src : target.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& data = working[name_lower];
      if (data.name.isEmpty()) {
         data.name = name;
      }
      data.statuses.target = src.status;
      for (const auto& prop : src.properties) {
         if (prop.status != vmad::property_status::defined_only_on_base) {
            data.properties_set_on_target = true;
            break;
         }
      }
   }
   //
   this->scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->scripts.push_back(std::move(it->second));
   }

   this->endResetModel();
}

void DKPapyrusScriptObjectListModel::clearTarget() {
   this->beginResetModel();
   this->attached_to = nullptr;
   this->vmads = {};
   this->scripts.clear();
   this->endResetModel();
}
void DKPapyrusScriptObjectListModel::syncToTarget() {
   if (this->attached_to == nullptr)
      return;
   if (this->vmads.target == nullptr)
      return;

   auto loaded = this->attached_to->get_content_if_loaded();
   assert(loaded);

   this->vmads.target->clear_scripts(*loaded);
   for (const auto& item : this->scripts) {
      auto& target_status_opt = item.statuses.target;
      auto& parent_status_opt = item.statuses.parent;
      if (target_status_opt.has_value()) {
         auto& dst = this->vmads.target->scripts.emplace_back();
         dst.name   = item.name.toStdString();
         dst.status = target_status_opt.value();
      }
   }
}

#pragma region Editor core hooks
void DKPapyrusScriptObjectListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (stub == this->attached_to && !is_just_flagged) {
      this->clearTarget();
   }
}
#pragma endregion

#pragma region QAbstractItemModel overrides
QModelIndex DKPapyrusScriptObjectListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return {};
   if (row >= 0 && row < this->scripts.size())
      return this->createIndex(row, column, (void*)&this->scripts[row]);
   return {};
}
QModelIndex DKPapyrusScriptObjectListModel::parent(const QModelIndex& index) const {
   return {};
}
int DKPapyrusScriptObjectListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->scripts.size();
}
int DKPapyrusScriptObjectListModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags DKPapyrusScriptObjectListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return {};
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DKPapyrusScriptObjectListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return {};
   auto item   = (const Script*)index.internalPointer();
   auto column = index.column();
   if (column != 0)
      return {};
   switch (role) {
      case Qt::DisplayRole:
         return item->name;
      case Qt::ToolTipRole:
         {
            auto status_opt = item->getComputedStatus();
            if (!status_opt.has_value())
               return tr("Status: Property unmodified");
            switch (status_opt.value()) {
               case script_status::defined_locally:
                  if (item->properties_set_on_target) {
                     return tr("Status: Script added and edited locally");
                  }
                  return tr("Status: Script added locally");
               case script_status::overrides_base:
                  return tr("Status: Script inherited and edited locally");
               case script_status::defined_on_base:
                  return tr("Status: Script inherited from parent");
               case script_status::removed:
                  return tr("Status: Script inherited and deleted locally");
            }
         }
         break;
      case Qt::DecorationRole:
         {
            auto status_opt = item->getComputedStatus();
            if (!status_opt.has_value()) {
               return {}; // Call QTableView::setIconSize to ensure there's always space reserved for icons.
            }
            switch (status_opt.value()) {
               case script_status::defined_locally:
               default:
                  if (item->properties_set_on_target) {
                     return QIcon(":/icons/papyrus-status-icons/added-edited.png");
                  }
                  return QIcon(":/icons/papyrus-status-icons/added.png");
               case script_status::overrides_base:
               case script_status::defined_on_base:
                  if (item->properties_set_on_target) {
                     return QIcon(":/icons/papyrus-status-icons/inherited-edited.png");
                  }
                  return QIcon(":/icons/papyrus-status-icons/inherited.png");
               case script_status::removed:
                  return QIcon(":/icons/papyrus-status-icons/removed.png");
            }
         }
         break;
   }
   return {};
}
QVariant DKPapyrusScriptObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal) {
      return {};
   }
   if (section == 0) {
      if (role == Qt::DisplayRole)
         return tr("Script Name");
   }
   return {};
}
inline const DKPapyrusScriptObjectListModel::raw_script_info DKPapyrusScriptObjectListModel::row(int rowIndex) const noexcept {
   raw_script_info out;
   if (rowIndex < 0 || rowIndex >= this->scripts.size())
      return {};

   out.attached_to = this->attached_to;

   const auto& item = this->scripts[rowIndex];
   const auto  name = item.name.toStdString();
   if (this->vmads.parent)
      out.parent_script = this->vmads.parent->lookup_script(name);
   if (this->vmads.target)
      out.target_script = this->vmads.target->lookup_script(name);

   return out;
}
QModelIndex DKPapyrusScriptObjectListModel::index(QString scriptname) const {
   size_t size = this->scripts.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = this->scripts[i];
      if (item.nameMatches(scriptname))
         return this->index(i, 0, {});
   }
   return {};
}

QModelIndex DKPapyrusScriptObjectListModel::addScript(QString scriptname) {
   auto qmi = this->index(scriptname);
   if (qmi.isValid())
      return qmi;

   size_t insert_at;
   for (insert_at = 0; insert_at < this->scripts.size(); ++insert_at) {
      auto& item = this->scripts[insert_at];
      if (item.name < scriptname)
         break;
   }
   this->beginInsertRows({}, insert_at, insert_at);
   {
      Script item;
      item.name = scriptname;
      item.statuses.target = script_status::defined_locally;

      this->scripts.insert(insert_at, std::move(item));
   }
   this->endInsertRows();
   return this->index(insert_at, 0, {});
}
void DKPapyrusScriptObjectListModel::removeScript(int i) {
   if (i < 0 || i >= this->scripts.size())
      return;
   if (this->vmads.parent) {
      auto& item = this->scripts[i];
      if (item.statuses.parent.has_value()) {
         item.statuses.target = script_status::removed;
         item.properties_set_on_target = false;
         
         auto qmi = this->index(i, 0, {});
         emit dataChanged(qmi, qmi);
         return;
      }
   }
   this->beginRemoveRows({}, i, i);
   this->scripts.removeAt(i);
   this->endRemoveRows();
}
void DKPapyrusScriptObjectListModel::undeleteInheritedScript(int i) {
   if (i < 0 || i >= this->scripts.size())
      return;
   auto& item = this->scripts[i];
   if (!item.statuses.parent.has_value())
      return;
   auto& s_target = item.statuses.target;
   if (!s_target.has_value())
      return;
   if (s_target.value() != script_status::removed)
      return;

   s_target = script_status::defined_on_base;
   item.properties_set_on_target = false;

   auto qmi = this->index(i, 0, {});
   emit dataChanged(qmi, qmi);
}

void DKPapyrusScriptObjectListModel::refreshScript(QString scriptname) {
   auto qmi = this->index(scriptname);
   if (!qmi.isValid())
      return;

   auto& item = this->scripts[qmi.row()];
   item.properties_set_on_target = false;
   //
   for (const auto& src : this->vmads.parent->scripts) {
      QString name = QString::fromStdString(src.name);
      if (!item.nameMatches(name))
         continue;
      item.statuses.parent = src.status;
      break;
   }
   for (const auto& src : this->vmads.target->scripts) {
      QString name = QString::fromStdString(src.name);
      if (!item.nameMatches(name))
         continue;
      item.statuses.target = src.status;
      for (const auto& prop : src.properties) {
         if (prop.status != vmad::property_status::defined_only_on_base) {
            item.properties_set_on_target = true;
            break;
         }
      }
   }
   emit dataChanged(qmi, qmi);
}
#pragma endregion