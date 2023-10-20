#pragma once
#include <optional>
#include <ranges>
#include <vector>
#include <QObject>
#include "helpers/unreachable.h"
#include "editor/subsystems/options/core.h"
#include "./control_scheme.h"

class QFile;

namespace dovahkit::subsystems::worldinput {
   class control_scheme_manager;
   class control_scheme_manager : public QObject, public cobb::singleton_ex<control_scheme_manager> {
      Q_OBJECT;
      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

         using scheme_name_type = decltype(control_scheme::name);
         using scheme_filename_type = QString;

         struct saved_control_scheme : control_scheme {
            friend class control_scheme_manager;
            protected:
               using control_scheme::control_scheme;
               saved_control_scheme(const control_scheme& d) : control_scheme(d) {}
               saved_control_scheme(control_scheme&& d) : control_scheme(std::move(d)) {}

            public:
               scheme_filename_type filename; // can only be blank for hardcoded schemes

               inline bool is_hardcoded() const { return filename.isEmpty() || filename[0] == '?'; }
         };

      protected:
         control_scheme_manager();
         ~control_scheme_manager() {
            this->_clear_all_schemes();
         }

         struct {
            std::vector<saved_control_scheme*> gamepad;
            std::vector<saved_control_scheme*> keyboard;
         } _schemes_by_device;
         
         constexpr const std::vector<saved_control_scheme*>& _get_schemes_by_device(input_device_type d) const {
            switch (d) {
               case input_device_type::keyboard_mouse:
                  return this->_schemes_by_device.keyboard;
               case input_device_type::xinput:
                  return this->_schemes_by_device.gamepad;
            }
            cobb::unreachable();
         }
         constexpr std::vector<saved_control_scheme*>& _get_schemes_by_device(input_device_type d) {
            const auto& r = std::as_const(*this)._get_schemes_by_device(d);
            return const_cast<std::decay_t<decltype(r)>&>(r);
         }

         static QString _path_for_scheme(const scheme_filename_type&);

         static cobb::ini::setting& _current_scheme_setting(input_device_type);

         std::optional<control_scheme> _load_scheme(QFile&);
         void _save_scheme(QFile&, const control_scheme&);

         void _clear_all_schemes();

      public:
         static QString path_for_scheme_folder();

         constexpr const auto schemes_by_device(input_device_type d) const {
            return this->_get_schemes_by_device(d) | std::ranges::views::transform([](auto* e) -> const saved_control_scheme* { return e; });
         }
         constexpr std::vector<const saved_control_scheme*> all_control_schemes() const {
            std::vector<const saved_control_scheme*> out;
            out.reserve(this->_schemes_by_device.gamepad.size() + this->_schemes_by_device.keyboard.size());
            for (const auto* item : this->_schemes_by_device.gamepad)
               out.push_back(item);
            for (const auto* item : this->_schemes_by_device.keyboard)
               out.push_back(item);
            return out;
         }

         // For example, if you pass in "My Name" and that's taken, you'll get "My Name (2)". Names must be 
         // unique per input device (i.e. schemes for different devices can have the same name). Returns true 
         // if any adjustment was made.
         bool adjust_scheme_name_by_availability(input_device_type, scheme_name_type& in_out_name);

         // Returns nullptr if that scheme name is taken for that device.
         const saved_control_scheme* add_scheme(const control_scheme&);

         void overwrite_scheme(const saved_control_scheme*, const control_scheme&);

         void delete_scheme(const saved_control_scheme*);

         const saved_control_scheme* lookup_scheme(input_device_type, const scheme_name_type&) const;
         const saved_control_scheme* lookup_scheme(input_device_type d, const std::string& n) const {
            if (n.empty())
               return nullptr;
            return this->lookup_scheme(d, QString::fromUtf8(n.data(), n.size()));
         }

         const saved_control_scheme* lookup_scheme_by_filename(input_device_type, const scheme_filename_type&) const;
         const saved_control_scheme* lookup_scheme_by_filename(input_device_type d, const std::string& n) const {
            if (n.empty())
               return nullptr;
            return this->lookup_scheme_by_filename(d, QString::fromUtf8(n.data(), n.size()));
         }

         void reload_all_control_schemes();

         const control_scheme& get_current_scheme(input_device_type);
         void set_current_scheme(const saved_control_scheme*);

         bool is_current_scheme(const saved_control_scheme*);

      signals:
         void beforeReloadAll();
         void beforeReset();
         void reloadedAll();

         // Emitted when any control scheme is modified. The name passed here is the one the control scheme 
         // had before any modifications; for the current (new) name, see after.name.
         void controlSchemeModified(const QString& name_prior, const control_scheme& after, bool is_current);

         // Emited when we have changed which scheme is the current scheme.
         void currentSchemeChanged(const control_scheme&);
   };
}