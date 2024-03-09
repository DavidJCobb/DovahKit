#include "./Script.h"
#include "./Property.h"
#include "helpers/type_traits/is_std_vector.h"

#include "dovah/data/papyrus/native_classes.h"
#include "editor/subsystems/papyrus/core.h"

// For loading PEXs:
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/papyrus/compiled_script.h"
#include "editor/subsystems/assets.h"

#include "./name_equals.h"

namespace {
   constexpr const bool show_raw_statuses = false;
}

namespace DKFormVMADModelObjects {
   Script::~Script() {
      for (auto* item : this->properties)
         delete item;
      this->properties.clear();
   }

   void Script::_load_property_definitions_from(std::string_view scriptname) {
      dovah::compiled_papyrus_script data;
      {
         auto& assets = dovahkit::subsystems::assets::get();

         std::filesystem::path path("scripts/");
         path /= std::string(scriptname) + ".pex";

         auto* file = assets.lookup_game_asset(path);
         if (!file) {
            this->load_results.failed = true;
            return;
         }
         try {
            data.read_file(file->data(), file->size());
            delete file;
         } catch (dovah::compiled_papyrus_script::read_exception& e) {
            this->load_results.failed = true;
            delete file;
            return;
         }
      }

      auto& papyrus = dovahkit::subsystems::papyrus::core::get();

      for (const auto& object : data.objects) {
         if (!dovah::papyrus::helpers::name_equals(object.name, scriptname))
            continue;
         for (const auto& prop : object.properties) {
            QString name = QString::fromStdString(prop.name);

            auto* dst_prop = this->lookup_property(name);
            if (!dst_prop) {
               dst_prop = new Property;
               this->properties.push_back(dst_prop);
            }

            dst_prop->name      = name;
            dst_prop->docstring = QString::fromStdString(prop.docstring);

            auto& dst_type = dst_prop->typeinfo;
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
               dst_type.raw_type = vmad::property_type::boolean;
            } else if (type_name == "float") {
               dst_type.raw_type = vmad::property_type::float32;
            } else if (type_name == "int") {
               dst_type.raw_type = vmad::property_type::integer;
            } else if (type_name == "string") {
               dst_type.raw_type = vmad::property_type::string;
            } else {
               dst_type.raw_type = vmad::property_type::object;
            }
            //
            if (dst_type.raw_type == vmad::property_type::object) {
               bool matched = false;
               for (const auto& info : dovah::papyrus::native_classes) {
                  auto info_name = QString::fromLatin1(info.name.data(), info.name.size());
                  if (type_name.compare(info_name, Qt::CaseInsensitive) == 0) {
                     dst_type.object_info.native_type = info.form_type;
                     matched = true;
                     break;
                  }
               }
               if (!matched) {
                  auto* definition = papyrus.lookup_known_script(type_name);
                  if (definition) {
                     dst_type.object_info.definition  = definition;
                     dst_type.object_info.native_type = definition->underlying_type();
                  }
               }
            }
            //
            if (is_array) {
               dst_type.raw_type = vmad::array_property_type_for(dst_type.raw_type);
            }
         }
      }
   }

   void Script::load_property_definitions() {
      if (this->name.isEmpty())
         return;

      auto& papyrus    = dovahkit::subsystems::papyrus::core::get();
      auto* definition = papyrus.lookup_known_script(this->name);
      if (!definition) {
         this->load_results.failed = true;
         return;
      }

      do {
         this->_load_property_definitions_from(definition->name);
         if (this->load_results.failed)
            break;
      } while (definition = definition->superclass());
   }

   std::optional<property_value> Script::_load_property_value(const vmad::property& src, const Property& info) {
      std::optional<property_value> out;
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

               if (info.typeinfo.object_info_available()) {
                  //
                  // We are capable of identifying this property's native base class, and handling 
                  // object-type values accordingly.
                  //
                  if (!info.typeinfo.object_info.native_type.has_value()) {
                     //
                     // If a script has no native base class, then it can't be attached to anything; 
                     // ergo no property with that script as its type can legally have a value.
                     //
                     return {};
                  }
                  switch (info.typeinfo.object_info.native_type.value()) {
                     case dovah::form_type::alias:
                     case dovah::form_type::location_alias:
                     case dovah::form_type::reference_alias:
                        is_alias = true;
                        break;
                  }
               } else {
                  //
                  // We can't identify this property's native base class. We'll have to try and 
                  // guess based on its current value.
                  // 
                  if (is_array) {
                     auto* src_p = std::get_if<std::vector<vmad::property_object_value>>(&src.value);
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
                     is_alias = aliases;
                  } else {
                     auto* src_p = std::get_if<vmad::property_object_value>(&src.value);
                     if (!src_p)
                        return {};
                     auto& src = *src_p;
                     is_alias = src.alias_id != vmad::property_object_value::no_alias;
                  }
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

   void Script::load_parent_property_value(const vmad::property& src) {
      QString prop_name = QString::fromStdString(src.name);

      auto* dst_prop = this->lookup_property(prop_name);
      if (!dst_prop) {
         this->load_results.some_data_discarded = true;
         return;
      }
      property_value value;
      if (src.status != vmad::property_status::inherited_and_removed) {
         if (vmad::property_type_for(src.value) != dst_prop->typeinfo.raw_type) {
            this->load_results.some_data_discarded = true;
            return;
         }
         auto value_opt = this->_load_property_value(src, *dst_prop);
         if (!value_opt.has_value()) {
            this->load_results.some_data_discarded = true;
            return;
         }
         value = value_opt.value();
         if (dst_prop->typeinfo_is_unknown()) {
            if (std::holds_alternative<ui::types::quest_alias>(value) || std::holds_alternative<std::vector<ui::types::quest_alias>>(value))
               dst_prop->typeinfo.object_info.guessed_alias = true;
         }
      }
      dst_prop->bindings.parent = {
         .status = src.status,
         .value  = value,
      };
   }
   void Script::load_target_property_value(const vmad::property& src) {
      QString prop_name = QString::fromStdString(src.name);

      auto* dst_prop = this->lookup_property(prop_name);
      if (!dst_prop) {
         this->load_results.some_data_discarded = true;
         return;
      }
      property_value value;
      if (src.status != vmad::property_status::inherited_and_removed) {
         if (vmad::property_type_for(src.value) != dst_prop->typeinfo.raw_type) {
            this->load_results.some_data_discarded = true;
            return;
         }
         auto value_opt = this->_load_property_value(src, *dst_prop);
         if (!value_opt.has_value()) {
            this->load_results.some_data_discarded = true;
            return;
         }
         if (dst_prop->typeinfo_is_unknown()) {
            //
            // We had to guess this property's value-type (e.g. form versus alias). If 
            // the value on the target is inconsistent with the value on the parent 
            // (and the guess made therein), then discard the value on the target.
            // 
            // NOTE: This will fail to handle a minor edge-case: that of a property 
            //       that was intended to be alias-type, was set to None on the parent 
            //       script, and was set to an alias on the target script. A property 
            //       like that will be mistaken for a form property and the value on 
            //       the target script will be cleared. C'est la vie. We'll only get 
            //       here if the PEX file for the property's value type is missing.
            //
            if (dst_prop->bindings.parent.has_value()) {
               const auto& value_parent = dst_prop->bindings.parent.value().value;
               if (value_opt.value().index() != value_parent.index()) {
                  this->load_results.some_data_discarded = true;
                  return;
               }
            }
         }
         value = value_opt.value();
      }
      dst_prop->bindings.target = {
         .status = src.status,
         .value  = value,
      };
   }

   Property* Script::lookup_property(QString name) {
      for (auto* prop : this->properties)
         if (prop->name_matches(name))
            return prop;
      return nullptr;
   }

   std::optional<vmad::script_status> Script::get_computed_status() const {
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
               case vmad::script_status::removed:
                  return vmad::script_status::removed;
               default:
                  return vmad::script_status::defined_on_base;
            }
         }
         if (t_opt.has_value()) {
            switch (t_opt.value()) {
               case vmad::script_status::removed:
                  return vmad::script_status::removed;
               default:
                  return vmad::script_status::defined_locally;
            }
         }
      }
      return {};
   }
   bool Script::name_matches(QString desired) const {
      return name_equals(this->name, desired);
   }
}