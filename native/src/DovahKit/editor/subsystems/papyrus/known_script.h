#pragma once
#include <optional>
#include <QString>
#include <QVector>
#include "helpers/passkey.h"
#include "dovah/form_types.h"

namespace dovahkit::subsystems::papyrus {
   class core;

   class known_script {
      public:
         struct per_file_info {
            QString docstring;
            struct {
               QString name;
               known_script* target = nullptr; // NOTE: We do not create `known_script` instances for hardcoded or SKSE types.

               // REMINDER: The value `dovah::form_type::none` represents the base Form class.
               // REMINDER: This enumeration also has values for aliases.
               std::optional<dovah::form_type_t> underlying_type;
            } extends;
            struct {
               bool conditional = false;
               bool hidden      = false;
            } flags;
         };

         using subsystem_passkey = cobb::passkey<core, known_script>;

      public:
         QString name;
         struct {
            std::optional<per_file_info> packed;
            std::optional<per_file_info> loose;
         } info;
         struct {
            bool cyclical = false;

            known_script* root_class = nullptr;
            struct {
               QVector<known_script*> loose;
               QVector<known_script*> packed;
            } potential_subclasses;
         } inheritance;

         constexpr bool exists() const noexcept {
            return info.packed.has_value() || info.loose.has_value();
         }

         int compare_name(QString) const;
         bool name_matches(QString) const;
         constexpr const known_script* superclass() const;
         constexpr known_script* superclass();

         bool is_of_type(dovah::form_type_t) const;
         bool is_of_type(QString desired) const; // DOES NOT handle hardcoded scriptnames representing form/alias types.
         constexpr std::optional<dovah::form_type_t> underlying_type() const;

         bool is_unreferenced() const;

         void receive_subclass(subsystem_passkey, known_script& subclass, bool loose);
         void abandon_loose_subclass(subsystem_passkey, known_script& subclass);

         // Non-recursive; used when initially loading all scripts.
         void _compute_root_class(subsystem_passkey);

         // Recursive: used when the class hierarchy has changed (e.g. because a loose file was added/edited/deleted).
         void _update_descendants_root_class(subsystem_passkey);
   };
}

#include "./known_script.inl"