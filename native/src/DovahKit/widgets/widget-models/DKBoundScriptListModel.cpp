#include "./DKBoundScriptListModel.h"
#include "./DKBoundScriptModel.h"
#include <QIcon>
#include "editor/core.h"

namespace vmad {
   using namespace dovah::loaded_forms::components::papyrus;
}

DKBoundScriptListModel::DKBoundScriptListModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &DKBoundScriptListModel::clear);
}
DKBoundScriptListModel::~DKBoundScriptListModel() {
   this->clear();
}

const DKBoundScriptListModel::bound_script* DKBoundScriptListModel::_script(const QModelIndex& qmi) const {
   if (!qmi.isValid() || qmi.model() != this)
      return nullptr;
   auto row = qmi.row();
   if (row >= this->_scripts.size())
      return nullptr;
   return this->_scripts[row];
}
DKBoundScriptListModel::bound_script* DKBoundScriptListModel::_script(const QModelIndex& qmi) {
   return const_cast<bound_script*>(std::as_const(*this)._script(qmi));
}

void DKBoundScriptListModel::clear() {
   for (auto* s : this->_scripts) {
      if (s->model)
         delete s->model;
      delete s;
   }
   this->_scripts.clear();
}
void DKBoundScriptListModel::initializeFrom(vmad_data& local) {
   this->beginResetModel();

   this->clear();
   this->_vmad.inherited = nullptr;
   this->_vmad.local     = &local;
   
   QMap<QString, bound_script*> working;
   for (const auto& src : local.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& ptr = working[name_lower];
      if (!ptr) {
         ptr = new bound_script;
         ptr->name = name;
      }
      switch (src.status) {
         case vmad::script_status::removed:
            ptr->status = src.status;
            break;
         default:
            ptr->status = vmad::script_status::defined_locally;
            break;
      }

      for (const auto& prop : src.properties) {
         if (prop.status == vmad::property_status::defined_locally) {
            ptr->has_local_properties = true;
            break;
         }
      }
   }
   
   this->_scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->_scripts.push_back(it->second);
   }

   this->endResetModel();
}
void DKBoundScriptListModel::initializeFrom(vmad_data& local, vmad_data& inherited) {
   this->beginResetModel();

   this->clear();
   this->_vmad.inherited = &inherited;
   this->_vmad.local     = &local;
   
   QMap<QString, bound_script*> working;
   for (const auto& src : inherited.scripts) {
      if (src.status == vmad::script_status::removed)
         continue;

      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& ptr = working[name_lower];
      if (!ptr) {
         ptr = new bound_script;
         ptr->name = name;
      }
      ptr->inherited = true;
      ptr->status    = vmad::script_status::defined_on_base;
   }
   for (const auto& src : local.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& ptr = working[name_lower];
      if (!ptr) {
         ptr = new bound_script;
         ptr->name = name;
      }
      switch (src.status) {
         case vmad::script_status::removed:
            ptr->status = src.status;
            break;
         default:
            ptr->status = vmad::script_status::defined_locally;
            if (ptr->inherited)
               ptr->status = vmad::script_status::overrides_base;
            break;
      }

      for (const auto& prop : src.properties) {
         if (prop.status == vmad::property_status::defined_locally) {
            ptr->has_local_properties = true;
            break;
         }
      }
   }
   
   this->_scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->_scripts.push_back(it->second);
   }

   this->endResetModel();
}
void DKBoundScriptListModel::commitTo(vmad_data& target, dovah::loaded_forms::Form& working_copy) {
   std::vector<std::string> scripts_to_remove;
   for (auto& script : target.scripts) {
      if (script.name.empty()) {
         //
         // Safety measure: clear nameless scripts in case they have form data (e.g. malformed 
         // garbage hex-edited or xEdited in by a user). This is because we use the absence of 
         // a name as a sentinel for scripts we're removing.
         //
         script.clear(working_copy);
         continue;
      }

      const bound_script* src = nullptr;
      for (const auto* item : this->_scripts) {
         if (dovah::papyrus::helpers::name_equals(item->name.toStdString(), script.name)) {
            src = item;
            break;
         }
      }
      if (!src) {
         script.clear(working_copy);
         scripts_to_remove.push_back(script.name);
         continue;
      }

      if (!src->model)
         continue;

      src->model->commitTo(script, working_copy);
   }
   for (const auto& name : scripts_to_remove) {
      std::erase_if(target.scripts, [&name](const auto& script) {
         return script.name == name;
      });
   }
   for (const auto* item : this->_scripts) {
      auto scriptname = item->name.toStdString();
      bool is_newly_added = true;
      for (const auto& script : target.scripts) {
         if (dovah::papyrus::helpers::name_equals(scriptname, script.name)) {
            is_newly_added = false;
            break;
         }
      }
      if (!is_newly_added)
         continue;

      auto& script = target.scripts.emplace_back();
      script.name = std::move(scriptname);
      if (!item->model)
         continue;
      item->model->commitTo(script, working_copy);
   }
   std::erase_if(target.scripts, [](auto& item) {
      return item.name.empty();
   });
}
      
#pragma region QAbstractItemModel overrides
   /*virtual*/ QModelIndex DKBoundScriptListModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
      if (!this->hasIndex(row, column, parent))
         return {};
      if (row < 0 || row >= this->_scripts.size())
         return {};
      return this->createIndex(row, column, (void*)this->_scripts[row]);
   }
   /*virtual*/ QModelIndex DKBoundScriptListModel::parent(const QModelIndex& index) const /*override*/ {
      return {};
   }
   /*virtual*/ int DKBoundScriptListModel::rowCount(const QModelIndex& parent) const /*override*/ {
      return this->_scripts.size();
   }
   /*virtual*/ int DKBoundScriptListModel::columnCount(const QModelIndex&) const /*override*/ {
      return 1;
   }
   /*virtual*/ Qt::ItemFlags DKBoundScriptListModel::flags(const QModelIndex& index) const /*override*/ {
      if (!index.isValid())
         return {};
      return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
   }
   /*virtual*/ QVariant DKBoundScriptListModel::data(const QModelIndex& qmi, int role) const /*override*/ {
      if (!qmi.isValid())
         return {};
      if (qmi.internalPointer() == nullptr)
         return {};
      auto* script = this->_script(qmi);
      if (!script)
         return {};
      
      switch (role) {
         case Qt::DisplayRole:
            return script->name;
         case Qt::ToolTipRole:
            switch (script->status) {
               case vmad::script_status::defined_locally:
                  if (script->has_local_properties) {
                     return tr("Status: Script added and edited locally");
                  }
                  return tr("Status: Script added locally");
               case vmad::script_status::overrides_base:
                  return tr("Status: Script inherited and edited locally");
               case vmad::script_status::defined_on_base:
                  return tr("Status: Script inherited from parent");
               case vmad::script_status::removed:
                  return tr("Status: Script inherited and deleted locally");
            }
            break;
         case Qt::DecorationRole:
            switch (script->status) {
               case vmad::script_status::defined_locally:
               default:
                  if (script->has_local_properties) {
                     return QIcon(":/icons/papyrus-status-icons/added-edited.png");
                  }
                  return QIcon(":/icons/papyrus-status-icons/added.png");
               case vmad::script_status::overrides_base:
               case vmad::script_status::defined_on_base:
                  if (script->has_local_properties) {
                     return QIcon(":/icons/papyrus-status-icons/inherited-edited.png");
                  }
                  return QIcon(":/icons/papyrus-status-icons/inherited.png");
               case vmad::script_status::removed:
                  return QIcon(":/icons/papyrus-status-icons/removed.png");
            }
            break;
      }

      return {};
   }
   /*virtual*/ QVariant DKBoundScriptListModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole)
         return {};
      if (section == 0)
         return tr("Script Name");
      return {};
   }
#pragma endregion

QModelIndex DKBoundScriptListModel::index(QString scriptname) const {
   for (size_t i = 0; i < this->_scripts.size(); ++i)
      if (this->_scripts[i]->name.compare(scriptname, Qt::CaseInsensitive) == 0)
         return this->index(i, 0, {});
   return {};
}

std::optional<vmad::script_status> DKBoundScriptListModel::status(const QModelIndex& qmi) const {
   if (auto* script = this->_script(qmi))
      return script->status;
   return {};
}

// Creates a script model for the given script. If the given script already has a model, 
// creates a copy. You can use this to open a dialog to edit the given script's properties: 
// create the model; if the user clicks OK, use `replaceScriptModelFor` to save it; else, 
// delete it yourself.
DKBoundScriptModel* DKBoundScriptListModel::createScriptModelFor(const QModelIndex& script_qmi) {
   auto* script = this->_script(script_qmi);
   if (!script)
      return nullptr;

   DKBoundScriptModel* out = nullptr;
   if (script->model) {
      out = new DKBoundScriptModel(*script->model, this);
   } else {
      out = new DKBoundScriptModel(script->name, this);

      auto scriptname = script->name.toUtf8().toStdString();

      vmad::attached_script* local     = nullptr;
      vmad::attached_script* inherited = nullptr;
      if (this->_vmad.local) {
         local = this->_vmad.local->lookup_script(scriptname);
      }
      if (this->_vmad.inherited) {
         inherited = this->_vmad.inherited->lookup_script(scriptname);
      }
      if (local) {
         if (this->_vmad.inherited)
            out->initializeFrom(*local, *this->_vmad.inherited);
         else
            out->initializeFrom(*local);
      } else if (inherited) {
         out->initializeFromInheritedOnly(*inherited);
      }
   }
   return out;
}

// Takes ownership of the model.
bool DKBoundScriptListModel::replaceScriptModelFor(const QModelIndex& qmi, DKBoundScriptModel* model) {
   auto* script = this->_script(qmi);
   if (!script)
      return false;

   if (script->model) {
      if (script->model == model)
         return true;
      delete script->model;
   }
   script->model = model;
   if (model) {
      script->has_local_properties = model->anyPropertiesEditedLocally();
   } else {
      script->has_local_properties = false;
   }
   return true;
}

QModelIndex DKBoundScriptListModel::addScript(QString name) {
   if (this->_scripts.size() >= vmad::attachment_data::max_script_count)
      return {};

   for (size_t i = 0; i < this->_scripts.size(); ++i) {
      auto* script = this->_scripts[i];
      if (script->name.compare(name, Qt::CaseInsensitive) == 0)
         return this->index(i, 0, {});
   }

   auto it = std::upper_bound(
      this->_scripts.begin(),
      this->_scripts.end(),
      name,
      [](QString name, const bound_script* item) {
         return item->name.compare(name, Qt::CaseInsensitive) < 0;
      }
   );
   size_t i = std::distance(this->_scripts.begin(), it);

   this->beginInsertRows({}, i, i);
   auto* script = new bound_script;
   this->_scripts.insert(it, script);
   script->name   = name;
   script->status = vmad::script_status::defined_locally;
   this->endInsertRows();
   return this->index(i, 0, {});
}

bool DKBoundScriptListModel::removeScript(const QModelIndex& qmi) {
   auto* script = this->_script(qmi);
   if (!script)
      return false;
   if (script->inherited) {
      if (script->model) {
         delete script->model;
         script->model = nullptr;
      }
      script->status = vmad::script_status::removed;
      script->has_local_properties = false;
      emit dataChanged(qmi, qmi);
      return true;
   } else {
      auto i = qmi.row();
      this->beginRemoveRows({}, i, i);
      this->_scripts.erase(this->_scripts.begin() + i);
      delete script;
      this->endRemoveRows();
      return true;
   }
}

bool DKBoundScriptListModel::undeleteScript(const QModelIndex& qmi) {
   auto* script = this->_script(qmi);
   if (!script)
      return false;
   if (script->status != vmad::script_status::removed)
      return false;

   script->status = vmad::script_status::defined_locally;
   if (script->inherited)
      script->status = vmad::script_status::defined_on_base;
   emit dataChanged(qmi, qmi);
   return true;
}

std::vector<std::string> DKBoundScriptListModel::getAllBoundScriptNames(bool include_inherited_and_removed) const {
   std::vector<std::string> names;
   for (const auto* script : this->_scripts) {
      if (!include_inherited_and_removed) {
         if (script->inherited && script->status == vmad_script_status::removed)
            continue;
      }
      names.push_back(script->name.toUtf8().toStdString());
   }
   return names;
}
QString DKBoundScriptListModel::boundScriptName(const QModelIndex& qmi) const {
   if (!qmi.isValid())
      return {};
   return this->boundScriptName(qmi.row());
}
QString DKBoundScriptListModel::boundScriptName(size_t row) const {
   if (row >= this->_scripts.size())
      return {};
   return this->_scripts[row]->name;
}