#include "./DKPapyrusFragmentFunctionModel.h"
#include "dovah/data/papyrus/native_classes.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/pex/parsers/function_collector.h"
#include "editor/subsystems/assets.h"
#include "editor/subsystems/papyrus/core.h"
#include "editor/subsystems/papyrus/known_script.h"

namespace {
   // Papyrus stringnames are case-insensitive within the C locale.
   bool _papyrus_streq(const QString a, const QString b) {
      size_t size = a.size();
      if (size != b.size())
         return false;
      for (size_t i = 0; i < size; ++i) {
         auto ca = a[(uint)i].unicode();
         auto cb = b[(uint)i].unicode();
         if (ca == cb)
            continue;
         if (ca >= 'A' && ca <= 'Z')
            ca += 0x20;
         else if (cb >= 'A' && cb <= 'Z')
            cb += 0x20;
         if (ca != cb)
            return false;
      }
      return true;
   }

   int _papyrus_strcmp(const QString a, const QString b) {
      size_t end = (std::min)(a.size(), b.size());
      for (size_t i = 0; i < end; ++i) {
         int ca = a[(uint)i].unicode();
         int cb = b[(uint)i].unicode();
         if (ca >= 'A' && ca <= 'Z')
            ca += 0x20;
         if (cb >= 'A' && cb <= 'Z')
            cb += 0x20;
         if (ca != cb)
            return ca - cb;
      }
      if (end < a.size()) {
         return 1;
      }
      if (end < b.size()) {
         return -1;
      }
      return 0;
   }

   bool _exclude_function(std::string_view name) {
      if (dovah::papyrus::helpers::name_equals(name, "GetState")) // compiler-generated function for all scripts
         return true;
      if (dovah::papyrus::helpers::name_equals(name, "GotoState")) // compiler-generated function for all scripts
         return true;
      return false;
   }
}

bool DKPapyrusFragmentFunctionModel::Script::is_none_item() const {
   return this->name.isEmpty();
}

DKPapyrusFragmentFunctionModel::DKPapyrusFragmentFunctionModel(QObject* parent) : QAbstractItemModel(parent) {
   auto* none = new Script;
   this->_data.push_back(none);
}
DKPapyrusFragmentFunctionModel::~DKPapyrusFragmentFunctionModel() {
   this->_clear(false);
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex DKPapyrusFragmentFunctionModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         //
         // Handle top-level items (scripts):
         //
         if (!parent.isValid()) {
            if (column > 0 || row >= this->_data.size())
               return {};
            return this->createIndex(row, column, nullptr);
         }
         //
         // Check for the user item and get its functions:
         //
         if (parent == this->userItemQMI()) {
            auto& parent_item = this->_user_item;
            if (column > 0 || row >= parent_item.functions.size())
               return {};
            return this->createIndex(row, column, (void*)&parent_item);
         }
         //
         // Get a child function of a given script:
         //
         if (column > 0)
            return {};
         auto i = parent.row();
         if (i >= this->_data.size())
            return {};
         auto* script = this->_data[i];
         return this->createIndex(row, column, script);
      }
      /*virtual*/ QModelIndex DKPapyrusFragmentFunctionModel::parent(const QModelIndex& index) const /*override*/ {
         if (auto* item = index.internalPointer()) {
            if (index.model() != this)
               return {};
            if (item == &this->_user_item)
               return this->userItemQMI();
            for (size_t i = 0; i < this->_data.size(); ++i) {
               if (this->_data[i] == item)
                  return this->createIndex(i, 0, nullptr);
            }
            return {};
         }
         return {};
      }
      /*virtual*/ int DKPapyrusFragmentFunctionModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (!parent.isValid())
            return this->_data.size();
         if (parent == this->userItemQMI())
            return this->_user_item.functions.size();

         if (parent.internalPointer())
            //
            // QMI is a function; the internal pointer is its script.
            //
            return 0;
         //
         // QMI is top-level and therefore a script.
         //
         auto i = parent.row();
         if (i >= this->_data.size())
            return 0;
         return this->_data[i]->functions.size();
      }
      /*virtual*/ int DKPapyrusFragmentFunctionModel::columnCount(const QModelIndex& item) const /*override*/ {
         return 1;
      }
   #pragma endregion
   #pragma region Data
      /*virtual*/ Qt::ItemFlags DKPapyrusFragmentFunctionModel::flags(const QModelIndex& index) const /*override*/ {
         if (index.model() != this || !index.isValid())
            return {};
         if (index == this->userItemQMI())
            return Qt::ItemFlag::ItemIsEnabled;
         if (index.internalPointer()) { // function inside of a scriptname
            return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemNeverHasChildren;
         }
         // else script
         return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
      }
      /*virtual*/ QVariant DKPapyrusFragmentFunctionModel::data(const QModelIndex& index, int role) const /*override*/ {
         if (index.model() != this || !index.isValid())
            return {};
         if (index == this->userItemQMI()) {
            if (role == Qt::EditRole && index.row() == 0 && index.column() == 0) {
               return this->_user_item.name;
            }
            return {};
         }

         auto i = index.row();
         const auto* parent_script = (Script*)index.internalPointer();
         if (parent_script) {
            if (i >= parent_script->functions.size())
               return {};
            if (role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::EditRole) {
               return parent_script->functions[i];
            }
            return {};
         }

         const auto* item = this->_data[i];
         if (i >= this->_data.size())
            return {};
         if (role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::EditRole) {
            if (item->is_none_item()) {
               if (role == Qt::EditRole)
                  return {};
               return tr("<NONE>");
            }
            return item->name;
         }

         return {};
      }
   #pragma endregion
   /*virtual*/ QVariant DKPapyrusFragmentFunctionModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      return {};
   }
#pragma endregion

QModelIndex DKPapyrusFragmentFunctionModel::noneScriptQMI() const {
   assert(!this->_data.empty());
   assert(this->_data[0]->is_none_item());
   return this->index(0, 0, {});
}

QModelIndex DKPapyrusFragmentFunctionModel::userItemQMI() const {
   return this->createIndex(0, 0, (void*)this);
}
QString DKPapyrusFragmentFunctionModel::userItemScriptname() const {
   return this->_user_item.name;
}
void DKPapyrusFragmentFunctionModel::setUserItemScriptname(QString n) {
   if (_papyrus_streq(this->_user_item.name, n)) {
      return;
   }
   auto  qmi  = this->userItemQMI();
   auto& item = this->_user_item;
   item.name = n;
   if (!item.functions.empty()) {
      this->beginRemoveRows(qmi, 0, item.functions.size());
      item.functions.clear();
      this->endRemoveRows();
   }
   this->_update_script_functions({}, item);
   if (!item.functions.empty()) {
      this->beginInsertRows(qmi, 0, item.functions.size() - 1);
      this->endInsertRows();
   }
}

QModelIndex DKPapyrusFragmentFunctionModel::scriptQMI(QString name) const {
   if (_papyrus_streq(name, this->_user_item.name)) {
      return this->userItemQMI();
   }
   auto& list = this->_data;
   auto  it   = std::find_if(list.begin(), list.end(), [&name](const Script* item) {
      return _papyrus_streq(item->name, name);
   });
   if (it == list.end())
      return this->noneScriptQMI();
   return this->index(std::distance(list.begin(), it), 0, {});
}

void DKPapyrusFragmentFunctionModel::_clear(bool emit_signals) {
   if (emit_signals)
      this->beginResetModel();
   {
      auto& list = this->_data;
      for (auto* item : list)
         delete item;
      list.clear();
   }
   this->_user_item = {};
   if (emit_signals)
      this->endResetModel();
}

decltype(DKPapyrusFragmentFunctionModel::_data)::iterator DKPapyrusFragmentFunctionModel::_insertion_point_for(const Script& item) {
   if (item.name.isEmpty())
      return this->_data.begin();

   return std::upper_bound(
      this->_data.begin(),
      this->_data.end(),
      &item,
      [](const Script* a, const Script* b) -> bool {
         if (a->is_none_item())
            return true;
         if (b->is_none_item())
            return false;
         return _papyrus_strcmp(a->name, b->name) < 0;
      }
   );
}

void DKPapyrusFragmentFunctionModel::_add_scriptname(QString name, bool emit_signals) {
   if (name.isEmpty())
      return;
   if (this->_has_scriptname(name))
      return;

   auto* script = new Script;
   script->name = name;

   auto it = this->_insertion_point_for(*script);
   if (emit_signals) {
      auto i = std::distance(this->_data.begin(), it);
      this->beginInsertRows({}, i, i);
   }
   this->_data.insert(it, script);
   this->_update_script_functions({}, *script);
   if (emit_signals) {
      this->endInsertRows();
   }
}
bool DKPapyrusFragmentFunctionModel::_has_scriptname(QString name) const {
   if (name.isEmpty())
      return false;
   for (const auto* item : this->_data)
      if (_papyrus_streq(item->name, name))
         return true;
   return false;
}
void DKPapyrusFragmentFunctionModel::_remove_scriptname(QString name) {
   if (name.isEmpty())
      return;

   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->is_none_item())
         continue;
      if (_papyrus_streq(item->name, name)) {
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         delete item;
         --i;
         --size;
         this->endRemoveRows();
         return;
      }
   }
}

void DKPapyrusFragmentFunctionModel::_add_function(const QModelIndex& qmi, Script& item, QString name) {
   bool valid_qmi = qmi.isValid();

   auto& dst = item.functions;
   auto  it  = std::upper_bound(
      dst.begin(),
      dst.end(),
      name,
      _papyrus_strcmp
   );
   // it == first element that is greater than `name`
   if (it != dst.begin()) {
      auto pt = it - 1;
      if (_papyrus_streq(*pt, name)) { // function already in list?
         return;
      }
   }
   auto i = std::distance(dst.begin(), it);
   if (valid_qmi)
      this->beginInsertRows(qmi, i, i);
   dst.insert(it, name);
   if (valid_qmi)
      this->endInsertRows();
}
void DKPapyrusFragmentFunctionModel::_update_script_functions(const QModelIndex& qmi, Script& item) {
   bool valid_qmi = qmi.isValid();

   if (item.is_none_item()) {
      if (!item.functions.empty()) {
         if (valid_qmi)
            this->beginRemoveRows(qmi, 0, item.functions.size() - 1);
         item.functions.clear();
         if (valid_qmi)
            this->endRemoveRows();
      }
      return;
   }
   assert(!item.name.isEmpty());

   std::vector<QString> found_functions;
   {
      auto& assets = dovahkit::subsystems::assets::get();
      auto& ps     = dovahkit::subsystems::papyrus::core::get();

      auto _handle_scriptname = [&assets, &ps, &qmi, &item, &found_functions](this auto&& recurse, const std::string& name) -> void {
         //
         // First, ensure we don't import functions from native classes.
         //
         for (const auto& native : dovah::papyrus::native_classes)
            if (dovah::papyrus::helpers::name_equals(name, native.name))
               return;

         std::string superclass_name;
         {
            std::filesystem::path path = "scripts";
            path /= name;
            path.replace_extension("pex");
            auto file = std::unique_ptr<dovah::bsa_archived_file>(assets.lookup_game_asset(path));
            if (!file)
               return;

            using parser_type         = dovah::pex::parsers::function_collector;
            using parser_string_table = parser_type::retained_string_table_type; // TODO: may not even need this; investigate ditching it

            parser_string_table shared_strings;
            parser_type parser(shared_strings);
            parser.collect_arguments = false;
            parser.desired_classname = name;
            try {
               parser.read_file((const char*)file->data(), file->size());
               if (!parser.results.name.empty()) {
                  for (auto& func : parser.results.functions) {
                     if (func.is_global)
                        continue;
                     if (_exclude_function(func.name))
                        continue;

                     auto converted = QString::fromUtf8(func.name.data(), func.name.size());

                     // avoid duplicates (e.g. from overrides of superclass functions)
                     auto it = std::find_if(found_functions.begin(), found_functions.end(), [&converted](const QString item) {
                        return _papyrus_streq(item, converted);
                     });
                     if (it != found_functions.end())
                        continue;

                     found_functions.push_back(converted);
                  }
               }
            } catch (const dovah::compiled_papyrus_script::read_exception&) {
               return;
            }
         }
         if (superclass_name.empty()) {
            const auto* known = ps.lookup_known_script(name);
            if (known) {
               const auto* super = known->superclass();
               if (super)
                  recurse(super->name);
            }
         } else {
            recurse(superclass_name);
         }
      };
      _handle_scriptname(item.name.toStdString());

      std::stable_sort(found_functions.begin(), found_functions.end(), [](const QString a, const QString b) {
         return _papyrus_strcmp(a, b) < 0;
      });
   }
   if (!valid_qmi) {
      item.functions = found_functions;
      return;
   }
   {  // Handle any functions that have been removed.
      auto&  list = item.functions;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto seen = list[i];
         auto it   = std::find_if(found_functions.begin(), found_functions.end(), [&seen](const QString subject) {
            return _papyrus_streq(subject, seen);
         });
         if (it == found_functions.end()) {
            this->beginRemoveRows(qmi, i, i);
            list.erase(list.begin() + i);
            --size;
            this->endRemoveRows();
         }
      }
   }
   for (auto& name : found_functions)
      this->_add_function(qmi, item, name);
}

void DKPapyrusFragmentFunctionModel::_reset_from_source() {
   this->beginResetModel();
   {
      auto user_item_name = this->_user_item.name;
      this->_clear(false);
      this->_user_item.name = user_item_name;
   }
   {
      auto* none = new Script;
      this->_data.push_back(none);
   }
   if (this->_source_widget) {
      auto names = this->_source_widget->allNonDeletedScriptnames();
      for (const auto& name : names)
         this->_add_scriptname(name, false);

      auto& item = this->_user_item;
      if (!item.name.isEmpty()) {
         this->_update_script_functions({}, item);
      }
   }
   this->endResetModel();
}

void DKPapyrusFragmentFunctionModel::setSourceWidget(DKPapyrusBoundScriptListPane* widget) {
   auto* prior = this->_source_widget.data();
   if (widget == prior)
      return;
   if (prior) {
      QObject::disconnect(prior, nullptr, this, nullptr);
   }
   this->_source_widget = widget;
   this->_reset_from_source();
   if (widget) {
      QObject::connect(widget, &DKPapyrusBoundScriptListPane::scriptAdded, this, [this](QString v) { this->_add_scriptname(v, true); });
      QObject::connect(widget, &DKPapyrusBoundScriptListPane::scriptRemoved, this, &DKPapyrusFragmentFunctionModel::_remove_scriptname);
      QObject::connect(widget, &DKPapyrusBoundScriptListPane::scriptListReset, this, &DKPapyrusFragmentFunctionModel::_reset_from_source);
   }
}