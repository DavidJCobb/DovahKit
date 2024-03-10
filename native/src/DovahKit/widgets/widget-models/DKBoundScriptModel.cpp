#include "./DKBoundScriptModel.h"
#include <cassert>
#include <type_traits>
#include <QIcon>
#include "helpers/type_traits/is_std_vector.h"

#include "dovah/data/papyrus/native_classes.h"
#include "editor/subsystems/papyrus/core.h"
#include "editor/core.h"

// For loading PEXs:
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/papyrus/compiled_script.h"
#include "editor/subsystems/assets.h"

namespace vmad {
   using namespace dovah::loaded_forms::components::papyrus;
}

namespace {
   // Qt's treeviews can display icons in cells, but if only some of a column's cells have 
   // an icon, then the others don't reserve space for an icon. This happens even if you 
   // call QTreeView::setIconSize, and even if you do that AND return a default-contructed 
   // QIcon as data for the no-icon cells.
   static QIcon& _blank_icon() {
      static QIcon icon([]() {
         QPixmap pixmap(16, 16); // also, QTreeView::setIconSize doesn't enlarge icons to match the desired size. what *does* it do?
         pixmap.fill(QColorConstants::Transparent);
         return pixmap;
      }());
      return icon;
   }
}

DKBoundScriptModel::DKBoundScriptModel(QString scriptname, QObject* parent) : QAbstractItemModel(parent), _scriptname(scriptname) {
   //
   // Load property definitions from the PEX:
   //
   {
      if (scriptname.isEmpty())
         return;

      auto& papyrus    = dovahkit::subsystems::papyrus::core::get();
      auto* definition = papyrus.lookup_known_script(scriptname);
      if (!definition) {
         this->_load_results.failed = true;
         return;
      }

      do {
         this->_load_property_definitions_from(definition->name);
         if (this->_load_results.failed)
            break;
      } while (definition = definition->superclass());
   }
   if (this->_load_results.failed)
      return;

   std::sort(
      this->_properties.begin(),
      this->_properties.end(),
      [](const auto* a, const auto* b) {
         return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
      }
   );
   
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &DKBoundScriptModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formModified,           this, &DKBoundScriptModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,         this, &DKBoundScriptModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &DKBoundScriptModel::_clear);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &DKBoundScriptModel::formsRenumberedEnMasse);

   auto& papyrus = dovahkit::subsystems::papyrus::core::get();
   QObject::connect(&papyrus, &dovahkit::subsystems::papyrus::core::knownScriptAboutToBeForgotten, this, [this](const auto& known_script) {
      for (auto* prop : this->_properties) {
         if (prop->typeinfo.object.definition == &known_script) // TODO: maybe use a refcounted known script pointer so we don't have to worry about this case at all?
            prop->typeinfo.object.definition = nullptr;
      }
   });
}
DKBoundScriptModel::DKBoundScriptModel(const DKBoundScriptModel& src, QObject* parent) : QAbstractItemModel(parent), _scriptname(src._scriptname) {
   size_t size = src._properties.size();
   this->_properties.resize(size);
   for (size_t i = 0; i < size; ++i) {
      this->_properties[i] = new property;
      auto& src_prop = *src._properties[i];
      auto& dst_prop = *this->_properties[i];
      dst_prop = src_prop;
   }
   this->_load_results = src._load_results;
   this->_status       = src._status;
   this->_cached       = src._cached;
}
DKBoundScriptModel::~DKBoundScriptModel() {
   this->_clear();
}

#pragma region Editor core hooks
void DKBoundScriptModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   for (size_t j = 0; j < this->_properties.size(); ++j) {
      auto* prop = this->_properties[j];

      bool changed = false;
      //
      if (false) { // TODO: if the base form is removed out from under us
         if (prop->values.inherited.has_value()) {
            //
            // We don't call `prop->clearParentBinding()` here because that recaches the value 
            // string, which may be redundant if `onFormDeletionImminent` below also changes 
            // the property's effective value.
            //
            prop->values.inherited.reset();
            changed = true;
         }
      }
      if (prop->on_form_deletion_imminent(*stub)) {
         changed = true;
      }
      //
      if (changed) {
         prop->recache_value_string();

         QModelIndex qmi_l = this->index(j, 0, {});
         QModelIndex qmi_r = this->index(j, ColumnCount - 1, {});

         emit dataChanged(qmi_l, qmi_r);
      }
   }
}
void DKBoundScriptModel::formModified(dovah::form_stub* stub) {
   for (size_t i = 0; i < this->_properties.size(); ++i) {
      auto* prop = this->_properties[i];

      if (prop->refers_to_form(*stub)) {
         prop->recache_value_string();
         auto index = this->index(i, Column::Value, {});
         emit dataChanged(index, index);
      }
   }
}
void DKBoundScriptModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   for (size_t i = 0; i < this->_properties.size(); ++i) {
      auto* prop = this->_properties[i];

      if (prop->refers_to_form(*stub)) {
         prop->recache_value_string();
         auto index = this->index(i, Column::Value, {});
         emit dataChanged(index, index);
      }
   }
}
void DKBoundScriptModel::formsRenumberedEnMasse() {
   //
   // We don't store enough information to check which list items have had their 
   // form IDs changed, so just blindly update the form IDs for all list items.
   //
   bool any = false;
   for (auto* prop : this->_properties) {
      if (prop->is_object_or_array_thereof()) {
         prop->recache_value_string();
         any = true;
      }
   }
   if (any) {
      QModelIndex upper_left  = this->index(0, Column::Value, {});
      QModelIndex lower_right = this->index(this->_properties.size() - 1, Column::Value, {});
      emit dataChanged(upper_left, lower_right);
   }
}
#pragma endregion

void DKBoundScriptModel::_clear() {
   for (auto* prop : this->_properties)
      delete prop;
   this->_properties.clear();
}

void DKBoundScriptModel::_load_property_definitions_from(std::string_view scriptname) {
   dovah::compiled_papyrus_script data;
   {
      auto& assets = dovahkit::subsystems::assets::get();

      std::filesystem::path path("scripts/");
      path /= std::string(scriptname) + ".pex";

      auto* file = assets.lookup_game_asset(path);
      if (!file) {
         this->_load_results.failed = true;
         return;
      }
      try {
         data.read_file(file->data(), file->size());
         delete file;
      } catch (dovah::compiled_papyrus_script::read_exception& e) {
         this->_load_results.failed = true;
         delete file;
         return;
      }
   }

   auto& papyrus = dovahkit::subsystems::papyrus::core::get();

   for (const auto& object : data.objects) {
      if (!dovah::papyrus::helpers::name_equals(object.name, scriptname))
         continue;
      for (const auto& prop : object.properties) {
         auto* dst_prop = this->_lookup_property(prop.name);
         if (!dst_prop) {
            dst_prop = new property;
            this->_properties.push_back(dst_prop);
         }

         dst_prop->name      = QString::fromStdString(prop.name);
         dst_prop->docstring = QString::fromStdString(prop.docstring);

         auto& dst_typeinfo = dst_prop->typeinfo;

         bool is_array  = prop.type.ends_with("[]");
         auto type_name = QString::fromStdString(prop.type).toLower();
         if (is_array)
            type_name.resize(type_name.size() - 2);
         //
         if (type_name == "bool") {
            dst_typeinfo.raw_type = vmad::property_type::boolean;
         } else if (type_name == "float") {
            dst_typeinfo.raw_type = vmad::property_type::float32;
         } else if (type_name == "int") {
            dst_typeinfo.raw_type = vmad::property_type::integer;
         } else if (type_name == "string") {
            dst_typeinfo.raw_type = vmad::property_type::string;
         } else {
            dst_typeinfo.raw_type = vmad::property_type::object;
         }
         //
         if (dst_typeinfo.raw_type == vmad::property_type::object) {
            bool matched = false;
            for (const auto& info : dovah::papyrus::native_classes) {
               auto info_name = QString::fromLatin1(info.name.data(), info.name.size());
               if (type_name.compare(info_name, Qt::CaseInsensitive) == 0) {
                  dst_typeinfo.object.native_type = info.form_type;
                  matched = true;
                  break;
               }
            }
            if (!matched) {
               dst_typeinfo.object.scriptname = type_name;

               auto* definition = papyrus.lookup_known_script(type_name);
               if (definition) {
                  dst_typeinfo.object.definition  = definition;
                  dst_typeinfo.object.native_type = definition->underlying_type();
               }
            }
         }
         //
         if (is_array) {
            dst_typeinfo.raw_type = vmad::array_property_type_for(dst_typeinfo.raw_type);
         }
      }
   }
}

/*static*/ std::optional<dovah::form_type> DKBoundScriptModel::_guess_object_property_type(const vmad_property_value& src) {
   bool is_array = std::holds_alternative<std::vector<vmad::property_object_value>>(src);
   if (is_array) {
      auto* src_p = std::get_if<std::vector<vmad::property_object_value>>(&src);
      if (!src_p)
         return {};
      auto&  src  = *src_p;
      size_t size = src.size();

      bool aliases = false;
      bool forms   = false;
      for (auto& item : src) {
         if (item.alias_id == vmad::property_object_value::no_alias) {
            if (item.form.get_form_stub() != nullptr)
               forms = true;
         } else {
            aliases = true;
         }
         if (forms && aliases)
            //
            // Mixed form-and-alias arrays are invalid in-game, as Form and Array are 
            // different native base classes. You can't get around that by using a 
            // script with no native base class, because as mentioned, the game won't 
            // let you attach that to anything.
            //
            return {};
      }
      if (aliases)
         return dovah::form_type::alias;
      return dovah::form_type::none;
   } else {
      auto* src_p = std::get_if<vmad::property_object_value>(&src);
      if (!src_p)
         return {};
      auto& src = *src_p;
      if (src.alias_id != vmad::property_object_value::no_alias)
         return dovah::form_type::alias;
      return dovah::form_type::none;
   }
}
std::optional<DKBoundScriptModel::PropertyValue> DKBoundScriptModel::_load_property_value(const vmad_property& src, const property& info) {
   std::optional<PropertyValue> out;
   switch (info.typeinfo.raw_type) {
      case vmad::property_type::object:
      case vmad::property_type::array_of_object:
         {
            bool is_alias = false;
            bool is_array = (info.typeinfo.raw_type == vmad::property_type::array_of_object);

            //
            // First, we need to figure out whether we're dealing with aliases or forms, so 
            // that we know how to store the value.
            //

            if (!info.typeinfo_is_unknown()) {
               //
               // We are capable of identifying this property's native base class, and handling 
               // object-type values accordingly.
               //
               is_alias = info.is_quest_alias_or_array_thereof();
            } else {
               auto guess = _guess_object_property_type(src.value);
               if (!guess.has_value())
                  return {};
               is_alias = guess.value() == dovah::form_type::alias;
            }

            //
            // Now that we know whether we're dealing with forms or aliases, let's retain the 
            // values.
            //

            if (is_array) {
               auto* src_p = std::get_if<std::vector<vmad::property_object_value>>(&src.value);
               if (!src_p)
                  return {};
               auto&  src  = *src_p;
               size_t size = src.size();
               if (is_alias) {
                  auto& dst = out.emplace().emplace<std::vector<ui::types::quest_alias>>();
                  dst.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     dst[i].quest    = src[i].form.get_form_stub();
                     dst[i].alias_id = src[i].alias_id;
                  }
               } else {
                  auto& dst = out.emplace().emplace<std::vector<dovah::form_stub*>>();
                  dst.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     if (src[i].alias_id != vmad::property_object_value::no_alias)
                        continue;
                     dst[i] = src[i].form.get_form_stub();
                  }
               }
            } else {
               auto* src_p = std::get_if<vmad::property_object_value>(&src.value);
               if (!src_p)
                  return {};
               auto& src = *src_p;
               if (is_alias) {
                  auto& dst = out.emplace().emplace<ui::types::quest_alias>();
                  dst.quest    = src.form.get_form_stub();
                  dst.alias_id = src.alias_id;
               } else {
                  if (src.alias_id != vmad::property_object_value::no_alias)
                     return {};
                  auto& dst = out.emplace().emplace<dovah::form_stub*>();
                  dst = src.form.get_form_stub();
               }
            }
         }
         break;
      default:
         std::visit(
            [&out](auto& src_v) {
               using value_type = std::decay_t<decltype(src_v)>;
               if constexpr (std::is_same_v<value_type, std::string>) {
                  out = QString::fromUtf8(QByteArray::fromStdString(src_v));
               } else if constexpr (std::is_same_v<value_type, std::vector<std::string>>) {
                  auto&  out_v = out.emplace().emplace<std::vector<QString>>();
                  size_t size  = src_v.size();
                  out_v.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     out_v[i] = QString::fromUtf8(QByteArray::fromStdString(src_v[i]));
                  }
               } else if constexpr (std::is_same_v<value_type, vmad::property_object_value> || std::is_same_v<value_type, std::vector<vmad::property_object_value>>) {
                  //
                  // No-op branch used to prevent the else-branch below from running into type errors. 
                  // This branch should never be reached, as we handle it in the switch-case above.
                  //
               } else {
                  out = src_v;
               }
            },
            src.value
         );
   }
   return out;
}

void DKBoundScriptModel::_update_any_properties_local() {
   this->_cached.any_properties_defined_locally = false;
   for (const auto* prop : this->_properties) {
      if (prop->values.local.has_value()) {
         this->_cached.any_properties_defined_locally = true;
         break;
      }
   }
}

void DKBoundScriptModel::initializeFrom(const vmad_script& local_script, const vmad::attachment_data& parent_vmad) {
   this->beginResetModel();

   for (auto* prop : this->_properties) {
      prop->values = {};
   }
   this->_cached.any_properties_defined_locally = false;
   this->_load_results.some_data_discarded      = false;

   auto* inheriting_from = parent_vmad.lookup_script(this->_scriptname.toUtf8().toStdString());

   for (auto& src : local_script.properties) {
      property* prop = this->_lookup_property(src.name);
      if (!prop) {
         this->_load_results.some_data_discarded = true;
         continue;
      }
      if (src.status == vmad::property_status::inherited_and_removed) {
         prop->values.local = std::monostate{};
      } else {
         prop->values.local = this->_load_property_value(src, *prop);
      }
      if (prop->values.local.has_value())
         this->_cached.any_properties_defined_locally = true;
   }
   if (inheriting_from) {
      for (auto& src : inheriting_from->properties) {
         property* prop = this->_lookup_property(src.name);
         if (!prop) {
            continue;
         }
         if (src.status == vmad::property_status::inherited_and_removed) {
            prop->values.inherited = std::monostate{};
         } else {
            prop->values.inherited = this->_load_property_value(src, *prop);
         }
      }
   }

   for (auto* prop : this->_properties)
      prop->recache_value_string();

   this->endResetModel();
}
void DKBoundScriptModel::initializeFrom(const vmad_script& local_script) {
   this->beginResetModel();

   for (auto* prop : this->_properties) {
      prop->values = {};
   }
   this->_cached.any_properties_defined_locally = false;
   this->_load_results.some_data_discarded      = false;

   for (auto& src : local_script.properties) {
      property* prop = this->_lookup_property(src.name);
      if (!prop) {
         this->_load_results.some_data_discarded = true;
         continue;
      }
      if (src.status == vmad::property_status::inherited_and_removed) {
         prop->values.local = std::monostate{};
      } else {
         prop->values.local = this->_load_property_value(src, *prop);
      }
      if (prop->values.local.has_value())
         this->_cached.any_properties_defined_locally = true;
   }

   for (auto* prop : this->_properties)
      prop->recache_value_string();

   this->endResetModel();
}
void DKBoundScriptModel::initializeFromInheritedOnly(const vmad_script& inherited_script) {
   this->beginResetModel();

   for (auto* prop : this->_properties) {
      prop->values = {};
   }
   this->_cached.any_properties_defined_locally = false;
   this->_load_results.some_data_discarded      = false;

   for (auto& src : inherited_script.properties) {
      property* prop = this->_lookup_property(src.name);
      if (!prop) {
         continue;
      }
      if (src.status == vmad::property_status::inherited_and_removed) {
         prop->values.inherited = std::monostate{};
      } else {
         prop->values.inherited = this->_load_property_value(src, *prop);
      }
   }

   for(auto* prop : this->_properties)
      prop->recache_value_string();

   this->endResetModel();
}
void DKBoundScriptModel::commitTo(vmad_script& local_script, dovah::loaded_forms::Form& working_copy) {
   local_script.clear_properties(working_copy);
   for (auto* src : this->_properties) {
      auto status_opt = src->get_computed_status();
      if (!status_opt.has_value()) {
         continue;
      }
      switch (status_opt.value()) {
         case vmad::property_status::defined_only_on_base:
            continue;
         case vmad::property_status::inherited_and_removed:
            if (this->_status.inherited == false)
               continue;
            break;
      }

      auto& dst = local_script.properties.emplace_back();

      dst.name   = src->name.toUtf8().toStdString();
      dst.status = status_opt.value();
      if (dst.status != vmad::property_status::inherited_and_removed) {
         auto& value_opt = src->values.local;
         if (value_opt.has_value()) {
            auto& value = value_opt.value();
            if (!std::holds_alternative<std::monostate>(value)) {
               ui::bound_script_models::property_value_to_vmad(value, dst.value, working_copy);
            }
         }
      }
   }
   if (this->_status.inherited && !this->_status.cleared && this->_cached.any_properties_defined_locally) {
      if (local_script.status != vmad::script_status::removed)
         local_script.status = vmad::script_status::overrides_base;
   }
}
      
#pragma region QAbstractItemModel overrides
   /*virtual*/ QModelIndex DKBoundScriptModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
      if (!this->hasIndex(row, column, parent))
         return {};
      if (row < 0 || row >= this->_properties.size())
         return {};
      return this->createIndex(row, column, (void*)this->_properties[row]);
   }
   /*virtual*/ QModelIndex DKBoundScriptModel::parent(const QModelIndex& index) const /*override*/ {
      return {};
   }
   /*virtual*/ int DKBoundScriptModel::rowCount(const QModelIndex& parent) const /*override*/ {
      return this->_properties.size();
   }
   /*virtual*/ int DKBoundScriptModel::columnCount(const QModelIndex&) const /*override*/ {
      return ColumnCount;
   }
   /*virtual*/ Qt::ItemFlags DKBoundScriptModel::flags(const QModelIndex& index) const /*override*/ {
      if (!index.isValid())
         return {};
      return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
   }
   /*virtual*/ QVariant DKBoundScriptModel::data(const QModelIndex& qmi, int role) const /*override*/ {
      if (!qmi.isValid())
         return {};
      if (qmi.internalPointer() == nullptr)
         return {};
      auto* prop = this->_property(qmi);
      if (!prop)
         return {};

      switch (role) {
         case IsArrayRole:
            return vmad::property_type_is_array(prop->typeinfo.raw_type);
         case NativeTypeRole:
            if (prop->typeinfo.object.native_type.has_value())
               return (int)prop->typeinfo.object.native_type.value();
            return {};
      }
      
      switch (qmi.column()) {
         case Column::Name:
            switch (role) {
               case Qt::DisplayRole:
                  return prop->name;
               case Qt::ToolTipRole:
                  {
                     QString tooltip;

                     auto status_opt = prop->get_computed_status();
                     if (!status_opt.has_value()) {
                        tooltip = tr("Status: Property unmodified");
                     } else {
                        switch (status_opt.value()) {
                           case vmad::property_status::defined_locally:
                              if (prop->is_inherited()) {
                                 tooltip = tr("Status: Property inherited and edited locally");
                              } else {
                                 tooltip = tr("Status: Property edited locally");
                              }
                              break;
                           case vmad::property_status::defined_only_on_base:
                              tooltip = tr("Status: Property inherited from parent");
                              break;
                           case vmad::property_status::inherited_and_removed:
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
                     return tooltip;
                  }
                  break;
               case Qt::DecorationRole:
                  {
                     auto status_opt = prop->get_computed_status();
                     if (!status_opt.has_value()) {
                        return _blank_icon();
                     }
                     switch (status_opt.value()) {
                        case vmad::property_status::defined_locally:
                           if (prop->is_inherited()) {
                              return QIcon(":/icons/papyrus-status-icons/inherited-edited.png");
                           }
                           return QIcon(":/icons/papyrus-status-icons/added-edited.png");
                        case vmad::property_status::defined_only_on_base:
                           return QIcon(":/icons/papyrus-status-icons/inherited.png");
                        case vmad::property_status::inherited_and_removed:
                           return QIcon(":/icons/papyrus-status-icons/removed.png");
                     }
                  }
                  break;
            }
            break;
         case Column::Type:
            if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
               return prop->type_string();
            }
            break;
         case Column::Value:
            if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
               return prop->cached.value_string;
            }
            break;
      }
      return {};
   }
   /*virtual*/ QVariant DKBoundScriptModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole)
         return {};
      switch (section) {
         case 0:
            return tr("Property Name");
         case 1:
            return tr("Type");
         case 2:
            return tr("Value");
      }
      return {};
   }
#pragma endregion

void DKBoundScriptModel::_emit_row_changed(const QModelIndex& qmi) {
   emit dataChanged(qmi.siblingAtColumn(0), qmi.siblingAtColumn(2));
}
DKBoundScriptModel::property* DKBoundScriptModel::_lookup_property(std::string_view desired) {
   size_t size = desired.size();
   for (auto* prop : this->_properties) {
      assert(prop != nullptr);
      auto& name = prop->name;
      if (name.size() != size)
         continue;
      bool match = true;
      for (size_t i = 0; i < size; ++i) {
         auto a = desired[i];
         auto b = name[(uint)i].unicode();
         if (a == b)
            continue;
         if (a >= 'a' && a <= 'z')
            a -= 0x20;
         if (b >= 'a' && b <= 'z')
            b -= 0x20;
         if (a != b) {
            match = false;
            break;
         }
      }
      if (match)
         return prop;
   }
   return nullptr;
}
const DKBoundScriptModel::property* DKBoundScriptModel::_property(const QModelIndex& qmi) const {
   if (!qmi.isValid() || qmi.model() != this)
      return nullptr;
   auto i = qmi.row();
   if (i > this->_properties.size())
      return nullptr;
   auto* prop = this->_properties[i];
   assert(prop != nullptr);
   return prop;
}
DKBoundScriptModel::property* DKBoundScriptModel::_property(const QModelIndex& qmi) {
   return const_cast<property*>(std::as_const(*this)._property(qmi));
}

void DKBoundScriptModel::autoFillProperty(const QModelIndex& qmi) {
   auto* prop = _property(qmi);
   if (!prop)
      return;
   if (prop->autofill()) {
      prop->recache_value_string();
      this->_cached.any_properties_defined_locally = true;
      _emit_row_changed(qmi);
   }
}
void DKBoundScriptModel::clearProperty(const QModelIndex& qmi) {
   auto* prop = _property(qmi);
   if (!prop)
      return;
   if (prop->clear()) {
      prop->recache_value_string();
      this->_update_any_properties_local();
      _emit_row_changed(qmi);
   }
}
void DKBoundScriptModel::makePropertyLocal(const QModelIndex& qmi) {
   auto* prop = _property(qmi);
   if (!prop)
      return;
   if (prop->values.local.has_value())
      return;
   prop->make_local();
   prop->recache_value_string();
   this->_cached.any_properties_defined_locally = true;
   _emit_row_changed(qmi);
}
void DKBoundScriptModel::revertProperty(const QModelIndex& qmi) {
   auto* prop = _property(qmi);
   if (!prop)
      return;
   if (!prop->values.inherited.has_value())
      return;
   if (!prop->values.local.has_value())
      return;
   prop->revert();
   prop->recache_value_string();
   this->_update_any_properties_local();
   _emit_row_changed(qmi);
}
void DKBoundScriptModel::setPropertyLocalValue(const QModelIndex& qmi, const PropertyValue& src) {
   auto* prop = _property(qmi);
   if (!prop)
      return;

   if (ui::bound_script_models::property_type_for(src) != prop->typeinfo.raw_type)
      return;

   prop->set_local_value(src);
   prop->recache_value_string();
   this->_cached.any_properties_defined_locally = true;
   _emit_row_changed(qmi);
}
void DKBoundScriptModel::setPropertyLocalValueElement(const QModelIndex& qmi, const PropertyValue& src, size_t array_index) {
   auto* prop = _property(qmi);
   if (!prop)
      return;

   std::visit(
      [](auto& src_v) {
         using value_type = std::decay_t<decltype(src_v)>;
         assert(!cobb::is_std_vector<value_type> && "Don't try to store an array inside of another array!");
      },
      src
   );

   if (!ui::bound_script_models::vmad::property_type_is_array(prop->typeinfo.raw_type))
      return;

   {
      auto dst_type = ui::bound_script_models::vmad::scalar_property_type_for(prop->typeinfo.raw_type);
      auto src_type = ui::bound_script_models::property_type_for(src);
      if (src_type != dst_type)
         return;
   }

   auto& opt_local     = prop->values.local;
   auto& opt_inherited = prop->values.inherited;

   PropertyValue array;
   if (opt_local.has_value()) {
      array = opt_local.value();
      if (std::holds_alternative<std::monostate>(array))
         return;
   } else if (opt_inherited.has_value()) {
      array = opt_inherited.value();
   } else {
      return;
   }
   bool success = false;
   std::visit(
      [&src, array_index, &success](auto& dst) {
         using dst_type = std::decay_t<decltype(dst)>;
         assert(cobb::is_std_vector<dst_type>);
         if constexpr (cobb::is_std_vector<dst_type>) {
            using element_type = typename dst_type::value_type;
            assert(std::holds_alternative<element_type>(src));
            if (array_index < dst.size()) {
               success          = true;
               dst[array_index] = std::get<element_type>(src);
            }
         }
      },
      array
   );
   if (success) {
      prop->recache_value_string();
      this->_cached.any_properties_defined_locally = true;
      _emit_row_changed(qmi);
   }
}

void DKBoundScriptModel::autoFillAllProperties() {
   for (size_t i = 0; i < this->_properties.size(); ++i) {
      auto* prop = this->_properties[i];
      assert(prop != nullptr);
      if (prop->values.local.has_value()) {
         auto& value = prop->values.local.value();
         if (!std::holds_alternative<std::monostate>(value)) {
            //
            // If you select a specific property and click "Auto-Fill," then the CK will 
            // clobber its current value. However, "Auto-Fill All" will never clobber any 
            // values.
            //
            continue;
         }
      }
      if (prop->autofill()) {
         prop->recache_value_string();
         this->_cached.any_properties_defined_locally = true;
         auto qmi = this->index(i, 0, {});
         emit dataChanged(qmi, qmi.siblingAtColumn(2));
      }
   }
}

std::optional<DKBoundScriptModel::PropertyInfo> DKBoundScriptModel::infoForProperty(const QModelIndex& qmi) const {
   auto* prop = this->_property(qmi);
   if (!prop)
      return {};
   PropertyInfo out;

   out.is_array    = vmad::property_type_is_array(prop->typeinfo.raw_type);
   out.scriptname  = prop->typeinfo.object.scriptname;
   out.raw_type    = prop->typeinfo.raw_type;
   out.native_type = prop->typeinfo.object.native_type;
   if (prop->typeinfo.object.guessed_type)
      out.native_type = prop->typeinfo.object.guessed_type;

   out.inherited = prop->is_inherited();
   {
      auto status = prop->get_computed_status();
      if (status.has_value()) {
         switch (status.value()) {
            case vmad::property_status::inherited_and_removed:
               out.cleared = true;
               break;
            case vmad::property_status::defined_locally:
               out.defined_locally = true;
               break;
         }
      }
   }

   return out;
}

std::optional<DKBoundScriptModel::PropertyValue> DKBoundScriptModel::getPropertyValue(const QModelIndex& qmi, bool local_only) const {
   auto* prop = _property(qmi);
   if (!prop)
      return {};

   auto& opt_local     = prop->values.local;
   auto& opt_inherited = prop->values.inherited;

   if (opt_local.has_value()) {
      return opt_local.value();
   } else {
      if (!local_only)
         return {};
   }
   if (opt_inherited.has_value())
      return opt_inherited.value();
   
   return {};
}
std::optional<DKBoundScriptModel::PropertyValue> DKBoundScriptModel::getPropertyValueElement(const QModelIndex& qmi, size_t array_index, bool local_only) const {
   auto* prop = _property(qmi);
   if (!prop)
      return {};
   if (!ui::bound_script_models::vmad::property_type_is_array(prop->typeinfo.raw_type))
      return {};
   
   auto& opt_local     = prop->values.local;
   auto& opt_inherited = prop->values.inherited;
   if (local_only && !opt_local.has_value())
      return {};
   const auto& value_opt = opt_local.has_value() ? opt_local : opt_inherited;
   if (!value_opt.has_value())
      return {};

   const auto& value = value_opt.value();

   std::optional<DKBoundScriptModel::PropertyValue> out;
   std::visit(
      [&out, array_index](const auto& src_v) {
         using value_type = std::decay_t<decltype(src_v)>;
         if constexpr (cobb::is_std_vector<value_type>) {
            if (array_index >= src_v.size())
               return;
            out = src_v[array_index];
         }
      },
      value
   );
   return out;
}