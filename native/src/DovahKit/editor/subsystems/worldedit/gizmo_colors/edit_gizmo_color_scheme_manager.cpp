#include "./edit_gizmo_color_scheme_manager.h"
#include "./hardcoded.h"

namespace dovahkit::subsystems::worldedit {
   gizmo_color_scheme_manager::gizmo_color_scheme_manager() : QObject(nullptr) {
   }

   std::vector<gizmo_color_scheme> gizmo_color_scheme_manager::all_hardcoded_color_schemes() const {
      const auto& src = hardcoded_gizmo_color_schemes;

      std::vector<gizmo_color_scheme> out;
      out.resize(src.size());
      for (size_t i = 0; i < out.size(); ++i)
         out[i] = src[i];
      return out;
   }
   const std::vector<gizmo_color_scheme>& gizmo_color_scheme_manager::all_user_color_schemes() const {
      return this->_user_schemes;
   }

   bool gizmo_color_scheme_manager::scheme_name_is_taken(const std::string& name) const {
      for (const auto& scheme : hardcoded_gizmo_color_schemes)
         if (scheme.name == name)
            return true;
      for (const auto& scheme : this->_user_schemes)
         if (scheme.name == name)
            return true;
      return false;
   }

   std::string gizmo_color_scheme_manager::add_new_color_scheme(const char* name, const gizmo_color_scheme& based_on) {
      std::string name_to_use = name;
      if (name_to_use.empty())
         return {}; // TODO: if we decide to throw exceptions, we should throw on this, too
      {
         // TODO: If `name` already has a parenthetical number, increment that instead 
         //       of appending to it.
         // 
         // TODO: Should we just throw an exception if the name is taken, and leave 
         //       incrementing the number as the UI's responsibility?
         //
         size_t attempts = 0;
         while (this->scheme_name_is_taken(name_to_use)) {
            name_to_use = name_to_use + " (" + std::to_string(attempts) + ")";
            if (++attempts > 100)
               return {};
         }
      }

      auto& scheme = this->_user_schemes.emplace_back();
      scheme = based_on;
      std::swap(scheme.name, name_to_use);
      return name_to_use;
   }

   void gizmo_color_scheme_manager::reload() {
      // TODO
   }
   void gizmo_color_scheme_manager::save() {
      // TODO
   }

   gizmo_color_scheme gizmo_color_scheme_manager::get_current_color_scheme() const {
      const auto& name = this->_current_scheme.name;
      if (!this->_current_scheme.name.empty()) {
         if (this->_current_scheme.is_hardcoded) {
            for (const auto& scheme : hardcoded_gizmo_color_schemes) {
               if (scheme.name == name)
                  return scheme;
            }
         } else {
            for (const auto& scheme : this->_user_schemes) {
               if (scheme.name == name)
                  return scheme;
            }
         }
      }
      return gizmo_color_scheme{
         .name = "Standard",
         .axes = {
            gizmo_color_scheme::rgb{ 255, 0, 0 },
            gizmo_color_scheme::rgb{ 0, 255, 0 },
            gizmo_color_scheme::rgb{ 0, 0, 255 }
         },
         .highlight = { 255, 255, 0 }
      };
   }
   gizmo_color_scheme_manager::color_scheme_id gizmo_color_scheme_manager::get_current_color_scheme_id() const {
      return this->_current_scheme;
   }
   void gizmo_color_scheme_manager::set_current_color_scheme_id(const color_scheme_id& id) {
      if (this->_current_scheme == id)
         return;
      
      // abort if no matching scheme exists
      bool found = false;
      if (id.is_hardcoded) {
         for (const auto& scheme : hardcoded_gizmo_color_schemes) {
            if (scheme.name == id.name) {
               found = true;
               break;
            }
         }
      } else {
         for (const auto& scheme : this->_user_schemes) {
            if (scheme.name == id.name) {
               found = true;
               break;
            }
         }
      }
      if (!found)
         return;

      this->_current_scheme = id;
      emit this->colorSchemeChanged(this->get_current_color_scheme());
   }
}