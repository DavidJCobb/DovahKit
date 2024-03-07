#include "./DKAllPapyrusScriptsModel.h"
#include <memory>
#include "dovah/data/papyrus/helpers/name_equals.h"
#include "editor/subsystems/papyrus/core.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/core.h"

namespace {
   // If `false`, sorting will be done by the QSortFilterProxyModels that wrap this model.
   constexpr const bool do_sorting_here = false;
}

namespace {
   using form_info_cache_subsystem = dovahkit::subsystems::form_info_cache::core;
   using papyrus_subsystem         = dovahkit::subsystems::papyrus::core;
}

DKAllPapyrusScriptsModel::DKAllPapyrusScriptsModel() {
   auto& editor  = DovahKitCore::get();
   auto& papyrus = papyrus_subsystem::get();
   
   QObject::connect(&papyrus, &papyrus_subsystem::initialKnownScriptDiscoveryComplete, this, [this]() {
      this->_gatherScriptnamesOnLoadingDone();
   });
   QObject::connect(&papyrus, &papyrus_subsystem::knownScriptDiscovered, this, [this](const known_script& subject) {
      auto item = std::make_unique<Script>();
      item->name = QString::fromStdString(subject.name);
      {
         std::string_view docstring;
         if (subject.info.loose.has_value()) {
            docstring = subject.info.loose.value().docstring;
         } else if (subject.info.packed.has_value()) {
            docstring = subject.info.packed.value().docstring;
         }
         if (!docstring.empty()) {
            item->docstring = QString::fromUtf8(docstring.data(), docstring.size());
         }
      }
      item->info = &subject;
      //
      auto& list = this->_scripts;
      if constexpr (do_sorting_here) {
         auto it = std::upper_bound(
            list.begin(),
            list.end(),
            item.get(),
            [](const auto& a, const auto& b) {
               return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
            }
         );
         auto n = std::distance(list.begin(), it);

         this->beginInsertRows({}, n, n);
         list.insert(it, item.get());
         this->endInsertRows();
      } else {
         auto end = list.size();
         this->beginInsertRows({}, end, end);
         list.push_back(item.get());
         this->endInsertRows();
      }
      //
      item.release();
   });
   QObject::connect(&papyrus, &papyrus_subsystem::knownScriptAboutToBeForgotten, this, [this](const known_script& subject) {
      auto& list = this->_scripts;
      for (size_t i = 0; i < list.size(); ++i) {
         auto* info = list[i];
         if (info->info == &subject) {
            this->beginRemoveRows({}, i, i);
            list.erase(list.begin() + i);
            delete info;
            this->endRemoveRows();
            return;
         }
      }
   });
   QObject::connect(&papyrus, &papyrus_subsystem::knownScriptChanged, this, [this](const known_script& subject) {
      auto& list = this->_scripts;
      for (size_t i = 0; i < list.size(); ++i) {
         if (list[i]->info == &subject) {
            list[i]->name = QString::fromStdString(subject.name); // in case the letter case changed

            auto qmi = this->index(i, 0, {});
            emit this->dataChanged(qmi, qmi);
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_clear();
   });

   if (papyrus.is_initial_script_discovery_complete()) {
      this->_gatherScriptnamesOnLoadingDone();
   }
}

void DKAllPapyrusScriptsModel::_clear(bool silent) {
   if (!silent)
      this->beginResetModel();

   for (auto* item : this->_scripts)
      delete item;
   this->_scripts.clear();

   if (!silent)
      this->endResetModel();
}
void DKAllPapyrusScriptsModel::_gatherScriptnamesOnLoadingDone() {
   this->beginResetModel();

   auto& papyrus = papyrus_subsystem::get();
   papyrus.for_each_known_script([this](const known_script& script) {
      auto item = std::make_unique<Script>();
      item->name = QString::fromStdString(script.name);
      {
         std::string_view docstring;
         if (script.info.loose.has_value()) {
            docstring = script.info.loose.value().docstring;
         } else if (script.info.packed.has_value()) {
            docstring = script.info.packed.value().docstring;
         }
         if (!docstring.empty()) {
            item->docstring = QString::fromUtf8(docstring.data(), docstring.size());
         }
      }
      item->info = &script;
      this->_scripts.push_back(item.get());
      item.release();
   });
   if constexpr (do_sorting_here) {
      std::sort(this->_scripts.begin(), this->_scripts.end(), [](const auto& a, const auto& b) {
         return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
      });
   }

   this->endResetModel();
}

//

bool DKAllPapyrusScriptsModel::scriptIsAttachableTo(const QModelIndex& qmi, dovah::form_type ft) const {
   if (!qmi.isValid())
      return false;
   return this->_scripts[qmi.row()]->info->is_attachable_to(ft);
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex DKAllPapyrusScriptsModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex DKAllPapyrusScriptsModel::parent(const QModelIndex& index) const {
         return {};
      }
      /*virtual*/ QModelIndex DKAllPapyrusScriptsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         return this->index(row, column, {});
      }
      /*virtual*/ int DKAllPapyrusScriptsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         return this->_scripts.size();
      }
      /*virtual*/ int DKAllPapyrusScriptsModel::columnCount(const QModelIndex& item) const /*override*/ {
         return 1;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant DKAllPapyrusScriptsModel::data(const QModelIndex& index, int role) const /*override*/ {
         auto row = index.row();
         if (row < 0)
            return {};

         switch (role) {
            case Qt::DisplayRole:
               return this->_scripts[row]->name;
            case Qt::ToolTipRole:
               {
                  auto text = this->_scripts[row]->docstring;
                  if (!text.isEmpty())
                     return text;
               }
               break;
         }

         return {};
      }
      /*virtual*/ Qt::ItemFlags DKAllPapyrusScriptsModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         Qt::ItemFlags flags = {};
         flags |= Qt::ItemFlag::ItemIsEnabled;
         flags |= Qt::ItemFlag::ItemIsSelectable;
         return flags;
      }
   #pragma endregion
#pragma endregion