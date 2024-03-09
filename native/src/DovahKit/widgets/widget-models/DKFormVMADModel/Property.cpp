#include "./Property.h"
#include <cassert>
#include "dovah/data/papyrus/native_classes.h"
#include "./name_equals.h"

namespace DKFormVMADModelObjects {
   bool Property::is_quest_alias_or_array_thereof() const {
      if (!this->typeinfo.object_info_available()) {
         return this->typeinfo.object_info.guessed_alias;
      }
      auto& nt = this->typeinfo.object_info.native_type;
      if (!nt.has_value())
         return false;
      switch (nt.value()) {
         case dovah::form_type::alias:
         case dovah::form_type::location_alias:
         case dovah::form_type::reference_alias:
            return true;
      }
      return false;
   }

   property_value Property::make_empty_value() const {
      switch (this->typeinfo.raw_type) {
         case vmad::property_type::boolean:
            return false;
         case vmad::property_type::float32:
            return 0.0F;
         case vmad::property_type::integer:
            return 0;
         case vmad::property_type::string:
            return QString();

         case vmad::property_type::array_of_boolean:
            return std::vector<bool>{};
         case vmad::property_type::array_of_float32:
            return std::vector<float>{};
         case vmad::property_type::array_of_integer:
            return std::vector<int32_t>{};
         case vmad::property_type::array_of_string:
            return std::vector<QString>{};
      }

      bool is_alias = false;
      if (this->typeinfo.object_info_available()) {
         auto& nt = this->typeinfo.object_info.native_type;
         if (!nt.has_value())
            return {};
         switch (nt.value()) {
            case dovah::form_type::alias:
            case dovah::form_type::location_alias:
            case dovah::form_type::reference_alias:
               is_alias = true;
               break;
         }
      } else {
         is_alias = this->typeinfo.object_info.guessed_alias;
      }

      if (this->typeinfo.raw_type == vmad::property_type::array_of_object) {
         if (is_alias) {
            return std::vector<ui::types::quest_alias>{};
         }
         return std::vector<dovah::form_stub*>{};
      } else {
         if (is_alias) {
            return ui::types::quest_alias{};
         }
         return (dovah::form_stub*)nullptr;
      }

      return {};
   }

   bool Property::is_object_or_object_array() const {
      switch (this->typeinfo.raw_type) {
         case vmad::property_type::object:
         case vmad::property_type::array_of_object:
            return true;
      }
      return false;
   }
   bool Property::refers_to_form(const dovah::form_stub& stub) const {
      auto _check = [](const dovah::form_stub& stub, const std::optional<Binding>& bind_opt) -> bool {
         if (!bind_opt.has_value())
            return false;
         auto& bind = bind_opt.value();
         if (auto* casted = std::get_if<dovah::form_stub*>(&bind.value)) {
            return *casted == &stub;
         } else if (auto* casted = std::get_if<std::vector<dovah::form_stub*>>(&bind.value)) {
            for (auto* item : *casted)
               if (item == &stub)
                  return true;
            return false;
         } else if (auto* casted = std::get_if<ui::types::quest_alias>(&bind.value)) {
            return casted->quest == &stub;
         } else if (auto* casted = std::get_if<std::vector<ui::types::quest_alias>>(&bind.value)) {
            for (const auto& item : *casted)
               if (item.quest == &stub)
                  return true;
            return false;
         }
         return false;
      };

      if (_check(stub, this->bindings.parent))
         return true;
      if (_check(stub, this->bindings.target))
         return true;
      if (_check(stub, this->bindings.edited))
         return true;

      return false;
   }
   QString Property::type_string() const {
      if (!this->is_object_or_object_array()) {
         QString out;
         auto    scalar_type = vmad::scalar_property_type_for(this->typeinfo.raw_type);
         switch (scalar_type) {
            case vmad::property_type::none:
               out = "None";
               break;
            case vmad::property_type::boolean:
               out = "Bool";
               break;
            case vmad::property_type::float32:
               out = "Float";
               break;
            case vmad::property_type::integer:
               out = "Int";
               break;
            case vmad::property_type::string:
               out = "String";
               break;
            default:
               out = "?";
               break;
         }
         if (this->typeinfo.raw_type != scalar_type)
            out += "[]";
         return out;
      }
      //
      // It's an object type.
      //
      if (this->typeinfo.object_info.native_type.has_value() && !this->typeinfo.object_info.definition) {
         //
         // It's a native class. Prefer the "canonical" spelling.
         //
         for (const auto& info : dovah::papyrus::native_classes)
            if (info.form_type == this->typeinfo.object_info.native_type.value())
               return QString::fromLatin1(info.name.data(), info.name.size());
      }
      return this->typeinfo.name;
   }
   void Property::recache_value_string() {
      if (auto& bind_opt = this->bindings.edited; bind_opt.has_value()) {
         auto& bind = bind_opt.value();
         switch (bind.status) {
            case vmad::property_status::unknown:
            case vmad::property_status::defined_locally:
               this->value_string = stringify(bind.value);
               return;
            case vmad::property_status::inherited_and_removed:
               this->value_string = "None";
               return;
         }
      }
      if (auto& bind_opt = this->bindings.target; bind_opt.has_value()) {
         auto& bind = bind_opt.value();
         switch (bind.status) {
            case vmad::property_status::unknown:
            case vmad::property_status::defined_locally:
               this->value_string = stringify(bind.value);
               return;
            case vmad::property_status::inherited_and_removed:
               this->value_string = "None";
               return;
         }
      }
      if (auto& bind_opt = this->bindings.parent; bind_opt.has_value()) {
         auto& bind = bind_opt.value();
         switch (bind.status) {
            case vmad::property_status::defined_locally:
            case vmad::property_status::defined_only_on_base:
               this->value_string = stringify(bind.value);
               return;
         }
      }
      this->value_string = "<<Default>>";
   }
   bool Property::on_form_deletion_imminent(const dovah::form_stub& stub) {
      bool any_changed = false;

      auto _check = [](const dovah::form_stub& stub, std::optional<Binding>& bind_opt) -> bool {
         if (!bind_opt.has_value())
            return false;
         auto& bind = bind_opt.value();
         if (auto* casted = std::get_if<dovah::form_stub*>(&bind.value)) {
            if (*casted == &stub) {
               *casted = nullptr;
               return true;
            }
            return false;
         } else if (auto* casted = std::get_if<std::vector<dovah::form_stub*>>(&bind.value)) {
            bool seen = false;
            for (auto*& item : *casted) {
               if (item == &stub) {
                  item = nullptr;
                  seen = true;
               }
            }
            return seen;
         } else if (auto* casted = std::get_if<ui::types::quest_alias>(&bind.value)) {
            if (casted->quest == &stub) {
               casted->quest = nullptr;
               return true;
            }
            return false;
         } else if (auto* casted = std::get_if<std::vector<ui::types::quest_alias>>(&bind.value)) {
            bool seen = false;
            for (auto& item : *casted) {
               if (item.quest == &stub) {
                  item.quest = nullptr;
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
      any_changed |= _check(stub, this->bindings.edited);

      return any_changed;
   }

   void Property::erase_non_inherited_value() {
      this->bindings.edited = {};
      this->bindings.target = {};
      this->recache_value_string();
   }

   void Property::clear_non_inherited_value() {
      this->bindings.edited = {
         .status = vmad::property_status::inherited_and_removed,
      };
      this->recache_value_string();
   }

   void Property::set_non_inherited_value(const property_value& value, bool bypass_working) {
      assert(property_type_for(value) == this->typeinfo.raw_type);

      auto& binding = bypass_working ? this->bindings.target : this->bindings.edited;
      binding = {
         .status = vmad::property_status::defined_locally,
         .value  = value,
      };
      this->recache_value_string();
   }

   std::optional<vmad::property_status> Property::get_computed_status() const {
      if (this->bindings.target.has_value()) {
         return this->bindings.target.value().status;
      }
      if (this->bindings.parent.has_value()) {
         return vmad::property_status::defined_only_on_base;
      }
      return {};
   }
   bool Property::name_matches(QString desired) const {
      return name_equals(this->name, desired);
   }
}