#pragma once
#include <optional>
#include <QString>
#include <QVector>
#include "./types.h"
#include "./property_value.h"

namespace DKFormVMADModelObjects {
   class Property;

   class Script {
      public:
         ~Script();

         QString name;
         struct {
            bool failed              = false; // any PEX files failed to load?
            bool some_data_discarded = false; // true if any properties were defined in VMAD but not in PEX
         } load_results;
         struct {
            std::optional<vmad::script_status> parent;
            std::optional<vmad::script_status> target;
         } statuses;
         bool properties_set_on_target = false;
         QVector<Property*> properties;

      protected:
         void _load_property_definitions_from(std::string_view scriptname);

         // Returns empty on failure.
         std::optional<property_value> _load_property_value(const vmad::property&, const Property& info);

      public:
         void load_property_definitions(); // i.e. from the PEX

         void load_parent_property_value(const vmad::property&);
         void load_target_property_value(const vmad::property&);

         Property* lookup_property(QString name);

         std::optional<vmad::script_status> get_computed_status() const;
         bool name_matches(QString) const;
   };
}