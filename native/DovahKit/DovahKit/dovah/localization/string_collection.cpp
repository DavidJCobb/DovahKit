#include "./string_collection.h"
#include "../loaded_game_setting.h"
#include "../tes/file.h"
#include "./active_string_file_bundle.h"
#include "./string_file_bundle.h"
#include "./use_node.h"

namespace dovah::localization {
   string_collection::~string_collection() {
      this->_clear();
   }

   const active_string_file_bundle* string_collection::bundle_for_active_file() const {
      return this->_active_file;
   }
   const string_file_bundle* string_collection::bundle_for_non_active_file(const tes::file& tes_file) const {
      for (const auto* bundle : this->_non_active_files)
         if (&bundle->get_tes_file() == &tes_file)
            return bundle;
      return nullptr;
   }

   std::optional<string_collection::const_string_view> string_collection::lookup(
      const tes::file& tes_file,
      localized_string_type type,
      language lang,
      bare_localized_string_id id
   ) const noexcept {
      const auto* bundle = this->bundle_for_non_active_file(tes_file);
      if (!bundle) {
         auto* ab = this->_active_file;
         if (ab && &ab->get_tes_file() == &tes_file) {
            return ab->lookup(type, lang, id);
         }
         return {};
      }
      return bundle->lookup(type, lang, id);
   }

   void string_collection::_clear() {
      for (auto& [_, data] : this->_use_info) {
         for (auto* p : data.info)
            delete p;
         data.info.clear();
         for (auto* p : data.general)
            delete p;
         data.general.clear();
         for (auto* p : data.description)
            delete p;
         data.description.clear();
      }

      if (auto* p = this->_active_file) {
         this->_active_file = nullptr;
         delete p;
      }
      for (auto*& bundle : this->_non_active_files) {
         delete bundle;
         bundle = nullptr;
      }
      this->_non_active_files.clear();
   }
   void string_collection::_destroy_use_node_if_no_users(use_node& use) {
      if (use.users.forms.empty() && use.users.game_settings.empty()) {
         //
         // This active-file string is now unused.
         //
         auto& uses = this->_use_info[use.used.tes_file].for_type(use.used.type);
         auto  uit  = std::find(uses.begin(), uses.end(), &use);
         assert(uit != uses.end() && "use that the string collection doesn't know about?!");
         uses.erase(uit);
         delete &use;
      }
   }

   void string_collection::_clear(lu::passkey<active_load_order>) {
      this->_clear();
   }
   void string_collection::_load(lu::passkey<active_load_order>, const dovah::bsa::archive_collection& bsa_collection, std::span<const tes::file* const> tes_files, const tes::file* active_file) {
      const size_t size = tes_files.size();
      this->_non_active_files.reserve(size);
      for (size_t i = 0; i < size; ++i) {
         auto* p = new string_file_bundle(*tes_files[i]);
         this->_non_active_files.push_back(p);
      }
      for (size_t i = 0; i < size; ++i) {
         this->_non_active_files[i]->load(bsa_collection);
      }
      if (active_file) {
         this->_active_file = new active_string_file_bundle(*active_file);
         this->_active_file->load(bsa_collection);
      }
   }

   use_node& string_collection::_get_or_create_use_node(
      lu::passkey<form_stub, loaded_game_setting>,
      const tes::file& tes_file,
      localized_string_type type,
      bare_localized_string_id id
   ) {
      auto& data = this->_use_info[&tes_file];
      auto& uses = data.for_type(type);
      for (auto* use_ptr : uses)
         if (use_ptr->used.id == id)
            return *use_ptr;
      auto* use_ptr = new use_node;
      use_ptr->used = {
         .tes_file = &tes_file,
         .type     = type,
         .id       = id
      };
      uses.push_back(use_ptr);
      return *use_ptr;
   }

   void string_collection::_remove_active_file_string_user(lu::passkey<form_stub>, use_node& use, form_stub& user) {
      // Ensure that this actually is an active-file string use:
      assert(!!this->_active_file);
      assert(use.used.tes_file == &this->_active_file->get_tes_file());

      auto it = use.users.forms.find(&user);
      assert(it != use.users.forms.end() && "use isn't bidirectional?!");
      if (--(it->second) == 0) {
         use.users.forms.erase(it);
         this->_destroy_use_node_if_no_users(use);
      }
   }
   void string_collection::_remove_active_file_string_user(lu::passkey<loaded_game_setting>, use_node& use, loaded_game_setting& user) {
      // Ensure that this actually is an active-file string use:
      assert(!!this->_active_file);
      assert(use.used.tes_file == &this->_active_file->get_tes_file());

      auto it = use.users.game_settings.find(user.name.lowercase);
      assert(it != use.users.game_settings.end() && "use isn't bidirectional?!");
      use.users.game_settings.erase(it);
   }
}