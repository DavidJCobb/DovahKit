#include "./QuestAliasesModel.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/forms/Quest.h"
#include "dovah/utils/story_event_member_id.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"

namespace {
   using alias_flag = dovah::loaded_forms::Alias::flag;
   using alias_type = dovah::loaded_forms::Alias::alias_type;
}

#pragma region cached_alias_data
   void QuestAliasesModel::cached_alias_data::recache(const dovah::loaded_forms::Alias& alias) {
      //
      // Fill parameters:
      //
      {
         using namespace dovah::loaded_forms::structs::alias_fill_params;
         auto& dst = this->fill;
         if (alias.type == alias_type::location) {
            auto& loc_alias = (const dovah::loaded_forms::LocationAlias&)alias;
            auto& fill      = loc_alias.fill_params;
            if (auto* casted = std::get_if<loc::at_reference_alias>(&fill)) {
               QString ref = tr("NONE");
               QString kwd = tr("NONE");
               if (auto* a = alias.owner.lookup_alias_by_id(casted->alias))
                  ref = QString::fromStdString(a->name);
               if (auto* stub = casted->keyword.get_form_stub())
                  kwd = QString::fromStdString(stub->editorID);
               dst = tr("%1's %2 Loc").arg(ref).arg(kwd);
            } else if (auto* casted = std::get_if<copy_external_alias>(&fill)) {
               QString quest_name = tr("NONE");
               QString alias_name = tr("NONE");
               auto* quest = casted->quest.get_form_stub();
               if (quest && quest->form_type == dovah::form_type::quest) {
                  quest_name = QString::fromStdString(quest->editorID);
                  auto loaded = quest->load().ptr_cast<dovah::loaded_forms::Quest>();
                  if (loaded) {
                     auto* other = loaded->lookup_alias_by_id(casted->alias);
                     if (other)
                        alias_name = QString::fromStdString(other->name);
                  } else {
                     alias_name = tr("???");
                  }
               }
               dst = tr("Match %1 in %2").arg(alias_name).arg(quest_name);
            } else if (auto* casted = std::get_if<loc::find>(&fill)) {
               dst = tr("Conditions", "fill type summary for Find loc aliases");
               if (casted->from_event.has_value()) {
                  auto& ev = casted->from_event.value();

                  QString member_name = tr("NONE");
                  if (ev.member != 0) {
                     member_name = dovah::story_event_member_id(ev.member).to_string().data();
                  }
                  {
                     auto code = ev.code;
                     if (!code)
                        code = alias.owner.event;

                     const auto* event_dfn = dovah::story_event_definition::lookup(code);
                     if (event_dfn) {
                        const auto* member_dfn = event_dfn->member_by_signature(ev.member);
                        if (member_dfn) {
                           member_name = member_dfn->name;
                        }
                     }
                  }
                  dst = tr("Event Data: %1").arg(member_name);
               }
            } else if (auto* casted = std::get_if<loc::preassigned>(&fill)) {
               QString loc = tr("NONE");
               if (auto* stub = casted->location.get_form_stub())
                  loc = QString::fromStdString(stub->editorID);
               dst = tr("Preassigned: %1").arg(loc);
            }
         } else if (alias.type == alias_type::reference) {
            auto& ref_alias = (const dovah::loaded_forms::ReferenceAlias&)alias;
            auto& fill      = ref_alias.fill_params;
            if (auto* casted = std::get_if<ref::at_location_alias>(&fill)) {
               QString loc = tr("NONE");
               QString lrt = tr("NONE");
               if (auto* a = alias.owner.lookup_alias_by_id(casted->alias))
                  loc = QString::fromStdString(a->name);
               if (auto* stub = casted->loc_ref_type.get_form_stub())
                  lrt = QString::fromStdString(stub->editorID);
               dst = tr("%1's RefType: %2").arg(loc).arg(lrt);
            } else if (auto* casted = std::get_if<copy_external_alias>(&fill)) {
               QString quest_name = tr("NONE");
               QString alias_name = tr("NONE");
               auto* quest = casted->quest.get_form_stub();
               if (quest&& quest->form_type == dovah::form_type::quest) {
                  quest_name = QString::fromStdString(quest->editorID);
                  auto loaded = quest->load().ptr_cast<dovah::loaded_forms::Quest>();
                  if (loaded) {
                     auto* other = loaded->lookup_alias_by_id(casted->alias);
                     if (other)
                        alias_name = QString::fromStdString(other->name);
                  } else {
                     alias_name = tr("???");
                  }
               }
               dst = tr("Match %1 in %2").arg(alias_name).arg(quest_name);
            } else if (auto* casted = std::get_if<ref::create>(&fill)) {
               QString base  = tr("NONE");
               QString basis = tr("NONE");
               if (auto* stub = casted->base_form.get_form_stub())
                  base = QString::fromStdString(stub->editorID);
               if (auto* create_at = alias.owner.lookup_alias_by_id(casted->at_reference.alias))
                  basis = QString::fromStdString(create_at->name);
               dst = tr("Create %1 at %2")
                  .arg(base)
                  .arg(basis);
            } else if (auto* casted = std::get_if<ref::find_from_event>(&fill)) {
               QString member_name = tr("NONE");
               if (casted->member != 0) {
                  member_name = dovah::story_event_member_id(casted->member).to_string().data();
               }
               {
                  auto code = casted->code;
                  if (!code)
                     code = alias.owner.event;

                  const auto* event_dfn = dovah::story_event_definition::lookup(code);
                  if (event_dfn) {
                     const auto* member_dfn = event_dfn->member_by_signature(casted->member);
                     if (member_dfn) {
                        member_name = member_dfn->name;
                     }
                  }
               }
               dst = tr("Event Data: %1").arg(member_name);
            } else if (auto* casted = std::get_if<ref::find_in_loaded_area>(&fill)) {
               dst = tr("Conditions", "fill type summary for Find In Loaded Area ref aliases");
            } else if (auto* casted = std::get_if<ref::find_near_alias>(&fill)) {
               QString basis = tr("NONE");
               if (auto* other = alias.owner.lookup_alias_by_id(casted->alias))
                  basis = QString::fromStdString(other->name);
               dst = tr("Linked Child Of: %1").arg(basis);
            } else if (auto* casted = std::get_if<ref::preassigned>(&fill)) {
               QString ref = tr("NONE");
               if (auto* stub = casted->ref.get_form_stub()) {
                  ref = QString::fromStdString(stub->editorID);
                  if (ref.isEmpty()) {
                     auto* base = dovah::form_stub_helpers::get_base_form(*stub);
                     if (base)
                        ref = QString::fromStdString(base->editorID);
                  }
                  ref = tr("%1 (%2)").arg(ref).arg(editor_helpers::form_id_to_string(stub->formID));
               }
               dst = tr("Preassigned: %1").arg(ref);
            } else if (auto* casted = std::get_if<ref::unique_actor>(&fill)) {
               QString edid = tr("NONE");
               if (auto* stub = casted->actor_base.get_form_stub())
                  edid = QString::fromStdString(stub->editorID);
               dst = tr("Unique Actor: %1").arg(edid);
            }
         }
      }
      //
      // Other parameters:
      //
      if (alias.type == alias_type::location) {
         this->factions  = tr("N/A");
         this->inventory = tr("N/A");
         this->keywords  = tr("N/A");
         this->packages  = tr("N/A");
         this->papyrus   = tr("N/A");
         this->spells    = tr("N/A");
      } else if (alias.type == alias_type::reference) {
         auto& ref_alias = (const dovah::loaded_forms::ReferenceAlias&)alias;

         auto _editor_id_list = [](auto& list) -> QString {
            QString string;
            size_t  size = list.size();
            for (size_t i = 0; i < size; ++i) {
               auto* stub = list[i].get_form_stub();
               if (!stub)
                  continue;
               string += QString::fromStdString(stub->editorID);
               if (i + 1 < size)
                  string += tr(", ");
            }
            return string;
         };

         this->factions = _editor_id_list(ref_alias.factions);
         {
            auto&  string = this->inventory;
            auto&  list   = ref_alias.inventory.entries;
            size_t size   = list.size();
            string.clear();
            for (size_t i = 0; i < size; ++i) {
               auto* stub = list[i].item.get_form_stub();
               if (!stub)
                  continue;
               string += tr("%1x %2").arg(list[i].count).arg(QString::fromStdString(stub->editorID));
               if (i + 1 < size)
                  string += tr(", ");
            }
         }
         this->keywords = _editor_id_list(ref_alias.keywords.forms);
         this->packages = _editor_id_list(ref_alias.packages);
         {
            auto&  string = this->papyrus;
            auto&  list   = ref_alias.script_data.scripts;
            size_t size   = list.size();
            string.clear();
            for (size_t i = 0; i < size; ++i) {
               string += QString::fromStdString(list[i].name);
               if (i + 1 < size)
                  string += tr(", ");
            }
         }
         this->spells = _editor_id_list(ref_alias.spells);
      }
   }
#pragma endregion

QuestAliasesModel::QuestAliasesModel(dovah::loaded_forms::Quest& q, QObject* parent) : QAbstractItemModel(parent), _quest(q) {
   for (auto* alias : this->_quest.aliases) {
      auto& cache = this->_cache[alias->id];
      cache.recache(*alias);
   }

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, &QuestAliasesModel::_recacheAllAliases);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &QuestAliasesModel::_recacheAllAliases);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &QuestAliasesModel::_recacheAllAliases);
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex QuestAliasesModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (parent.isValid())
            return {};
         if (row < 0 || row >= this->_quest.aliases.size())
            return {};
         return this->createIndex(row, column, nullptr);
      }
      /*virtual*/ QModelIndex QuestAliasesModel::parent(const QModelIndex&) const /*override*/ {
         return {};
      }
      /*virtual*/ QModelIndex QuestAliasesModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         return this->index(row, column, {});
      }
      /*virtual*/ int QuestAliasesModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.isValid())
            return 0;
         return this->_quest.aliases.size();
      }
      /*virtual*/ int QuestAliasesModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant QuestAliasesModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (!index.isValid())
            return {};
         auto i = index.row();
         if (i >= this->_quest.aliases.size())
            return {};
         const auto& alias = *this->_quest.aliases[i];
         const auto* cache = (cached_alias_data*)nullptr;
         {
            auto it = this->_cache.find(alias.id);
            if (it != this->_cache.end())
               cache = &it->second;
         }
         if (role == Qt::EditRole) {
            switch (index.column()) {
               case Column::Name:
                  return QString::fromStdString(alias.name);
               case Column::ID:
                  return alias.id;
               case Column::Optional:
                  return (bool)(alias.flags & alias_flag::optional);
               case Column::Type:
                  return (int)alias.type;
               case Column::Flags:
                  return alias.flags;
            }
            return {};
         }
         if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            switch (index.column()) {
               case Column::Name:
                  return QString::fromStdString(alias.name);
               case Column::ID:
                  return alias.id;
               case Column::Optional:
                  if (alias.flags & alias_flag::optional)
                     return tr("Y");
                  return tr("N");
               case Column::Type:
                  if (alias.type == alias_type::reference) {
                     return tr("Ref");
                  } else if (alias.type == alias_type::location) {
                     return tr("Loc");
                  }
                  break;
               case Column::Fill:
                  if (cache)
                     return cache->fill;
                  break;
               case Column::Flags:
                  {
                     QString out;
                     if (alias.flags & alias_flag::reserves_target)
                        out += tr("R", "flag: Reserves target");
                     if (alias.flags & alias_flag::optional)
                        out += tr("O", "flag: Optional");
                     if (alias.flags & alias_flag::make_essential)
                        out += tr("E", "flag: Essential");
                     if (alias.flags & alias_flag::stores_text)
                        out += tr("D", "flag: stores text");
                     if (alias.flags & alias_flag::quest_object)
                        out += tr("Q", "flag: Quest object");
                     if (alias.flags & alias_flag::uses_stored_text)
                        out += tr("U", "flag: Uses stored text");
                     if (alias.flags & alias_flag::make_protected)
                        out += tr("P", "flag: Protected");
                     if (alias.type == alias_type::reference) {
                        auto& ref_alias = (const dovah::loaded_forms::ReferenceAlias&)alias;
                        if (ref_alias.additional_voicetype.get_form_stub())
                           out += tr("F", "flag: voicetypes For export");
                     }
                     return out;
                  }
                  break;
               case Column::Allow:
                  {
                     QString out;

                     auto _append_flag = [&alias, &out]<auto Flag>(QString text) {
                        if (!(alias.flags & Flag))
                           return;
                        if (!out.isEmpty())
                           out += tr(", ");
                        out += text;
                     };
                     
                     _append_flag.operator()<alias_flag::allow_cleared>(tr("Cleared"));
                     _append_flag.operator()<alias_flag::allow_dead>(tr("Dead"));
                     _append_flag.operator()<alias_flag::allow_destroyed>(tr("Destroyed"));
                     _append_flag.operator()<alias_flag::allow_disabled>(tr("Disabled"));
                     _append_flag.operator()<alias_flag::allow_reserved>(tr("Reserved"));
                     _append_flag.operator()<alias_flag::allow_reuse_in_quest>(tr("Reuse"));

                     return out;
                  }
                  break;
               case Column::Papyrus:
                  if (cache)
                     return cache->papyrus;
                  break;
               case Column::Packages:
                  if (cache)
                     return cache->packages;
                  break;
               case Column::Inventory:
                  if (cache)
                     return cache->inventory;
                  break;
               case Column::Factions:
                  if (cache)
                     return cache->factions;
                  break;
               case Column::Spells:
                  if (cache)
                     return cache->spells;
                  break;
               case Column::Keywords:
                  if (cache)
                     return cache->keywords;
                  break;
            }
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags QuestAliasesModel::flags(const QModelIndex& index) const /*override*/ {
         if (!index.isValid()) {
            return {};
         }
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant QuestAliasesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      switch (section) {
         case Column::Name:
            return tr("Name");
         case Column::ID:
            return tr("ID");
         case Column::Optional:
            return tr("Optional");
         case Column::Type:
            return tr("Type");
         case Column::Fill:
            return tr("Fill");
         case Column::Flags:
            return tr("Flags");
         case Column::Allow:
            return tr("Allow");
         case Column::Papyrus:
            return tr("Papyrus");
         case Column::Packages:
            return tr("Packages");
         case Column::Inventory:
            return tr("Inventory");
         case Column::Factions:
            return tr("Factions");
         case Column::Spells:
            return tr("Spells");
         case Column::Keywords:
            return tr("Keywords");
      }
      return {};
   }
#pragma endregion

const dovah::loaded_forms::Alias* QuestAliasesModel::alias(const QModelIndex& qmi) const {
   if (qmi.model() != this)
      return nullptr;
   if (!qmi.isValid())
      return nullptr;
   if (qmi.row() >= this->_quest.aliases.size())
      return nullptr;
   return this->_quest.aliases[qmi.row()];
}
dovah::loaded_forms::Alias* QuestAliasesModel::alias(const QModelIndex& qmi) {
   return const_cast<dovah::loaded_forms::Alias*>(std::as_const(*this).alias(qmi));
}
const dovah::loaded_forms::Alias* QuestAliasesModel::aliasByID(int32_t id) const {
   return this->_quest.lookup_alias_by_id(id);
}
dovah::loaded_forms::Alias* QuestAliasesModel::aliasByID(int32_t id) {
   return const_cast<dovah::loaded_forms::Alias*>(std::as_const(*this).aliasByID(id));
}

QModelIndex QuestAliasesModel::createRefAlias() {
   auto id_opt = _next_alias_id();
   if (!id_opt.has_value())
      return {};

   auto i = this->_quest.aliases.size();
   this->beginInsertRows({}, i, i);
   auto& alias_ptr = this->_quest.aliases.emplace_back();
   alias_ptr = new dovah::loaded_forms::ReferenceAlias(this->_quest);
   alias_ptr->id = id_opt.value();
   this->_cache[alias_ptr->id].recache(*alias_ptr);
   this->endInsertRows();

   return this->index(i, 0, {});
}
QModelIndex QuestAliasesModel::createLocAlias() {
   auto id_opt = _next_alias_id();
   if (!id_opt.has_value())
      return {};

   auto i = this->_quest.aliases.size();
   this->beginInsertRows({}, i, i);
   auto& alias_ptr = this->_quest.aliases.emplace_back();
   alias_ptr = new dovah::loaded_forms::LocationAlias(this->_quest);
   alias_ptr->id = id_opt.value();
   this->_cache[alias_ptr->id].recache(*alias_ptr);
   this->endInsertRows();

   return this->index(i, 0, {});
}

void QuestAliasesModel::deleteAlias(const QModelIndex& qmi) {
   if (qmi.model() != this)
      return;
   if (!qmi.isValid())
      return;
   if (qmi.row() >= this->_quest.aliases.size())
      return;

   this->beginRemoveRows({}, qmi.row(), qmi.row());
   if (auto* alias = this->_quest.aliases[qmi.row()]) {
      this->_cache.erase(alias->id);
   }
   this->_quest.aliases[qmi.row()]->clear(this->_quest);
   this->_quest.aliases.erase(this->_quest.aliases.begin() + qmi.row());
   this->endRemoveRows();
}

void QuestAliasesModel::onAliasChanged(const QModelIndex& qmi) {
   if (qmi.model() != this)
      return;
   if (!qmi.isValid())
      return;
   if (qmi.row() >= this->_quest.aliases.size())
      return;

   auto* alias = this->_quest.aliases[qmi.row()];
   assert(alias != nullptr);
   auto& cache = this->_cache[alias->id];
   cache.recache(*alias);

   QModelIndex tl = qmi.siblingAtColumn(0);
   QModelIndex br = qmi.siblingAtColumn(ColumnCount - 1);
   emit dataChanged(tl, br);
}

std::optional<uint32_t> QuestAliasesModel::_next_alias_id() {
   if (this->_quest.aliases.size() == std::numeric_limits<uint32_t>::max() - 1)
      return {};
   auto base_id = this->_quest.next_alias_id;
   auto id      = this->_quest.next_alias_id;
   while (auto* alias = this->_quest.lookup_alias_by_id(id)) {
      ++id;
      if (id == -1)
         continue;
      assert(id != base_id);
   }
   return id;
}

void QuestAliasesModel::_recacheAllAliases() {
   for (size_t i = 0; i < this->_quest.aliases.size(); ++i) {
      auto* alias = this->_quest.aliases[i];
      assert(!!alias);
      this->_cache[alias->id].recache(*alias);

      QModelIndex tl = this->index(i, 0, {});
      QModelIndex br = this->index(i, ColumnCount - 1, {});
      emit dataChanged(tl, br);
   }
}