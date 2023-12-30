#include "DKPapyrusScriptObjectModel.h"
#include <string>
#include <vector>
#include <QIcon>
#include "helpers/qt/strings.h"
#include "helpers/type_traits/is_std_vector.h"
#include "helpers/type_containers/fixed_map.h"
#include "helpers/function_traits.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/assets.h"
#include "editor/core.h"

// For loading PEXs:
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/papyrus/compiled_script.h"

/*

   TODO:

    - When no properties, grey out entire pane and show "<no properties>" in first row's name column
 
*/

namespace {
   namespace model {
      using object_property_value = DKPapyrusScriptObjectModel::object_property_value;
      using property_value        = DKPapyrusScriptObjectModel::property_value;
   }
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
      using object_property_value = property_object_value;
   }
   using papyrus_attachment_data = DKPapyrusScriptObjectModel::papyrus_attachment_data;
   using property_value_type     = DKPapyrusScriptObjectModel::property_value_type;
}

namespace {
   template<typename Src>
   struct value_conversion_handlers {
      static void convert() {} // dummy, so `handler_info` doesn't fail outright if instantiated on a value type with no handler
   };

   #pragma region value_conversion_handlers<SourceType> specializations
   //
   // Specializations should look like:
   // 
   //    template<>
   //    struct value_conversion_handlers<SourceType> {
   //       void convert(const SourceType& src, DestinationType& dst);
   //    };
   // 
   // Or, if you're converting from the UI-side type to the Dovah-side type and the data 
   // includes form stubs (which need to be crammed into form_reference_ts, which in turn 
   // requires access to the owning form):
   // 
   //    template<>
   //    struct value_conversion_handlers<SourceType> {
   //       void convert(const SourceType& src, DestinationType& dst, dovah::loaded_forms::Form& dst_form);
   //    };
   //

   template<>
   struct value_conversion_handlers<vmad::object_property_value> {
      static void convert(const vmad::object_property_value& src, model::object_property_value& dst) {
         dst = model::object_property_value{
            .form     = src.form.get_form_stub(),
            .alias_id = src.alias_id,
         };
      }
   };
   template<>
   struct value_conversion_handlers<model::object_property_value> {
      static void convert(const model::object_property_value& src, vmad::object_property_value& dst, dovah::loaded_forms::Form& dst_form) {
         dst.alias_id = src.alias_id;
         dst.form.set(dst_form, src.form);
      }
   };
   
   template<>
   struct value_conversion_handlers<std::string> {
      static void convert(const std::string& src, QString& dst) {
         dst = QString::fromStdString(src);
      }
   };
   template<>
   struct value_conversion_handlers<QString> {
      static void convert(const QString& src, std::string& dst) {
         dst = src.toStdString();
      }
   };
   #pragma endregion

   namespace _impl {
      template<typename T>
      using handler_traits = cobb::function_traits<decltype(&value_conversion_handlers<T>::convert)>;

      template<typename T>
      struct handler_info {
         using src_type = std::tuple_element_t<0, typename handler_traits<T>::arg_tuple>;
         using dst_type = std::tuple_element_t<1, typename handler_traits<T>::arg_tuple>;

         static constexpr const bool src_type_is_valid = std::is_lvalue_reference_v<src_type> && std::is_const_v<std::remove_reference_t<src_type>>;
         static constexpr const bool dst_type_is_valid = std::is_lvalue_reference_v<dst_type> && !std::is_const_v<std::remove_reference_t<dst_type>>;
      };

      template<typename T>
      concept value_convertible_without_form = requires {
         requires (handler_traits<T>::arg_count == 2);
         requires handler_info<T>::src_type_is_valid;
         requires handler_info<T>::dst_type_is_valid;
      };
      template<typename T>
      concept value_convertible_with_form = requires {
         requires (handler_traits<T>::arg_count == 3);
         requires handler_info<T>::src_type_is_valid;
         requires handler_info<T>::dst_type_is_valid;
         //
         requires std::is_same_v<std::tuple_element_t<2, typename handler_traits<T>::arg_tuple>, dovah::loaded_forms::Form&>;
      };

      template<typename T>
      struct value_requires_conversion {
         static constexpr const bool value = false;
      };
      template<typename T> requires (handler_traits<T>::arg_count >= 2)
      struct value_requires_conversion<T> {
         static constexpr const bool value = value_convertible_without_form<T> || value_convertible_with_form<T>;
      };
   }
   //
   template<typename T>
   concept value_requires_conversion = _impl::value_requires_conversion<T>::value;

   template<value_requires_conversion T>
   using value_converts_to = std::decay_t<typename _impl::handler_info<T>::dst_type>;

   model::property_value _convert_value(const vmad::property_value& v) {
      model::property_value out;
      std::visit(
         [&out](auto& src) {
            using value_type = std::decay_t<decltype(src)>;

            if constexpr (cobb::is_std_vector<value_type>) {
               using item_type = typename value_type::value_type;
               if constexpr (value_requires_conversion<item_type>) {
                  size_t size = src.size();
                  auto&  dst  = out.emplace<std::vector<value_converts_to<item_type>>>();
                  dst.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     value_conversion_handlers<item_type>::convert(src[i], dst[i]);
                  }
               } else {
                  out = src;
               }
            } else if constexpr (value_requires_conversion<value_type>) {
               auto& dst = out.emplace<value_converts_to<value_type>>();
               value_conversion_handlers<value_type>::convert(src, dst);
            } else {
               out = src;
            }
         },
         v
      );
      return out;
   }

   void _convert_value(const model::property_value& src_v, vmad::property_value& dst_v, dovah::loaded_forms::Form& dst_form) {
      std::visit(
         [&dst_v, &dst_form](auto& src) {
            using value_type = std::decay_t<decltype(src)>;

            if constexpr (cobb::is_std_vector<value_type>) {
               using item_type = typename value_type::value_type;
               if constexpr (value_requires_conversion<item_type>) {
                  size_t size = src.size();
                  auto&  dst  = dst_v.emplace<std::vector<value_converts_to<item_type>>>();
                  dst.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     if constexpr (_impl::value_convertible_with_form<item_type>) {
                        value_conversion_handlers<item_type>::convert(src[i], dst[i], dst_form);
                     } else {
                        value_conversion_handlers<item_type>::convert(src[i], dst[i]);
                     }
                  }
               } else {
                  dst_v = src;
               }
            } else if constexpr (value_requires_conversion<value_type>) {
               auto& dst = dst_v.emplace<value_converts_to<value_type>>();
               if constexpr (_impl::value_convertible_with_form<value_type>) {
                  value_conversion_handlers<value_type>::convert(src, dst, dst_form);
               } else {
                  value_conversion_handlers<value_type>::convert(src, dst);
               }
            } else {
               dst_v = src;
            }
         },
         src_v
      );
   }

   QString _stringify_value(bool v) {
      return v ? "True" : "False";
   }
   QString _stringify_value(float v) {
      return QString::number(v);
   }
   QString _stringify_value(int32_t v) {
      return QString::number(v);
   }
   QString _stringify_value(const model::object_property_value& v) {
      auto form_str = editor_helpers::form_identifiers_to_string(v.form);
      if (v.alias_id != -1) {
         return QString("Quest alias ID#%1 on %2")
            .arg(v.alias_id) // TODO: Look up quest, load it if present, and display alias name.
            .arg(form_str);
      }
      return form_str;
   }
   QString _stringify_value(QString v) {
      return v;
   }
   //
   QString _stringify_value(const model::property_value& value) {
      if (std::holds_alternative<std::monostate>(value)) {
         return "None";
      }
      QString out;
      std::visit(
         [&out](const auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               out = "[ ";
               size_t size = v.size();
               for (size_t i = 0; i < size; ++i) {
                  out += _stringify_value(v[i]);
                  if (i + 1 < size)
                     out += ", ";
               }
               out += " ]";
            } else {
               out = _stringify_value(v);
            }
         },
         value
      );
      return out;
   }
}

#pragma region property
vmad::property_type DKPapyrusScriptObjectModel::Property::Binding::typecode() const {
   if (std::holds_alternative<std::monostate>(this->value))
      return vmad::property_type::none;
   //
   if (std::holds_alternative<bool>(this->value))
      return vmad::property_type::boolean;
   if (std::holds_alternative<float>(this->value))
      return vmad::property_type::float32;
   if (std::holds_alternative<int32_t>(this->value))
      return vmad::property_type::integer;
   if (std::holds_alternative<model::object_property_value>(this->value))
      return vmad::property_type::object;
   if (std::holds_alternative<QString>(this->value))
      return vmad::property_type::string;
   //
   if (std::holds_alternative<std::vector<bool>>(this->value))
      return vmad::property_type::array_of_boolean;
   if (std::holds_alternative<std::vector<float>>(this->value))
      return vmad::property_type::array_of_float32;
   if (std::holds_alternative<std::vector<int32_t>>(this->value))
      return vmad::property_type::array_of_integer;
   if (std::holds_alternative<std::vector<model::object_property_value>>(this->value))
      return vmad::property_type::array_of_object;
   if (std::holds_alternative<std::vector<QString>>(this->value))
      return vmad::property_type::array_of_string;
   //
   return vmad::property_type::none;
}

void DKPapyrusScriptObjectModel::Property::clearParentBinding() {
   this->bindings.parent = {};
   this->recacheValueString();
}
void DKPapyrusScriptObjectModel::Property::clearTargetBinding() {
   this->bindings.target = {};
   this->recacheValueString();
}
void DKPapyrusScriptObjectModel::Property::setParentBinding(const vmad_property& src) {
   this->bindings.parent = {
      .status = src.status,
      .value  = _convert_value(src.value)
   };
   this->recacheValueString();
}
void DKPapyrusScriptObjectModel::Property::setTargetBinding(const vmad_property& src) {
   this->bindings.target = {
      .status = src.status,
      .value  = _convert_value(src.value)
   };
   this->recacheValueString();
}
void DKPapyrusScriptObjectModel::Property::setBindings(const vmad_property& parent, const vmad_property& target) {
   this->bindings.parent = {
      .status = parent.status,
      .value  = _convert_value(parent.value)
   };
   this->bindings.target = {
      .status = target.status,
      .value  = _convert_value(target.value)
   };
   this->recacheValueString();
}

bool DKPapyrusScriptObjectModel::Property::valueTypeIsOrContainsForm() const {
   if (auto& bind_opt = this->bindings.parent; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      if (std::holds_alternative<model::object_property_value>(bind.value))
         return true;
      if (std::holds_alternative<std::vector<model::object_property_value>>(bind.value))
         return true;
   }
   if (auto& bind_opt = this->bindings.target; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      if (std::holds_alternative<model::object_property_value>(bind.value))
         return true;
      if (std::holds_alternative<std::vector<model::object_property_value>>(bind.value))
         return true;
   }
   return false;
}
bool DKPapyrusScriptObjectModel::Property::isOrContainsForm(const dovah::form_stub& stub) const {
   auto _check = [](const dovah::form_stub& stub, const std::optional<Binding>& bind_opt) -> bool {
      if (!bind_opt.has_value())
         return false;
      auto& bind = bind_opt.value();
      if (auto* casted = std::get_if<model::object_property_value>(&bind.value)) {
         return casted->form == &stub;
      } else if (auto* casted = std::get_if<std::vector<model::object_property_value>>(&bind.value)) {
         for (auto& item : *casted)
            if (item.form == &stub)
               return true;
         return false;
      }
      return false;
   };

   if (_check(stub, this->bindings.parent))
      return true;
   if (_check(stub, this->bindings.target))
      return true;

   return false;
}

void DKPapyrusScriptObjectModel::Property::recacheValueString() {
   if (auto& bind_opt = this->bindings.target; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      switch (bind.status) {
         case property_status::unknown:
         case property_status::defined_locally:
            this->value_string = _stringify_value(bind.value);
            return;
         case property_status::inherited_and_removed:
            this->value_string = "None";
            return;
      }
   }
   if (auto& bind_opt = this->bindings.parent; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      switch (bind.status) {
         case property_status::defined_locally:
         case property_status::defined_only_on_base:
            this->value_string = _stringify_value(bind.value);
            return;
      }
   }
   this->value_string = "<<Default>>";
}
bool DKPapyrusScriptObjectModel::Property::onFormDeletionImminent(const dovah::form_stub& stub) {
   bool any_changed = false;

   auto _check = [](const dovah::form_stub& stub, std::optional<Binding>& bind_opt) -> bool {
      if (!bind_opt.has_value())
         return false;
      auto& bind = bind_opt.value();
      if (auto* casted = std::get_if<model::object_property_value>(&bind.value)) {
         if (casted->form == &stub) {
            casted->form = nullptr;
            return true;
         }
         return false;
      } else if (auto* casted = std::get_if<std::vector<model::object_property_value>>(&bind.value)) {
         bool seen = false;
         for (auto& item : *casted) {
            if (item.form == &stub) {
               item.form = nullptr;
               seen = true;
            }
         }
         return seen;
      }
      return false;
   };

   any_changed |= _check(stub, this->bindings.parent);
   //
   // If we're deleting the form referenced by the parent BoundScript, then DovahKit should 
   // update that BoundScript during the deletion process, to clear the form out. We should 
   // thus clear it out in our copy of that BoundScript's property data. The way we clear it 
   // should be made to match the backend, i.e. if we update the backend to straight-up 
   // delete properties that pointed to deleted forms, then we should do the same here.

   any_changed |= _check(stub, this->bindings.target);

   return any_changed;
}

void DKPapyrusScriptObjectModel::Property::changeValueTo(const property_value& src) {
   this->bindings.target = {
      .status = property_status::defined_locally,
      .value  = src,
   };
   this->recacheValueString();
}

std::optional<vmad::property_status> DKPapyrusScriptObjectModel::Property::getComputedStatus() const {
   if (this->bindings.target.has_value()) {
      return this->bindings.target.value().status;
   }
   if (this->bindings.parent.has_value()) {
      return property_status::defined_only_on_base;
   }
   return {};
}
#pragma endregion

DKPapyrusScriptObjectModel::DKPapyrusScriptObjectModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &DKPapyrusScriptObjectModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formRenumbered,         this, &DKPapyrusScriptObjectModel::formRenumbered);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &DKPapyrusScriptObjectModel::clearTarget);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &DKPapyrusScriptObjectModel::formsRenumberedEnMasse);
}

void DKPapyrusScriptObjectModel::_loadPropertiesFromPex(QMap<QString, Property>& dst) {
   if (this->scriptname.isEmpty())
      return;

   dovah::compiled_papyrus_script data;

   auto& assets = dovahkit::subsystems::assets::get();

   std::filesystem::path path("scripts/");
   path /= this->scriptname.toStdString() + ".pex";

   auto* file = assets.lookup_game_asset(path);
   try {
      data.read_file(file->data(), file->size());
   } catch (dovah::compiled_papyrus_script::read_exception& e) {
      return;
   }

   auto scriptname = this->scriptname.toStdString();
   for (const auto& object : data.objects) {
      if (!dovah::papyrus::helpers::name_equals(object.name, scriptname))
         continue;
      for (const auto& prop : object.properties) {
         QString name       = QString::fromStdString(prop.name);
         QString name_lower = name.toLower();

         auto& dst_prop = dst[name_lower];
         if (!dst_prop.type.has_value())
            dst_prop.type.emplace();
         auto& dst_type = dst_prop.type.value();

         dst_prop.name      = std::move(name);
         dst_prop.docstring = QString::fromStdString(prop.docstring);
         dst_type.name      = QString::fromStdString(prop.type);

         bool is_array = false;
         if (dst_type.name.size() > 2) {
            if (dst_type.name.endsWith("[]"))
               is_array = true;
         }
         auto type_name = dst_type.name.toLower();
         if (is_array)
            type_name.resize(type_name.size() - 2);
         //
         if (type_name == "bool") {
            dst_type.underlying_type = vmad::property_type::boolean;
         } else if (type_name == "float") {
            dst_type.underlying_type = vmad::property_type::float32;
         } else if (type_name == "int") {
            dst_type.underlying_type = vmad::property_type::integer;
         } else if (type_name == "string") {
            dst_type.underlying_type = vmad::property_type::string;
         } else {
            dst_type.underlying_type = vmad::property_type::object;
         }
         //
         if (is_array) {
            dst_type.underlying_type = vmad::array_property_type_for(dst_type.underlying_type);
         }
      }
   }
}
void DKPapyrusScriptObjectModel::_importProperties(QMap<QString, Property>& map) {
   this->properties.clear();
   this->properties.reserve(map.size());
   for (auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
      this->properties.push_back(std::move(it->second));
   }
}

void DKPapyrusScriptObjectModel::setTarget(dovah::form_stub& target_stub, vmad_script& target) {
   if (this->attached_to == &target_stub && this->vmad_scripts.target == &target && this->vmad_scripts.parent == nullptr)
      return;

   this->beginResetModel();

   this->attached_to = &target_stub;
   this->vmad_scripts.target = &target;
   this->vmad_scripts.parent = nullptr;
   //
   this->scriptname = QString::fromStdString(target.name);

   QMap<QString, Property> working;
   this->_loadPropertiesFromPex(working);
   for (const auto& src : target.properties) {
      QString name_lower = QString::fromStdString(src.name).toLower();

      auto& data = working[name_lower];
      if (data.name.isEmpty()) {
         data.name = QString::fromStdString(src.name);
      }
      data.bindings.target = {
         .status = src.status,
         .value  = _convert_value(src.value),
      };
   }
   this->_importProperties(working);

   this->endResetModel();
}
void DKPapyrusScriptObjectModel::setTarget(dovah::form_stub& target_stub, vmad_script& target, vmad_script& parent) {
   if (this->attached_to == &target_stub && this->vmad_scripts.target == &target && this->vmad_scripts.parent == &parent)
      return;
   assert(target.name_matches(parent.name));

   this->beginResetModel();

   this->attached_to = &target_stub;
   this->vmad_scripts.target = &target;
   this->vmad_scripts.parent = &parent;
   //
   this->scriptname = QString::fromStdString(target.name);

   QMap<QString, Property> working;
   this->_loadPropertiesFromPex(working);
   for (const auto& src : parent.properties) {
      QString name_lower = QString::fromStdString(src.name).toLower();

      auto& data = working[name_lower];
      if (data.name.isEmpty()) {
         data.name = QString::fromStdString(src.name);
      }
      data.bindings.parent = {
         .status = src.status,
         .value  = _convert_value(src.value),
      };
   }
   for (const auto& src : target.properties) {
      QString name_lower = QString::fromStdString(src.name).toLower();

      auto& data = working[name_lower];
      if (data.name.isEmpty()) {
         data.name = QString::fromStdString(src.name);
      }
      data.bindings.target = {
         .status = src.status,
         .value  = _convert_value(src.value),
      };
   }
   this->_importProperties(working);

   for (auto& item : this->properties) {
      item.recacheValueString();
   }

   this->endResetModel();
}

void DKPapyrusScriptObjectModel::clearTarget() {
   this->beginResetModel();
   this->attached_to = nullptr;
   this->vmad_scripts = {};
   this->scriptname.clear();
   this->properties.clear();
   this->endResetModel();
}
void DKPapyrusScriptObjectModel::syncToTarget() {
   if (this->attached_to == nullptr)
      return;
   if (this->vmad_scripts.target == nullptr)
      return;

   auto loaded = this->attached_to->get_content_if_loaded();
   assert(loaded);

   this->vmad_scripts.target->clear_properties(*loaded);
   for (const auto& item : this->properties) {
      auto& target_bind_opt = item.bindings.target;
      auto& parent_bind_opt = item.bindings.parent;

      if (target_bind_opt.has_value()) {
         auto& dst = this->vmad_scripts.target->properties.emplace_back();

         auto& target_bind = target_bind_opt.value();
         if (target_bind.status == property_status::inherited_and_removed) {
            dst.status = target_bind.status;
            dst.value  = {};
         } else {
            dst.status = target_bind.status;
            _convert_value(target_bind.value, dst.value, *loaded);
         }
         continue;
      }
   }
}

#pragma region Editor core hooks
void DKPapyrusScriptObjectModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (stub == this->attached_to && !is_just_flagged) {
      this->clearTarget();
      return;
   }

   auto& list = this->properties;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (item.onFormDeletionImminent(*stub)) {
         auto index = this->index(i, ColumnValue, {});
         emit dataChanged(index, index);
      }
   }
}
void DKPapyrusScriptObjectModel::formRenumbered(const dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
   auto& list = this->properties;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (item.isOrContainsForm(*stub)) {
         item.recacheValueString();
         auto index = this->index(i, 2, QModelIndex());
         emit dataChanged(index, index);
         return;
      }
   }
}
void DKPapyrusScriptObjectModel::formsRenumberedEnMasse() {
   //
   // We don't store enough information to check which list items have had their 
   // form IDs changed, so just blindly update the form IDs for all list items.
   //
   bool any = false;
   for (auto& item : this->properties) {
      if (item.valueTypeIsOrContainsForm()) {
         item.recacheValueString();
         any = true;
      }
   }
   if (!any)
      return;

   QModelIndex upper_left  = this->index(0, ColumnValue, {});
   QModelIndex lower_right = this->index(this->properties.size() - 1, ColumnValue, {});
   emit dataChanged(upper_left, lower_right);
}
#pragma endregion

#pragma region QAbstractItemModel overrides
QModelIndex DKPapyrusScriptObjectModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return {};
   if (row >= 0 && row < this->properties.size())
      return this->createIndex(row, column, (void*)&this->properties[row]);
   return {};
}
QModelIndex DKPapyrusScriptObjectModel::parent(const QModelIndex& index) const {
   return {};
}
int DKPapyrusScriptObjectModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->properties.size();
}
int DKPapyrusScriptObjectModel::columnCount(const QModelIndex& item) const {
   return 3;
}
Qt::ItemFlags DKPapyrusScriptObjectModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return {};
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DKPapyrusScriptObjectModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto item   = (const Property*)index.internalPointer();
   auto column = index.column();
   switch (column) {
      case ColumnName: // editor ID or parent cell information
         switch (role) {
            case Qt::DisplayRole:
               return item->name;
            case Qt::ToolTipRole:
               {
                  QString out = [&item](){
                     auto status_opt = item->getComputedStatus();
                     if (!status_opt.has_value())
                        return tr("Status: Property unmodified");
                     switch (status_opt.value()) {
                        case property_status::defined_locally:
                           if (item->bindings.parent.has_value()) {
                              return tr("Status: Property inherited and edited locally");
                           }
                           return tr("Status: Property edited locally");
                        case property_status::defined_only_on_base:
                           return tr("Status: Property inherited from parent");
                        case property_status::inherited_and_removed:
                           return tr("Status: Property inherited and cleared locally");
                     }
                     return tr("Status: <unknown>");
                  }();
                  if (!item->docstring.isEmpty()) {
                     out += "\n";
                     out += item->docstring;
                  }
                  return out;
               }
               break;
            case Qt::DecorationRole:
               {
                  auto status_opt = item->getComputedStatus();
                  if (!status_opt.has_value()) {
                     return {}; // Call QTableView::setIconSize to ensure there's always space reserved for icons.
                  }
                  switch (status_opt.value()) {
                     case property_status::defined_locally:
                        if (item->bindings.parent.has_value()) {
                           return QIcon(":/icons/papyrus-status-icons/inherited-edited.png");
                        }
                        return QIcon(":/icons/papyrus-status-icons/added-edited.png");
                     case property_status::defined_only_on_base:
                        return QIcon(":/icons/papyrus-status-icons/inherited.png");
                     case property_status::inherited_and_removed:
                        return QIcon(":/icons/papyrus-status-icons/removed.png");
                  }
               }
               break;
         }
         break;
      case ColumnType:
         if (role != Qt::DisplayRole)
            break;
         {
            auto _typecode_to_string = [](property_value_type t) -> QString {
               switch (t) {
                  case property_value_type::none:
                     return "None";
                     //
                  case property_value_type::boolean:
                     return "Bool";
                  case property_value_type::float32:
                     return "Float";
                  case property_value_type::integer:
                     return "Int";
                  case property_value_type::object:
                     return "Form";
                  case property_value_type::string:
                     return "String";
                     //
                  case property_value_type::array_of_boolean:
                     return "Bool[]";
                  case property_value_type::array_of_float32:
                     return "Float[]";
                  case property_value_type::array_of_integer:
                     return "Int[]";
                  case property_value_type::array_of_object:
                     return "Form[]";
                  case property_value_type::array_of_string:
                     return "String[]";
               }
               return {};
            };

            QString out;
            if (item->type.has_value()) {
               auto& typeinfo = item->type.value();
               if (!typeinfo.name.isEmpty()) {
                  return typeinfo.name;
               }
               return _typecode_to_string(typeinfo.underlying_type);
            }
            if (item->bindings.parent.has_value()) {
               return _typecode_to_string(item->bindings.parent.value().typecode()) + "?";
            }
            if (item->bindings.target.has_value()) {
               return _typecode_to_string(item->bindings.target.value().typecode()) + "?";
            }
         }
         break;
      case ColumnValue:
         switch (role) {
            case Qt::DisplayRole:
               return item->value_string;
         }
         break;
   }
   return {};
}
QVariant DKPapyrusScriptObjectModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal) {
      return {};
   }
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnName:  return tr("Name");
            case ColumnType:  return tr("Type");
            case ColumnValue: return tr("Value");
         }
         break;
   }
   return {};
}
inline const DKPapyrusScriptObjectModel::Property* DKPapyrusScriptObjectModel::row(int rowIndex) const noexcept {
   return &this->properties[rowIndex];
}
#pragma endregion

void DKPapyrusScriptObjectModel::clearPropertyValue(int rowIndex) {
   if (rowIndex < 0 || rowIndex >= this->properties.size())
      return;
   auto& item = this->properties[rowIndex];
   {
      auto& v_opt = item.bindings.target;
      if (!v_opt.has_value())
         v_opt.emplace();
      auto& v_bind = v_opt.value();
      //
      v_bind.value  = {};
      v_bind.status = vmad::property_status::inherited_and_removed;
   }
   item.recacheValueString();

   QModelIndex upper_left  = this->index(rowIndex, ColumnName,  {});
   QModelIndex lower_right = this->index(rowIndex, ColumnValue, {});
   emit dataChanged(upper_left, lower_right);
}
void DKPapyrusScriptObjectModel::revertPropertyValue(int rowIndex) {
   if (rowIndex < 0 || rowIndex >= this->properties.size())
      return;
   auto& item = this->properties[rowIndex];
   {
      auto& v_opt = item.bindings.target;
      if (!v_opt.has_value())
         v_opt.emplace();
      auto& v_bind = v_opt.value();
      //
      v_bind.value  = {};
      v_bind.status = vmad::property_status::defined_only_on_base;
   }
   item.recacheValueString();

   QModelIndex upper_left  = this->index(rowIndex, ColumnName,  {});
   QModelIndex lower_right = this->index(rowIndex, ColumnValue, {});
   emit dataChanged(upper_left, lower_right);
}
void DKPapyrusScriptObjectModel::setPropertyValue(int rowIndex, const property_value& data) {
   if (rowIndex < 0 || rowIndex >= this->properties.size())
      return;
   auto& item = this->properties[rowIndex];
   item.changeValueTo(data);

   QModelIndex upper_left  = this->index(rowIndex, ColumnName,  {});
   QModelIndex lower_right = this->index(rowIndex, ColumnValue, {});
   emit dataChanged(upper_left, lower_right);
}