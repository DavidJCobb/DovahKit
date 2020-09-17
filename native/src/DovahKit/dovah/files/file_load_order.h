#pragma once
#include "../core.h"
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "file_load_order_normalizer.h"

namespace dovah {
   class form_stub;
   using map_of_forms = std::unordered_map<bare_form_id_t, form_stub*>;

   class file_header;
   namespace tes_file_reading {
      class file_reader;
   }

   class file_load_order {
      //
      // Used to select files to load, and stores all loaded forms after the load process 
      // is complete.
      //
      friend void add_hardcoded_forms_to_load_order(file_load_order&);
      public:
         static constexpr uint8_t invalid_load_prefix = 0xFF;
         using loaded_file = tes_file_reading::file_reader;
         //
         enum class form_id_status {
            valid,
            out_of_bounds,
            missing_master,
            null_is_not_allowed,
         };
         //
      protected:
         struct _form_map {
            mutable std::mutex lock;
            map_of_forms forms;
         };
         using _form_map_by_type = std::array<_form_map, form_types.size()>;
         //
         file_load_order_normalizer normalizer;
         std::vector<loaded_file*> files;
         loaded_file*      hardcoded_forms_file = nullptr; // needed so that form_stubs for non-overridden hardcoded forms can find this file_load_order. form_stubs rely on accessing the load order through their owning files.
         loaded_file*      active_file          = nullptr;
         _form_map         forms;
         _form_map_by_type forms_by_type;
         _form_map         active_file_forms; // all forms that come from the active file AND all edited forms, which means that some of these may have originally loaded from different files.
         _form_map_by_type active_file_forms_by_type;
         //
         bool    loading_is_complete = false; // exists so that TESPluginBaseReader::nextSubrecord can call LoadOrder::logError without having to worry about whether it's running during or after the initial load
         uint8_t loading_index       = 0;     // which load order index we're loading, or 0 if none; set in (load_queued_files); see (_guidedLoadOrderPrefixFor)
         uint8_t active_file_index   = invalid_load_prefix;
         //
         void _make_hardcoded_forms();
         void _accept_hardcoded_form(form_stub*) noexcept;
         void _build_use_info();
         //
         uint8_t load_order_prefix_for(const loaded_file*) const noexcept;
         uint8_t guided_load_order_prefix_for(const loaded_file*) const noexcept; // a version of (load_order_prefix_for) that's faster when called while loading the specified file
         //
      public:
         ~file_load_order();
         //
         #pragma region Content related to loading
         struct {
            std::string base_path;
            std::vector<std::string> files;
            std::string active_file;
            //
            struct {
               bool allow_unknown_record_signatures    = false;
               bool allow_suspicious_record_signatures = false;
            } options;
         } queued_load;
         file_read_error load_error;
         //
         void queue_file(const std::string& name);
         void unqueue_file(const std::string& name);
         void queue_active_file(const std::string& name); // TODO
         bool load_queued_files();
         //
         inline bool is_loading() const noexcept { return !this->loading_is_complete; };
         
         form_id_status local_formID_to_global_formID(const loaded_file* file, uint32_t& id) const;
         form_id_status local_formID_to_global_formID(form_stub* stub, uint32_t& out) const;

         // (acceptFormStub)
         // Used by TESPluginFile to store a newly-loaded form stub. If the newly-loaded stub originates 
         // from an override record, then the overridden record's stub is deleted and replaced -- we 
         // only retain the last-loaded record for any given form ID, like the game and the CK.
         //
         // Notably NOT used for the initial load of hardcoded forms; see _accept_hardcoded_form.
         //
         form_id_status accept_form_stub(form_stub*) noexcept;
         #pragma endregion
         
         #pragma region Content related to already-loaded data
         uint32_t count_forms_of_type(form_type_t) const noexcept;
         inline uint8_t file_count() const noexcept { return this->files.size(); }
         bool has_form(bare_form_id_t formID) const noexcept;
         uint8_t index_of_loaded_file(const std::string& filename) const noexcept;
         form_stub* get_form(bare_form_id_t formID) const noexcept;
         form_stub* get_form(form_type_t, bare_form_id_t formID) const noexcept; // use when you KNOW the form's type
         form_stub* get_form_of_probable_type(form_type_t, bare_form_id_t formID) const noexcept; // searches (formType) first, then the other types
         bool for_each_form_of_type(form_type_t formType, std::function<bool(form_stub*)>); // if the functor returns (true), this function stops early and also returns (true); otherwise, this function returns (false).
         bool form_is_from_active_file(const form_stub*) const noexcept;
         bool form_is_from_active_file(bare_form_id_t) const noexcept;
         //
         void stub_flagged_as_edited(form_stub*) noexcept; // called by form_stub::set_edited
         #pragma endregion
         
   };
}