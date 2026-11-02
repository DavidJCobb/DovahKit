#include "./string_collection.h"
#include "../tes/file.h"
#include "./active_string_file_bundle.h"
#include "./string_file_bundle.h"

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
}