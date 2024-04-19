#pragma once
#include "../core.h"
#include <atomic>
#include <filesystem>
#include <functional>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "file_load_order_normalizer.h"
#include "../utils/file_prefix.h"
#include "../detailed_notice.h"
#include "../form_stub.h"
#include "../localized_strings.h"
#include "../notice_code_t.h"
#include "../data/game_settings.h"

namespace dovah {
   namespace load_order_interfaces {
      class file_load;
      class form_load;
      class form_save;
   }
   namespace notices {
      class base_error;

      class base_warning;
      class base_form_load_warning;
   }
   namespace tes_file_reading {
      class file_loader;
      class file_header_reader;
      class read_results;
   }
   namespace tes_file_writing {
      struct write_config;
      class  write_results;
      class  file_writer;
   }
   class form_creation_request;
   class form_duplication_request;
   class form_deletion_request;
   class form_renumber_request;
   class game_setting_edit_request;
   class game_setting_renumber_request;

   //

   class form_stub;
   using map_of_forms = std::unordered_map<bare_form_id_t, form_stub*>;

   class  bsa_load_order;
   struct tes_file_header;
   class  threaded_load_order_use_info_builder;
}

namespace dovah {
   class loaded_game_setting {
      public:
         const game_setting_definition*       definition  = nullptr;
         const tes_file_reading::file_loader* source_file = nullptr; // needed so we can deal with LSTRING values
         std::string        name;  // needed separate from the definition so we can track GMSTs with invalid/no names
         game_setting_value value;
         bare_form_id_t     formID = 0;

         game_setting_type get_type() const noexcept;
         void set_value(const game_setting_value&) noexcept;
   };

   class file_load_order {
      //
      // Used to select files to load, and stores all loaded forms after the load process 
      // is complete.
      //
      friend void add_hardcoded_forms_to_load_order(file_load_order&);
      friend class form_deletion_request;
      friend class load_order_interfaces::file_load;
      friend class load_order_interfaces::form_load;
      friend class load_order_interfaces::form_save;
      public:
         static constexpr const uint8_t invalid_load_prefix = 0xFF;
         static constexpr const uint8_t light_load_prefix   = 0xFE;
         using loaded_file   = tes_file_reading::file_loader;
         using loaded_header = tes_file_reading::file_header_reader;
         
         enum class form_id_status {
            valid,
            out_of_bounds,
            missing_master,
            null_is_not_allowed,
            form_type_mismatch,
            injected_partial,
         };
         
         using form_create_callback_t     = void(*)(form_stub*);
         using form_loss_callback_t       = void(*)(form_stub&);
         using form_renumber_callback_t   = void(*)(form_stub&, bare_form_id_t oldID, bare_form_id_t newID);
         using detailed_notice_callback_t = void(*)(const detailed_notice&);
         using generic_callback_t         = void(*)();

         using emit_error_callback = void(*)(const notices::base_error&);
         using emit_warning_callback = void(*)(const notices::base_warning&);
         
      protected:
         struct _form_map {
            mutable std::mutex lock;
            map_of_forms forms;
         };
         class _form_map_by_type : public std::array<_form_map, form_types.size()> {
            protected:
               static constexpr const size_t form_types_are_contiguous_up_to = []() -> size_t {
                  for (size_t i = 0; i < form_types.size(); ++i)
                     if ((size_t)form_types[i].form_type != i)
                        return i;
                  return form_types.size();
               }();
               static constexpr const bool all_form_types_are_contiguous = form_types_are_contiguous_up_to == form_types.size();

            public:
               using array::operator[];

               constexpr _form_map& operator[](form_type ft) {
                  return const_cast<_form_map&>(std::as_const(*this).operator[](ft));
               }
               constexpr const _form_map& operator[](form_type ft) const {
                  if constexpr (all_form_types_are_contiguous) {
                     return operator[]((size_t)ft);
                  } else {
                     if ((size_t)ft < form_types_are_contiguous_up_to)
                        return operator[]((size_t)ft);
                     for (size_t i = form_types_are_contiguous_up_to; i < form_types.size(); ++i)
                        if (form_types[i].form_type == ft)
                           return operator[](i);
                     throw std::out_of_range("no form map exists for this form type");
                  }
               }

               static constexpr bool supports_form_type(form_type ft) {
                  if constexpr (all_form_types_are_contiguous) {
                     return (size_t)ft < form_types_are_contiguous_up_to;
                  } else {
                     if ((size_t)ft < form_types_are_contiguous_up_to)
                        return true;
                     for (size_t i = form_types_are_contiguous_up_to; i < form_types.size(); ++i)
                        if (form_types[i].form_type == ft)
                           return true;
                     return false;
                  }
               }
         };
         
         file_load_order_normalizer normalizer;
         //
         std::array<tes_file_reading::threaded_load_order_use_info_builder*, 8> use_info_build_threads{};
         mutable std::mutex use_info_build_threads_lock;
         //
         struct {
            std::unordered_map<std::string, std::vector<loaded_game_setting>> by_name; // convert name to lowercase before lookups/insertions. vector should end up going from oldest-loaded to latest-loaded
            mutable std::mutex lock;
         } game_settings;
         bsa_load_order*   archives             = nullptr;
         std::vector<loaded_file*> files;
         loaded_file*      hardcoded_forms_file = nullptr; // needed so that form_stubs for non-overridden hardcoded forms can find this file_load_order. form_stubs rely on accessing the load order through their owning files.
         loaded_file*      none_stubs_file      = nullptr; // needed so that none-stubs can find this file_load_order.
         loaded_file*      active_file          = nullptr;
         _form_map         forms;
         _form_map_by_type forms_by_type;
         _form_map         active_file_forms; // all forms that come from the active file AND all forms overridden in the active file, which means that some of these may have originally loaded from different files.
         _form_map_by_type active_file_forms_by_type;
         game              current_game = game::skyrim_special;
         //
         struct {
            mutable std::recursive_mutex lock;
            std::vector<bare_form_id_t> reserved_formIDs; // form IDs reserved for form creation or form renumbering
         } form_creation_request_info;
         //
         #pragma region State fields for tracking whether and how we are saving and loading
         enum class save_load_type : uint8_t {
            none = 0,
            is_loading,
            is_saving,
         };
         struct _save_load_lock_guard {
            protected:
               file_load_order& owner;
               bool success = false;
            public:
               _save_load_lock_guard(file_load_order& o, file_load_order::save_load_type);
               ~_save_load_lock_guard();
               inline operator bool() const noexcept { return this->success; }
         };
         struct save_load_flag { // Flags for reporting progress and status to UI and other non-critical systems
            save_load_flag() = delete;
            enum type : uint8_t {
               none = 0,
               loading_is_complete        = 0x01,
               use_info_outbound_complete = 0x02,
               use_info_build_is_complete = 0x04,
            };
         };
         using save_load_flags_t = std::underlying_type_t<save_load_flag::type>;
         //
         struct {
            std::atomic<save_load_type> type  = save_load_type::none;
            save_load_flags_t           flags = save_load_flag::none; // helps with UI progress display
            uint8_t loading_index = 0; // which load order index we're loading, or 0 if none; set in (load_queued_files); see (_guidedLoadOrderPrefixFor)
            tes_file_reading::read_results* current_load_results = nullptr;
         } save_load_state;
         #pragma endregion
         //
         void _make_hardcoded_forms();
         void _accept_hardcoded_form(form_stub*) noexcept;
         void _build_none_stubs();
         void _reparent_persistent_references();
         void _build_use_info();

         void _log_load_warning(const detailed_notice&);
         void _log_save_warning(const detailed_notice&);

         void _log_warning(const notices::base_warning&);

         bool _abandon_form_id_reservation(bare_form_id_t);

         notice_code_t _destroy_none_stub(form_stub&);
         
         void _renumber_form(form_stub&, bare_form_id_t new_id, bool update_users);
         notice_code_t _renumber_game_setting(loaded_game_setting&, bare_form_id_t new_id);

         uint32_t _count_game_settings_with_form_id(bare_form_id_t) const noexcept; // doesn't lock
         
         //
         // Try to change whether light plug-in support is enabled. This can be done before files are loaded, or it 
         // can be done when saving (i.e. when converting a Skyrim Classic file to a Skyrim Special ESL, or when 
         // converting a Skyrim Special file to a Skyrim Classic file.)
         //

         notice_code_t _change_current_game(game g, bool because_we_are_changing_whether_the_active_file_is_light);
         notice_code_t _can_change_current_game(game g, bool because_we_are_changing_whether_the_active_file_is_light) const noexcept;
         
      public:
         ~file_load_order();
         //
         bool is_light_plugin_support_enabled() const noexcept;
         //
         inline game get_current_game() const noexcept { return this->current_game; }
         notice_code_t can_change_current_game(game) const noexcept;
         notice_code_t change_current_game(game);
         //
         #pragma region Content related to loading
         std::string base_path; // used for loading and saving. changing this between loading files and saving them back out is undefined behavior.
         struct {
            std::vector<std::string> files;
            std::string active_file;
            //
            struct {
               bool allow_unknown_record_signatures    = false;
               bool allow_suspicious_record_signatures = false;
            } options;
         } queued_load;
         form_create_callback_t     on_form_create   = nullptr;
         form_loss_callback_t       on_form_loss     = nullptr; // occurs when a form stub is about to be unexpectedly deleted due to backend processes (e.g. SSE-only forms being lost after a conversion to Classic); frontend code MUST abandon the stub and its loaded form data
         form_renumber_callback_t   on_form_renumber = nullptr;
         generic_callback_t         on_mass_renumber = nullptr; // occurs when changing whether the active file is an ESL
         detailed_notice_callback_t on_read_warning  = nullptr; // warnings that occur when reading data from a TES file, whether during the initial stub build or when loading forms later. frontend is responsible for maintaining thread-safety.
         detailed_notice_callback_t on_save_warning  = nullptr; // warnings that occur when saving a file. frontend is responsible for maintaining thread-safety.
         emit_error_callback        on_error         = nullptr;
         emit_warning_callback      on_warning       = nullptr;
         //
         void queue_file(const std::string& name);
         void unqueue_file(const std::string& name);
         void queue_active_file(const std::string& name); // TODO
         bool load_queued_files(tes_file_reading::read_results& results);
         //
         bool is_loading() const noexcept;
         
         form_id_status local_formID_to_global_formID(const loaded_file* file, uint32_t& id) const;
         form_id_status local_formID_to_global_formID(form_stub* stub, uint32_t& out) const;
         bare_form_id_t remap_formID_for_save(bare_form_id_t) const noexcept;

         // 
         // Used by file_or_file_part_loader to store a newly-loaded form stub. If the newly-loaded stub 
         // originates from an override record, then the overridden stub will import data from the new 
         // stub, and then the new stub will be deleted with the passed-in pointer set to the overridden 
         // stub. We only retain the last-loaded record for any given form ID, like the game and the CK.
         //
         // This will not delete the stub if an error occurs.
         //
         // The incoming stub should have exactly one file in its source file list: the file it was read 
         // from. We assert this if the stub is an override.
         //
         // Notably NOT used for the initial load of hardcoded forms; see _accept_hardcoded_form.
         //
         form_id_status accept_form_stub(form_stub*&) noexcept;

         void accept_game_setting(const loaded_file* file, const loaded_game_setting&, bare_form_id_t formID) noexcept;
         #pragma endregion

         #pragma region content related to BSAs
         void adopt_archive_list(bsa_load_order&) noexcept; // takes ownership of the given list
         inline bsa_load_order* get_archive_list() const noexcept { return this->archives; }
         #pragma endregion

         float assess_load_progress() const noexcept;

         bool is_form_loading_blocked(const form_stub*) const noexcept;

         #pragma region Finding files
         file_prefix file_prefix_for(const std::filesystem::path& filename) const noexcept;
         file_prefix file_prefix_for(const loaded_file& file) const noexcept;
         file_prefix file_prefix_for(const loaded_file& file, bool pretend_is_or_isnt_light) const noexcept;
         inline uint8_t load_prefix_for(const std::filesystem::path& f) const noexcept {
            return this->file_prefix_for(f).load_prefix();
         }
         inline uint8_t load_prefix_for(const loaded_file& f) const noexcept {
            return this->file_prefix_for(f).load_prefix();
         }
         inline uint8_t light_prefix_for(const std::filesystem::path& f) const noexcept {
            return this->file_prefix_for(f).light_prefix();
         }
         inline uint8_t light_prefix_for(const loaded_file& f) const noexcept {
            return this->file_prefix_for(f).light_prefix();
         }
         file_prefix active_file_prefix() const noexcept;
         //
         int index_of_file(const loaded_file&) const noexcept;
         int index_of_prefix(file_prefix) const noexcept;
         //
         const loaded_file* get_file_by_prefix(file_prefix) const noexcept;
         const loaded_file* get_file_by_index(int) const noexcept;
         //
         bool file_is_active(const loaded_file&) const noexcept;
         bool has_file(const std::filesystem::path& filename) const noexcept;
         bool has_non_active_file(const std::filesystem::path& filename) const noexcept;
         #pragma endregion
         
         #pragma region Content related to already-loaded data
         std::vector<const loaded_file*> get_loaded_files() const noexcept;

         uint32_t count_forms_of_type(form_type) const noexcept;
         inline uint8_t file_count() const noexcept { return this->files.size(); }
         bool has_form(bare_form_id_t formID) const noexcept;
         form_stub* get_canonical_instance_of_singleton_form(form_type) const noexcept;
         form_stub* get_canonical_instance_of_singleton_form(form_type, bool create_if_missing = false) noexcept; // (create_if_missing) can fail if no available form IDs
         form_stub* get_form(bare_form_id_t formID, bool ignore_none_stubs = true) const noexcept;
         form_stub* get_form(form_type, bare_form_id_t formID) const noexcept; // use when you KNOW the form's type
         form_stub* get_form_of_probable_type(form_type, bare_form_id_t formID, bool ignore_none_stubs = true) const noexcept; // searches (formType) first, then the other types
         bool for_each_form_of_type(form_type formType, std::function<bool(form_stub*)> functor); // if the functor returns (true), this function stops early and also returns (true); otherwise, this function returns (false).
         //
         bool active_file_has_name() const noexcept;
         uint32_t active_file_form_count() const noexcept;
         bool active_file_has_forms_of_type(form_type) const noexcept;
         bare_form_id_t find_first_free_form_id_in_active_file(bare_form_id_t start_from = 0) const noexcept; // returns 0 if no free IDs
         bool for_each_active_file_form(std::function<bool(form_stub*)> functor);
         bool for_each_active_file_form_of_type(form_type form_type, std::function<bool(form_stub*)> functor);
         bool for_each_active_file_override_of_type(form_type form_type, std::function<bool(form_stub*)> functor);
         bool for_each_impossible_to_save_form(game, std::function<bool(form_stub*)> functor);
         bool for_each_top_level_form_needing_save(form_type form_type, std::function<bool(form_stub*)> functor);
         void get_active_file_name(std::filesystem::path& out) const noexcept;
         bool has_active_file() const noexcept;
         bool is_defined_in_active_file(const form_stub& stub) const noexcept;
         bool is_defined_or_overridden_in_active_file(const form_stub& stub) const noexcept;
         bool is_active_file_formID(bare_form_id_t) const noexcept;
         //
         bool for_each_loaded_game_setting(std::function<bool(const loaded_game_setting&)> functor);
         bool for_each_active_file_game_setting(std::function<bool(const loaded_game_setting&)> functor);
         bool get_loaded_setting_by_name(const std::string& name, loaded_game_setting& out) const noexcept;
         bool get_loaded_setting_by_name(const game_setting_definition& name, loaded_game_setting& out) const noexcept;
         
         form_stub* create_form_of_type(form_type) noexcept;
         form_creation_request request_form_creation(form_type) noexcept;
         form_stub* commit_form_creation_request(form_creation_request&) noexcept; // you can call this, but you're meant to call form_creation_request::commit instead
         form_duplication_request request_form_duplication() noexcept;
         form_deletion_request request_form_deletion(form_stub&) noexcept;
         form_renumber_request request_form_renumber(form_stub&, bare_form_id_t desiredID) noexcept;
         void commit_form_renumber_request(form_renumber_request&) noexcept; // you can call this, but you're meant to call form_renumber_request::commit instead
         game_setting_edit_request request_game_setting_change(bool automatic_id = true) noexcept;
         void commit_game_setting_change_request(game_setting_edit_request&) noexcept;
         game_setting_renumber_request request_game_setting_renumber() noexcept;
         void commit_game_setting_renumber_request(game_setting_renumber_request&) noexcept;
         //
         void set_reserved_form_id_for(game_setting_edit_request&,     bare_form_id_t desired = 0);
         void set_reserved_form_id_for(game_setting_renumber_request&, bare_form_id_t desired = 0);
         //
         void abandon_form_id_reservation(form_creation_request&);
         void abandon_form_id_reservation(form_renumber_request&);
         void abandon_form_id_reservation(game_setting_edit_request&);
         void abandon_form_id_reservation(game_setting_renumber_request&);
         
         bool for_each_load_order_filename(std::function<bool(std::filesystem::path, bool is_active_file)> functor);
         //
         void stub_flagged_as_edited(form_stub*) noexcept; // called by form_stub::set_edited
         //
         inline const loaded_file* get_active_file() const noexcept { return this->active_file; }
         tes_file_header* get_active_file_header() const noexcept;
         #pragma endregion

         //
         // Saves the active file.
         //
         // Even if a save operation succeeds, individual forms may be discarded, perhaps because they were 
         // of a type not  supported by the target game or  perhaps because they were  none-stubs. The form 
         // loss callback will fire for every  such stub. Stubs are  deleted using a form deletion request, 
         // so no dangling references or pointers should be left within loaded form data.
         //
         // The return value is a  success bool. Specific error information can be  found in the load order 
         // instance's (save_error) field.
         //
         bool save_active_file(std::filesystem::path replacement_filename, const dovah::tes_file_writing::write_config& cfg, dovah::tes_file_writing::write_results& results);
   };
}