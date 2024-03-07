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
      using object_property_value = DKFormVMADModel::object_property_value;
      using property_value        = DKFormVMADModel::property_value;
   }
   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
      using object_property_value = property_object_value;
   }
}

// Helpers
namespace {
   bool _papyrus_name_matches(QString a, QString b) {
      //
      // This is not a straightforward QString::toLower check because Bethesda's string table is 
      // only case-insensitive within the C locale.
      //
      size_t size = a.size();
      if (size != b.size())
         return false;
      for (size_t i = 0; i < size; ++i) {
         auto ac = a[(uint)i].unicode(); // it's very annoying that QString was implemented in such a manner as to make this cast necessary.
         auto bc = b[(uint)i].unicode();
         if (ac == bc)
            continue;
         if (ac >= 'a' && ac <= 'z')
            ac -= 0x20;
         if (bc >= 'a' && bc <= 'z')
            bc -= 0x20;
         if (ac != bc)
            return false;
      }
      return true;
   }
}

#pragma region Property
namespace {
   namespace _properties {
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

      #pragma region model::T convert_value(const vmad::T&) and vice versa
      model::property_value convert_value(const vmad::property_value& v) {
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

      void convert_value(const model::property_value& src_v, vmad::property_value& dst_v, dovah::loaded_forms::Form& dst_form) {
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
      #pragma endregion

      #pragma region QString stringify_value(const T&)
      QString stringify_value(bool v) {
         return v ? "True" : "False";
      }
      QString stringify_value(float v) {
         return QString::number(v);
      }
      QString stringify_value(int32_t v) {
         return QString::number(v);
      }
      QString stringify_value(const model::object_property_value& v) {
         auto form_str = editor_helpers::form_identifiers_to_string(v.form);
         if (v.alias_id != model::object_property_value::no_alias) {
            static_assert(allow_incomplete_polishing, "POLISH: Look up QUST form, load it if possible, and display alias's actual name.");
            return QString("Quest alias ID#%1 on %2")
               .arg(v.alias_id) // TODO: Look up quest, load it if present, and display alias name.
               .arg(form_str);
         }
         static_assert(allow_incomplete_polishing, "POLISH: If the value is a REFR, append stringified info about its containing CELL/WRLD.");
         return form_str;
      }
      QString stringify_value(QString v) {
         return v;
      }
      //
      QString stringify_value(const model::property_value& value) {
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
                     out += stringify_value(v[i]);
                     if (i + 1 < size)
                        out += ", ";
                  }
                  out += " ]";
               } else {
                  out = stringify_value(v);
               }
            },
            value
         );
         return out;
      }
      #pragma endregion
   }
}

vmad::property_type DKFormVMADModel::Property::Binding::typecode() const {
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

void DKFormVMADModel::Property::clearParentBinding() {
   this->bindings.parent = {};
   this->recacheValueString();
}
void DKFormVMADModel::Property::clearTargetBinding() {
   this->bindings.target = {};
   this->recacheValueString();
}
void DKFormVMADModel::Property::setParentBinding(const vmad_property& src) {
   this->bindings.parent = {
      .status = src.status,
      .value  = _properties::convert_value(src.value)
   };
   this->recacheValueString();
}
void DKFormVMADModel::Property::setTargetBinding(const vmad_property& src) {
   this->bindings.target = {
      .status = src.status,
      .value  = _properties::convert_value(src.value)
   };
   this->recacheValueString();
}
void DKFormVMADModel::Property::setBindings(const vmad_property& parent, const vmad_property& target) {
   this->bindings.parent = {
      .status = parent.status,
      .value  = _properties::convert_value(parent.value)
   };
   this->bindings.target = {
      .status = target.status,
      .value  = _properties::convert_value(target.value)
   };
   this->recacheValueString();
}

bool DKFormVMADModel::Property::valueTypeIsOrContainsForm() const {
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
bool DKFormVMADModel::Property::isOrContainsForm(const dovah::form_stub& stub) const {
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
QString DKFormVMADModel::Property::typeString() const {
   std::optional<property_type> underlying_type;

   if (this->type.has_value()) {
      auto& typeinfo = this->type.value();
      if (!typeinfo.name.isEmpty()) {
         return typeinfo.name;
      }
      underlying_type = typeinfo.underlying_type;
   }
   if (!underlying_type.has_value()) {
      if (this->bindings.parent.has_value()) {
         underlying_type = this->bindings.parent.value().typecode();
      }
      if (this->bindings.target.has_value()) {
         underlying_type = this->bindings.target.value().typecode();
      }
   }

   QString out;
   if (underlying_type.has_value()) {
      switch (underlying_type.value()) {
         case property_type::none:
            out = "None";
            break;
            //
         case property_type::boolean:
            out = "Bool";
            break;
         case property_type::float32:
            out = "Float";
            break;
         case property_type::integer:
            out = "Int";
            break;
         case property_type::object:
            out = "Form";
            break;
         case property_type::string:
            out = "String";
            break;
            //
         case property_type::array_of_boolean:
            out = "Bool[]";
            break;
         case property_type::array_of_float32:
            out = "Float[]";
            break;
         case property_type::array_of_integer:
            out = "Int[]";
            break;
         case property_type::array_of_object:
            out = "Form[]";
            break;
         case property_type::array_of_string:
            out = "String[]";
            break;

         default:
            return {};
      }
   } else {
      return {};
   }
   out += "?";
   return out;
}

void DKFormVMADModel::Property::recacheValueString() {
   if (auto& bind_opt = this->bindings.target; bind_opt.has_value()) {
      auto& bind = bind_opt.value();
      switch (bind.status) {
         case property_status::unknown:
         case property_status::defined_locally:
            this->value_string = _properties::stringify_value(bind.value);
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
            this->value_string = _properties::stringify_value(bind.value);
            return;
      }
   }
   this->value_string = "<<Default>>";
}
bool DKFormVMADModel::Property::onFormDeletionImminent(const dovah::form_stub& stub) {
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

void DKFormVMADModel::Property::changeValueTo(const property_value& src) {
   this->bindings.target = {
      .status = property_status::defined_locally,
      .value  = src,
   };
   this->recacheValueString();
}

std::optional<vmad::property_status> DKFormVMADModel::Property::getComputedStatus() const {
   if (this->bindings.target.has_value()) {
      return this->bindings.target.value().status;
   }
   if (this->bindings.parent.has_value()) {
      return property_status::defined_only_on_base;
   }
   return {};
}
bool DKFormVMADModel::Property::nameMatches(QString s) const {
   return _papyrus_name_matches(s, this->name);
}
#pragma endregion

#pragma region Script
DKFormVMADModel::Script::~Script() {
   for (auto* item : this->properties) {
      assert(item);
      delete item;
   }
   this->properties.clear();
}

void DKFormVMADModel::Script::loadPropertiesFromPex() {
   if (this->name.isEmpty())
      return;

   std::string scriptname = this->name.toStdString();

   dovah::compiled_papyrus_script data;
   {
      auto& assets = dovahkit::subsystems::assets::get();

      std::filesystem::path path("scripts/");
      path /= scriptname + ".pex";

      auto* file = assets.lookup_game_asset(path);
      if (!file)
         return;
      try {
         data.read_file(file->data(), file->size());
         delete file;
      } catch (dovah::compiled_papyrus_script::read_exception& e) {
         delete file;
         return;
      }
   }

   for (const auto& object : data.objects) {
      if (!dovah::papyrus::helpers::name_equals(object.name, scriptname))
         continue;
      for (const auto& prop : object.properties) {
         QString name = QString::fromStdString(prop.name);

         auto* dst_prop = this->lookupProperty(name);
         if (!dst_prop) {
            dst_prop = new Property;
            this->properties.push_back(dst_prop);
         }

         dst_prop->name      = name;
         dst_prop->docstring = QString::fromStdString(prop.docstring);

         if (!dst_prop->type.has_value())
            dst_prop->type.emplace();
         auto& dst_type = dst_prop->type.value();
         dst_type.name = QString::fromStdString(prop.type);

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
void DKFormVMADModel::Script::loadParentPropertyData(const vmad_property& src) {
   QString prop_name = QString::fromStdString(src.name);
   
   auto* dst_prop = this->lookupProperty(prop_name);
   if (!dst_prop) {
      dst_prop = new Property;
      this->properties.push_back(dst_prop);
      dst_prop->name = prop_name;
   }
   dst_prop->bindings.parent = {
      .status = src.status,
      .value  = _properties::convert_value(src.value),
   };
}
void DKFormVMADModel::Script::loadTargetPropertyData(const vmad_property& src) {
   QString prop_name = QString::fromStdString(src.name);

   auto* dst_prop = this->lookupProperty(prop_name);
   if (!dst_prop) {
      dst_prop = new Property;
      this->properties.push_back(dst_prop);
      dst_prop->name = prop_name;
   }
   dst_prop->bindings.target = {
      .status = src.status,
      .value  = _properties::convert_value(src.value),
   };
   
   if (src.status != vmad::property_status::defined_only_on_base) {
      this->properties_set_on_target = true;
   }
}

DKFormVMADModel::Property* DKFormVMADModel::Script::lookupProperty(QString name) {
   for (auto* prop : this->properties)
      if (prop->nameMatches(name))
         return prop;
   return nullptr;
}

std::optional<vmad::script_status> DKFormVMADModel::Script::getComputedStatus() const {
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
bool DKFormVMADModel::Script::nameMatches(QString s) const {
   return _papyrus_name_matches(s, this->name);
}
#pragma endregion

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
         if (prop->onFormDeletionImminent(*stub)) {
            changed = true;
         }
         //
         if (changed) {
            prop->recacheValueString();

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
            if (script->nameMatches(scriptname)) {
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
            dst->loadPropertiesFromPex();
         }
         dst->statuses.parent = src.status;

         dst->properties.reserve(src.properties.size());
         for (const auto& prop : src.properties) {
            //dst->loadParentPropertyData(prop); // need to be able to call begin-/endInsertRows, so can't use this
            
            QString prop_name = QString::fromStdString(src.name);

            auto* dst_prop        = dst->lookupProperty(prop_name);
            bool  is_new_property = !dst_prop;
            if (is_new_property) {
               if (!is_new_script) {
                  this->beginInsertRows({}, dst->properties.size(), dst->properties.size());
               }
               dst_prop = new Property;
               dst->properties.push_back(dst_prop);
               dst_prop->name = prop_name;
            }
            dst_prop->bindings.parent = {
               .status = prop.status,
               .value  = _properties::convert_value(prop.value),
            };
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

         if (prop->isOrContainsForm(*stub)) {
            prop->recacheValueString();
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

         if (prop->isOrContainsForm(*stub)) {
            prop->recacheValueString();
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
         if (prop->valueTypeIsOrContainsForm()) {
            prop->recacheValueString();
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
         ptr->loadPropertiesFromPex();
      }
      ptr->statuses.target = src.status;

      ptr->properties.reserve(src.properties.size());
      for (const auto& prop : src.properties) {
         ptr->loadTargetPropertyData(prop);
      }
   }
   //
   this->scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->scripts.push_back(it->second);
      for (auto* prop : it->second->properties)
         prop->recacheValueString();
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
         ptr->loadPropertiesFromPex();
      }
      ptr->statuses.parent = src.status;

      ptr->properties.reserve(src.properties.size());
      for (const auto& prop : src.properties) {
         ptr->loadParentPropertyData(prop);
      }
   }
   for (const auto& src : target.scripts) {
      QString name       = QString::fromStdString(src.name);
      QString name_lower = name.toLower();

      auto& ptr = working[name_lower];
      if (!ptr) {
         ptr = new Script;
         ptr->name = name;
         ptr->loadPropertiesFromPex();
      }
      ptr->statuses.target = src.status;

      ptr->properties.reserve(ptr->properties.size() + src.properties.size());
      for (const auto& prop : src.properties) {
         ptr->loadTargetPropertyData(prop);
      }
      ptr->properties.shrink_to_fit();
   }
   //
   this->scripts.reserve(working.size());
   for (auto it = working.keyValueBegin(); it != working.keyValueEnd(); ++it) {
      this->scripts.push_back(it->second);
      for (auto* prop : it->second->properties)
         prop->recacheValueString();
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
                  _properties::convert_value(target_bind.value, dst_prop.value, *loaded);
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
                  auto status_opt = script->getComputedStatus();
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
                  auto status_opt = script->getComputedStatus();
                  if (!status_opt.has_value()) {
                     return; // Call QTableView::setIconSize to ensure there's always space reserved for icons.
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

                        auto status_opt = prop->getComputedStatus();
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
                        auto status_opt = prop->getComputedStatus();
                        if (!status_opt.has_value()) {
                           return; // Call QTableView::setIconSize to ensure there's always space reserved for icons.
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
                  out = prop->typeString();
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
      .inherited_and_removed = is_on_parent && is_on_target && script->getComputedStatus() != script_status::removed,
   };
}
QModelIndex DKFormVMADModel::scriptIndex(QString scriptname) const {
   size_t size = this->scripts.size();
   for (size_t i = 0; i < size; ++i) {
      const auto* item = this->scripts[i];
      if (item->nameMatches(scriptname))
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
      if (prop->nameMatches(name))
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
               prop->clearTargetBinding();
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
         prop->clearTargetBinding();
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
   prop->recacheValueString();

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
   prop->recacheValueString();

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
   prop->changeValueTo(data);
   
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
   return prop->getComputedStatus();
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

   if (prop->type.has_value()) {
      switch (prop->type.value().underlying_type) {
         case vmad::property_type::boolean:
            return false;
         case vmad::property_type::float32:
            return 0.0F;
         case vmad::property_type::integer:
            return 0;
         case vmad::property_type::object:
            return model::object_property_value{};
         case vmad::property_type::string:
            return QString{};

         case vmad::property_type::array_of_boolean:
            return std::vector<bool>{};
         case vmad::property_type::array_of_float32:
            return std::vector<float>{};
         case vmad::property_type::array_of_integer:
            return std::vector<int32_t>{};
         case vmad::property_type::array_of_object:
            return std::vector<model::object_property_value>{};
         case vmad::property_type::array_of_string:
            return std::vector<QString>{};
      }
   }

   return {};
}
void DKFormVMADModel::clearPropertyWorkingValue(QModelIndex qmi) {
   auto* script = this->_getContainingScript(qmi);
   if (!script)
      return;
   auto* prop = (Property*)qmi.internalPointer();
   assert(prop != nullptr);
   auto& dst = prop->bindings.edited.emplace();
   dst.status = property_status::inherited_and_removed;
   dst.value  = {};
   prop->recacheValueString();

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
   prop->recacheValueString();

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
   auto& dst = prop->bindings.edited.emplace();
   dst.status = property_status::defined_locally;
   dst.value  = data;
   prop->recacheValueString();
   
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
      prop->bindings.target = prop->bindings.edited;
      prop->bindings.edited.reset();
      prop->recacheValueString();
      
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
                  out = _properties::stringify_value(v[array_index]);
            } else {
               out = _properties::stringify_value(v);
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

   if (prop->bindings.edited.has_value()) {
      out.status = prop->bindings.edited.value().status;
   } else {
      out.status = prop->getComputedStatus();
   }

   if (prop->type.has_value()) {
      //out.typeinfo.underlying = prop->type.value().underlying_type; // TODO
      out.typeinfo.scriptname = prop->type.value().name;
   }
   out.typeinfo.display_typename = prop->typeString();

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