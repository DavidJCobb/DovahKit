#pragma once
#include <optional>
#include <QString>
#include "./types.h"
#include "./property_value.h"

namespace dovahkit::subsystems::papyrus {
   class known_script;
}
namespace DKFormVMADModelObjects {
   using known_script = dovahkit::subsystems::papyrus::known_script;
}

namespace DKFormVMADModelObjects {
   class Property {
      public:
         struct Binding {
            vmad::property_status status;
            property_value        value;
         };

         struct Typeinfo {
            QString name;
            vmad::property_type raw_type = vmad::property_type::none;
            struct {
               const known_script* definition = nullptr;
               std::optional<dovah::form_type> native_type;
               //
               // If the script is loadable but has no native base class, then `definition` is non-null 
               // and `native_type` is empty. If the script isn't loadable, then `definition` is null 
               // and `native_type` is empty. If the script is a native class being used directly, then 
               // `definition` is null and `native_type` is non-empty.
               //
               bool guessed_alias = false; // If not loadable, and if we guess it's an alias, then `true`
            } object_info;

            constexpr bool object_info_available() const {
               return this->object_info.definition != nullptr || this->object_info.native_type.has_value();
            }
         };

      public:
         QString  name;
         QString  docstring;
         Typeinfo typeinfo;
         struct {
            std::optional<Binding> parent; // base form, if the form we're currently editing is a REFR
            std::optional<Binding> target; // form we're currently editing. NOTE: should have a value if clearing an inherited property value REFR-side!

            std::optional<Binding> edited; // "working" value, for edits made to a bound script via the GUI, prior to them being committed ("OK") or discarded ("Cancel")
         } bindings;
         //
         QString value_string; // cached; computed from `bindings`

         constexpr bool typeinfo_is_unknown() const {
            switch (this->typeinfo.raw_type) {
               case vmad::property_type::object:
               case vmad::property_type::array_of_object:
                  return !this->typeinfo.object_info_available();
            }
            return false;
         }

         bool is_quest_alias_or_array_thereof() const;

         property_value make_empty_value() const;

         bool is_object_or_object_array() const;
         bool refers_to_form(const dovah::form_stub&) const; // true if the property is a form [array], or if it's a quest alias [array] and you pass in the quest's stub
         QString type_string() const;
         void recache_value_string();
         bool on_form_deletion_imminent(const dovah::form_stub&); // returns true if anything about this property has changed

         // For CK-consistent behavior, remove inherited scripts from a form by setting their status 
         // to removed and calling this function on all of their properties. That will cause loss of 
         // their non-inherited property values even if the removal is undone by the user.  (You may 
         // instead want to avoid calling this, so the values are recoverable.)
         void erase_non_inherited_value();

         // Use when the user wants to clear this property's value.
         void clear_non_inherited_value();

         // Use when the user wants to set this property's value. Marks the property as edited locally. 
         // NOTE: Asserts that the passed-in value is more-or-less of the same raw type as ourselves.
         void set_non_inherited_value(const property_value&, bool bypass_working = false);

         std::optional<vmad::property_status> get_computed_status() const;
         bool name_matches(QString) const;
   };
}
