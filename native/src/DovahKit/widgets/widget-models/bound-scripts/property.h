#pragma once
#include <optional>
#include <QString>
#include "dovah/form_types.h"

#include "dovah/forms/components/papyrus/property_status.h"
#include "dovah/forms/components/papyrus/property_type.h"

#include "./_forward_declarations.h"
#include "./property_value.h"

namespace ui::bound_script_models {
   class property {
      public:
         QString name;
         QString docstring;
         struct {
            vmad::property_type raw_type = vmad::property_type::none;
            struct {
               const known_script* definition = nullptr;
               QString             scriptname; // empty if it's a native script e.g. ObjectReference or Actor
               std::optional<dovah::form_type> native_type;
               std::optional<dovah::form_type> guessed_type; // if `scriptname` can't be loaded, then this is us guessing whether it's a form or an alias
            } object;
         } typeinfo;
         struct {
            std::optional<property_value> inherited; // empty if never set
            std::optional<property_value> local;     // empty if never set; std::monostate if cleared (i.e. clearing inherited value)
         } values;
         //
         struct {
            QString value_string;
         } cached;

      public:
         constexpr std::optional<vmad::property_status> get_computed_status() const;
         constexpr bool is_inherited() const;

         constexpr bool is_form_or_array_thereof() const;
         constexpr bool is_object_or_array_thereof() const;
         constexpr bool is_quest_alias_or_array_thereof() const;

         constexpr bool typeinfo_is_unknown() const {
            switch (this->typeinfo.raw_type) {
               case vmad::property_type::object:
               case vmad::property_type::array_of_object:
                  if (this->typeinfo.object.definition != nullptr)
                     return false;
                  if (this->typeinfo.object.native_type.has_value())
                     return false;
                  return true;
            }
            return false;
         }

         bool refers_to_form(const dovah::form_stub&) const;
         QString type_string() const;

         void recache_value_string();

         bool on_form_deletion_imminent(const dovah::form_stub&); // returns true if anything about this property has changed

         #pragma region actions
         // "Clear Value" button in the UI.
         // If the property has a local or inherited value, then forcibly clears it, such that 
         // its value will be the PEX-level default (if one exists) or None.
         //
         // Returns `true` if any change happens.
         bool clear();

         // "Edit Value" button in the UI.
         // Only available if the property has no local or inherited value. This creates a new 
         // local value, with a default falsy/empty value.
         void make_local();

         void set_local_value(const property_value&);

         // "Revert" button in the UI.
         // Only available if the property is inherited and has a local value. This clears the 
         // local value, such that the inherited value is the effective value.
         void revert();

         // "Auto-Fill" button in the UI (or "Auto-Fill All").
         // Only available if the property is a non-array form property. This sets the local 
         // value to the form whose editor ID is an ASCII-case-insensitive match for the 
         // property name. If no such form exists, then this has no effect.
         //
         // Returns `true` if the local value changes.
         bool autofill();
         #pragma endregion
   };
}

#include "./property.inl"