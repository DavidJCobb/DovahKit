#include "./property.h"
#include <cassert>

// for auto-fill
#include "dovah/data/papyrus/helpers/name_equals.h"
#include "editor/core.h"

#include "dovah/data/papyrus/native_classes.h"

namespace ui::bound_script_models {
   bool property::refers_to_form(const dovah::form_stub& stub) const {
      auto _check = [](const dovah::form_stub& stub, const std::optional<property_value>& value_opt) -> bool {
         if (!value_opt.has_value())
            return false;
         auto& value = value_opt.value();
         if (auto* casted = std::get_if<dovah::form_stub*>(&value)) {
            return *casted == &stub;
         } else if (auto* casted = std::get_if<std::vector<dovah::form_stub*>>(&value)) {
            for (auto* item : *casted)
               if (item == &stub)
                  return true;
            return false;
         } else if (auto* casted = std::get_if<ui::types::quest_alias>(&value)) {
            return casted->quest == &stub;
         } else if (auto* casted = std::get_if<std::vector<ui::types::quest_alias>>(&value)) {
            for (const auto& item : *casted)
               if (item.quest == &stub)
                  return true;
            return false;
         }
         return false;
      };

      if (_check(stub, this->values.inherited))
         return true;
      if (_check(stub, this->values.local))
         return true;

      return false;
   }
   QString property::type_string() const {
      if (!this->is_object_or_array_thereof()) {
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
      if (this->typeinfo.object.native_type.has_value() && !this->typeinfo.object.definition) {
         //
         // It's a native class. Prefer the "canonical" spelling.
         //
         for (const auto& info : dovah::papyrus::native_classes)
            if (info.form_type == this->typeinfo.object.native_type.value())
               return QString::fromLatin1(info.name.data(), info.name.size());
      }
      return this->typeinfo.object.scriptname;
   }

   void property::recache_value_string() {
      if (auto& value_opt = this->values.local; value_opt.has_value()) {
         auto& value = value_opt.value();
         if (std::holds_alternative<std::monostate>(value)) {
            this->cached.value_string = "<<Default>>";
            return;
         }
         this->cached.value_string = stringify(value);
         return;
      }
      if (auto& value_opt = this->values.inherited; value_opt.has_value()) {
         this->cached.value_string = stringify(value_opt.value());
         return;
      }
      this->cached.value_string = "<<Default>>";
   }

   bool property::on_form_deletion_imminent(const dovah::form_stub& stub) {
      bool any_changed = false;

      auto _check = [](const dovah::form_stub& stub, std::optional<property_value>& value_opt) -> bool {
         if (!value_opt.has_value())
            return false;
         auto& value = value_opt.value();
         if (auto* casted = std::get_if<dovah::form_stub*>(&value)) {
            if (*casted == &stub) {
               *casted = nullptr;
               return true;
            }
            return false;
         } else if (auto* casted = std::get_if<std::vector<dovah::form_stub*>>(&value)) {
            bool seen = false;
            for (auto*& item : *casted) {
               if (item == &stub) {
                  item = nullptr;
                  seen = true;
               }
            }
            return seen;
         } else if (auto* casted = std::get_if<ui::types::quest_alias>(&value)) {
            if (casted->quest == &stub) {
               *casted = {};
               return true;
            }
            return false;
         } else if (auto* casted = std::get_if<std::vector<ui::types::quest_alias>>(&value)) {
            bool seen = false;
            for (auto& item : *casted) {
               if (item.quest == &stub) {
                  item = {};
                  seen = true;
               }
            }
            return seen;
         }
         return false;
      };

      any_changed |= _check(stub, this->values.inherited);
      //
      // If we're deleting the form referenced by the parent BoundScript, then DovahKit should 
      // update that BoundScript during the deletion process, to clear the form out. We should 
      // thus clear it out in our copy of that BoundScript's property data. The way we clear it 
      // should be made to match the backend, i.e. if we update the backend to straight-up 
      // delete properties that pointed to deleted forms, then we should do the same here.

      any_changed |= _check(stub, this->values.local);

      return any_changed;
   }


   bool property::clear() {
      bool changed = false;
      if (this->is_inherited()) {
         changed = !this->values.local.has_value() || !std::holds_alternative<std::monostate>(this->values.local.value());
         this->values.local = std::monostate{};
      } else {
         changed = this->values.local.has_value();
         this->values.local.reset();
      }
      return changed;
   }

   // "Edit Value" button in the UI.
   // Only available if the property has no local or inherited value. This creates a new 
   // local value, with a default falsy/empty value.
   void property::make_local() {
      switch (this->typeinfo.raw_type) {
         case vmad::property_type::none:
            this->values.local = std::monostate{};
            return;

         case vmad::property_type::boolean:
            this->values.local = false;
            return;
         case vmad::property_type::float32:
            this->values.local = 0.0F;
            return;
         case vmad::property_type::integer:
            this->values.local = 0;
            return;
         case vmad::property_type::string:
            this->values.local = QString();
            return;

         case vmad::property_type::array_of_boolean:
            this->values.local.emplace().emplace<std::vector<bool>>();
            return;
         case vmad::property_type::array_of_float32:
            this->values.local.emplace().emplace<std::vector<float>>();
            return;
         case vmad::property_type::array_of_integer:
            this->values.local.emplace().emplace<std::vector<int32_t>>();
            return;
         case vmad::property_type::array_of_string:
            this->values.local.emplace().emplace<std::vector<QString>>();
            return;
      }

      bool is_alias = this->is_quest_alias_or_array_thereof();

      if (this->typeinfo.raw_type == vmad::property_type::object) {
         auto& dst = this->values.local.emplace();
         if (is_alias) {
            dst.emplace<ui::types::quest_alias>();
         } else {
            dst = (dovah::form_stub*)nullptr;
         }
      } else if (this->typeinfo.raw_type == vmad::property_type::array_of_object) {
         auto& dst = this->values.local.emplace();
         if (is_alias) {
            dst.emplace<std::vector<ui::types::quest_alias>>();
         } else {
            dst.emplace<std::vector<dovah::form_stub*>>();
         }
      }
   }

   void property::set_local_value(const property_value& src) {
      assert(!std::holds_alternative<std::monostate>(src)      && "Use `clear()` directly; `std::monostate` working like this is an implementation detail, not a promise.");
      assert(property_type_for(src) == this->typeinfo.raw_type && "The property value you pass in must be of a valid type for this particular property.");

      this->values.local = src;
   }

   void property::revert() {
      if (!this->is_inherited())
         return;
      this->values.local.reset();
   }

   bool property::autofill() {
      if (!this->is_form_or_array_thereof())
         return false;

      dovah::form_type  underlying_type = this->typeinfo.object.native_type.value_or(dovah::form_type::none);
      dovah::form_stub* found = nullptr;

      std::string name = this->name.toUtf8().toStdString();

      auto& editor = DovahKitCore::get();
      if (underlying_type == dovah::form_type::none) {
         editor.for_each_form([this, &name, &found](dovah::form_stub* stub) -> bool {
            if (dovah::papyrus::helpers::name_equals(name, stub->editorID)) {
               found = stub;
               return true;
            }
            return false;
         });
      } else {
         editor.for_each_form_of_type(underlying_type, [this, &name, &found](dovah::form_stub* stub) -> bool {
            if (dovah::papyrus::helpers::name_equals(name, stub->editorID)) {
               found = stub;
               return true;
            }
            return false;
         });
      }

      if (!found)
         return false;

      this->values.local = found;
      return true;
   }
}