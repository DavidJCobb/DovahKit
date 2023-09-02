#pragma once
#include <optional>
#include <vector>
#include <QObject>
#include "editor/subsystems/options/core.h"
#include "./gizmo_color_scheme.h"

class QXmlStreamReader;

namespace dovahkit::subsystems::worldedit {
   class gizmo_color_scheme_manager;
   class gizmo_color_scheme_manager : public QObject, public cobb::singleton_ex<gizmo_color_scheme_manager>, public dovahkit::subsystems::options::option_collection {
      Q_OBJECT;
      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

         struct color_scheme_id {
            std::string name; // utf-8
            bool is_hardcoded = false; // we try to prevent two schemes from having the same name, but hardcoded ones added in an update could conflict; this disambiguates

            constexpr bool operator==(const color_scheme_id&) const = default;
         };

      protected:
         gizmo_color_scheme_manager();

         std::vector<gizmo_color_scheme> _user_schemes;
         color_scheme_id _current_scheme;

         std::optional<color_scheme_id> _parse_current_id(QXmlStreamReader&);
         void _parse_scheme(QXmlStreamReader&);
         void _parse_file(QXmlStreamReader&);

      public:
         std::vector<gizmo_color_scheme> all_hardcoded_color_schemes() const;
         const std::vector<gizmo_color_scheme>& all_user_color_schemes() const;

         bool scheme_name_is_taken(const std::string&) const;

         gizmo_color_scheme get_current_color_scheme() const;
         color_scheme_id get_current_color_scheme_id() const;
         void set_current_color_scheme_id(const color_scheme_id&);

         // Returns the name used. If the name you specified is taken, a number is appended.
         std::string add_new_color_scheme(const char* name, const gizmo_color_scheme& based_on);

         void replace_all_user_schemes(const std::vector<gizmo_color_scheme>&);

         virtual void reload() override;
         virtual void save() override;

      signals:
         void colorSchemeChanged(const gizmo_color_scheme&);
   };
}