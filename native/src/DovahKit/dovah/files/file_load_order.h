#pragma once
#include "../core.h"
#include <atomic>
#include <filesystem>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "file_load_order_normalizer.h"
#include "file_write_error.h"
#include "file_read_warning.h"
#include "../notice_code_t.h"
#include "../localized_strings.h"
#include "../data/game_settings.h"

namespace dovah {
   class form_stub;
   using map_of_forms = std::unordered_map<bare_form_id_t, form_stub*>;

   class  bsa_load_order;
   struct tes_file_header;
   class  threaded_load_order_use_info_builder;
   namespace tes_file_reading {
      class file_reader;
      class file_header_reader;
   }
   namespace tes_file_writing {
      struct write_config;
   }

   class form_creation_request;
   class form_duplication_request;
   class form_deletion_request;
   class form_renumber_request;

   namespace load_order_interfaces {
      class form_load;
   }

   class loaded_game_setting {
      public:
         const game_setting_definition*       definition  = nullptr;
         const tes_file_reading::file_reader* source_file = nullptr; // needed so we can deal with LSTRING values
         std::string        name;  // needed separate from the definition so we can track GMSTs with invalid/no names
         game_setting_value value;
         bare_form_id_t     formID = 0;
   };

   class file_load_order {
      //
      // Used to select files to load, and stores all loaded forms after the load process 
      // is complete.
      //
      friend void add_hardcoded_forms_to_load_order(file_load_order&);
      friend class form_creation_request;
      friend class form_deletion_request;
      friend class form_renumber_request;
      friend class load_order_interfaces::form_load;
      public:
         static constexpr uint8_t invalid_load_prefix = 0xFF;
         static constexpr uint8_t light_load_prefix   = 0xFE;
         using loaded_file   = tes_file_reading::file_reader;
         using loaded_header = tes_file_reading::file_header_reader;
         //
         enum class form_id_status {
            valid,
            out_of_bounds,
            missing_master,
            null_is_not_allowed,
            form_type_mismatch,
         };
         //
         using form_create_callback_t   = void(*)(form_stub*);
         using form_loss_callback_t     = void(*)(form_stub&);
         using form_renumber_callback_t = void(*)(form_stub&, bare_form_id_t oldID, bare_form_id_t newID);
         using warning_callback_t       = void(*)(const file_read_warning&);
         using generic_callback_t       = void(*)();
         //
      protected:
         struct _form_map {
            mutable std::mutex lock;
            map_of_forms forms;
         };
         using _form_map_by_type = std::array<_form_map, form_types.size()>;
         //
         file_load_order_normalizer normalizer;
         //
         std::array<threaded_load_order_use_info_builder*, 8> use_info_build_threads{};
         mutable std::mutex use_info_build_threads_lock;
         //
         struct {
            std::unordered_map<std::string, std::vector<loaded_game_setting>> by_name; // convert name to lowercase before lookups/insertions. vector should end up going from oldest-loaded to latest-loaded
            mutable std::mutex lock;
         } game_settings;
         bsa_load_order*   archives             = nullptr;
         std::vector<loaded_file*> files;
         loaded_file*      hardcoded_forms_file = nullptr; // needed so that form_stubs for non-overridden hardcoded forms can find this file_load_order. form_stubs rely on accessing the load order through their owning files.
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
         } save_load_state;
         #pragma endregion
         //
         void _make_hardcoded_forms();
         void _accept_hardcoded_form(form_stub*) noexcept;
         void _build_use_info();
         
         void _renumber_form(form_stub&, bare_form_id_t new_id, bool update_users);
         
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
         file_read_error          load_error;
         file_write_error         save_error;
         file_write_warning       save_warning;
         form_create_callback_t   on_form_create   = nullptr;
         form_loss_callback_t     on_form_loss     = nullptr; // occurs when a form stub is about to be unexpectedly deleted due to backend processes (e.g. SSE-only forms being lost after a conversion to Classic); frontend code MUST abandon the stub and its loaded form data
         form_renumber_callback_t on_form_renumber = nullptr;
         generic_callback_t       on_mass_renumber = nullptr; // occurs when changing whether the active file is an ESL
         warning_callback_t       on_read_warning  = nullptr; // warnings that occur when reading data from a TES file, whether during the initial stub build or when loading forms later. frontend is responsible for maintaining thread-safety.
         //
         void queue_file(const std::string& name);
         void unqueue_file(const std::string& name);
         void queue_active_file(const std::string& name); // TODO
         bool load_queued_files();
         //
         bool is_loading() const noexcept;
         
         form_id_status local_formID_to_global_formID(const loaded_file* file, uint32_t& id) const;
         form_id_status local_formID_to_global_formID(form_stub* stub, uint32_t& out) const;
         bare_form_id_t remap_formID_for_save(bare_form_id_t) const noexcept;
         void log_load_warning(const file_read_warning&);

         // (acceptFormStub)
         // Used by TESPluginFile to store a newly-loaded form stub. If the newly-loaded stub originates 
         // from an override record, then the overridden record's stub is deleted and replaced -- we 
         // only retain the last-loaded record for any given form ID, like the game and the CK.
         //
         // Notably NOT used for the initial load of hardcoded forms; see _accept_hardcoded_form.
         //
         form_id_status accept_form_stub(form_stub*) noexcept;
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
         int index_of_prefix(file_prefix) const noexcept;
         //
         bool has_file(const std::filesystem::path& filename) const noexcept;
         bool has_non_active_file(const std::filesystem::path& filename) const noexcept;
         #pragma endregion
         
         #pragma region Content related to already-loaded data
         uint32_t count_forms_of_type(form_type_t) const noexcept;
         inline uint8_t file_count() const noexcept { return this->files.size(); }
         bool has_form(bare_form_id_t formID) const noexcept;
         form_stub* get_form(bare_form_id_t formID) const noexcept;
         form_stub* get_form(form_type_t, bare_form_id_t formID) const noexcept; // use when you KNOW the form's type
         form_stub* get_form_of_probable_type(form_type_t, bare_form_id_t formID) const noexcept; // searches (formType) first, then the other types
         bool for_each_form_of_type(form_type_t formType, std::function<bool(form_stub*)> functor); // if the functor returns (true), this function stops early and also returns (true); otherwise, this function returns (false).
         //
         bool active_file_has_name() const noexcept;
         uint32_t active_file_form_count() const noexcept;
         bool active_file_has_forms_of_type(form_type_t) const noexcept;
         bare_form_id_t find_first_free_form_id_in_active_file(bare_form_id_t start_from = 0) const noexcept; // returns 0 if no free IDs
         bool for_each_active_file_form(std::function<bool(form_stub*)> functor);
         bool for_each_active_file_form_of_type(form_type_t form_type, std::function<bool(form_stub*)> functor);
         bool for_each_active_file_override_of_type(form_type_t form_type, std::function<bool(form_stub*)> functor);
         bool for_each_impossible_to_save_form(game, std::function<bool(form_stub*)> functor);
         bool for_each_top_level_form_needing_save(form_type_t form_type, std::function<bool(form_stub*)> functor);
         void get_active_file_name(std::filesystem::path& out) const noexcept;
         bool has_active_file() const noexcept;
         bool is_defined_in_active_file(const form_stub& stub) const noexcept;
         bool is_defined_or_overridden_in_active_file(const form_stub& stub) const noexcept;
         bool is_active_file_formID(bare_form_id_t) const noexcept;
         //
         bool get_loaded_setting_by_name(const std::string& name, loaded_game_setting& out) const noexcept;
         bool get_loaded_setting_by_name(const game_setting_definition& name, loaded_game_setting& out) const noexcept;
         //
         form_stub* create_form_of_type(form_type_t) noexcept;
         form_creation_request request_form_creation(form_type_t) noexcept;
         form_stub* commit_form_creation_request(form_creation_request&) noexcept; // you can call this, but you're meant to call form_creation_request::commit instead
         form_duplication_request request_form_duplication() noexcept;
         form_deletion_request request_form_deletion(form_stub&) noexcept;
         form_renumber_request request_form_renumber(form_stub&, bare_form_id_t desiredID) noexcept;
         void commit_form_renumber_request(form_renumber_request&) noexcept; // you can call this, but you're meant to call form_renumber_request::commit instead
         //
         bool for_each_load_order_filename(std::function<bool(std::filesystem::path, bool is_active_file)> functor);
         //
         void stub_flagged_as_edited(form_stub*) noexcept; // called by form_stub::set_edited
         //
         inline const loaded_file* get_active_file() const noexcept { return this->active_file; }
         tes_file_header* get_active_file_header() const noexcept;
         #pragma endregion

         bool save_active_file(std::filesystem::path replacement_filename, const dovah::tes_file_writing::write_config& cfg);
   };

   #pragma region Requests to manipulate forms
   class form_creation_request {
      friend class file_load_order;
      friend class form_duplication_request;
      //
      // Instances of this class can be created through the (file_load_order), and allow outside 
      // code to take actions in between reserving a form ID for use with a new form, and actually 
      // creating the new form. The use case that drove its creation: being able to have this UI 
      // flow:
      //
      //  - User asks to create a new form. We immediately try to reserve a form ID.
      //
      //  - If the reservation fails, we report an error and abort immediately.
      //
      //  - We ask the user for the desired editor ID.
      //
      //  - We create the form, with that editor ID, all in one go.
      //
      // This class is capable of creating a new, blank form, or of duplicating a single form. If 
      // you wish to duplicate a form and its children, then use (form_duplication_request).
      //
      public:
         enum class error_code {
            none,
            no_active_file,
            no_form_id_available,
            bad_form_type_requested,
            unsupported_form_type_requested,
            invalid_parent_child_relationship, // forms of (child_of)'s type cannot have children of type (form_type)
            exterior_grid_coordinates_already_taken, // cannot create an exterior cell; the desired grid coordinates are used by another cell in the same world
            cannot_create_reference_with_no_parent_cell,
            interior_cell_clone_cannot_have_parent,
            exterior_cell_clone_must_have_parent,
            form_created_but_clone_failed, // we were able to make a new form, but Form::clone() returned false
         };
      protected:
         file_load_order& owner;
         form_type_t      form_type = form_type::none;
         bare_form_id_t   formID    = 0;       // the form ID reserved for the newly-created form. set by the owning load order
         form_stub*       child_of  = nullptr; // what form should serve as the new form's parent?
         form_stub*       clone_of  = nullptr; // do we want to create a new form from scratch, or duplicate an existing one?
         error_code       error     = error_code::none;
         //
         // In order to return (form_creation_request) instances from functions that construct them 
         // without (form_creation_request::~form_creation_request) blowing away all of our data, we 
         // must: define a move constructor; and delete all copy constructors and copy-assignments. 
         // Copying shouldn't be allowed for this class anyway, though.
         //
         form_creation_request(file_load_order& o);
         form_creation_request(form_creation_request&&);
         form_creation_request(const form_creation_request&) = delete;
         form_creation_request& operator=(const form_creation_request&) = delete;
         //
      public:
         ~form_creation_request();
         //
         std::string editorID; // the editor ID to be used for the new form
         struct {
            int32_t x = 0;
            int32_t y = 0;
         } cell_grid_coordinates; // grid coordinates to use when creating an exterior cell
         //
         inline bool is_valid() const noexcept { return this->formID != 0 && this->error == error_code::none; } // returns (true) if the request has a reserved ID and has not yet completed/failed
         inline error_code get_error_code() const noexcept { return this->error; }
         //
         void set_parent_form(form_stub* parent);
         void set_parent_form(bare_form_id_t parentID);
         //
         void queue_clone(form_stub* original);
         form_stub* commit();
   };

   class form_duplication_request {
      public:
         using error_code = form_creation_request::error_code;
      protected:
         file_load_order& owner;
         form_creation_request* main_request = nullptr;
         std::vector<form_creation_request*> child_requests;
         //
         form_stub* parent = nullptr; // if the original form has a parent and you want the clone to have a different parent, use this. (nullptr) defaults to same parent.
         //
         form_duplication_request(form_duplication_request&&);
         form_duplication_request(const form_duplication_request&) = delete;
         form_duplication_request& operator=(const form_duplication_request&) = delete;
      public:
         form_duplication_request(file_load_order& o);
         ~form_duplication_request();
         //
         std::string editorID;
         //
         void set_target(form_stub* original);
         //
         void set_parent_form(form_stub* parent);
         void set_parent_form(bare_form_id_t parentID);
         //
         form_stub* commit();
         //
         error_code get_main_form_error_code() const noexcept;
         std::vector<error_code> get_child_form_error_codes() const noexcept; // returns only non-none errors
         std::vector<error_code> get_error_codes() const noexcept; // returns all error codes, including nones. main first, then children
         bool has_error() const noexcept;
         bool is_valid() const noexcept;
         unsigned int get_total_form_count() const noexcept;
   };
   
   class form_deletion_request {
      friend file_load_order;
      public:
         enum class result_code {
            pending,
            success,
            error_cannot_delete_hardcoded_form,
            error_cannot_load_form, // we could not load the to-be-deleted form to set its "deleted" flag
            error_cannot_load_user, // unable to sever use info: we could not load a form that uses the to-be-deleted form
         };
      protected:
         file_load_order& owner;
         form_stub&       target;
         result_code      result = result_code::pending;
         file_prefix      active_file_prefix; // cached for faster checks
         //
         std::set<form_stub*> seen_stubs;
         std::set<form_stub*> forms_needing_delete;
         std::set<form_stub*> forms_needing_flag;
         //
         form_deletion_request(file_load_order& o, form_stub& t);
         form_deletion_request(form_deletion_request&&);
         form_deletion_request(const form_deletion_request&) = delete;
         form_deletion_request& operator=(const form_deletion_request&) = delete;
         //
         bool _form_should_be_flagged(form_stub&);
         void _gather_others(form_stub* start = nullptr);
         void _prep_for_delete(form_stub&); // use for deletion and for flagging as deleted
         //
      public:
         bool force_delete_overrides = false; // if (true), then we will straight-up delete ALL forms. if (false), then forms outside the active file are overridden and FLAGGED AS deleted.
         //
         std::vector<form_stub*> get_forms_pending_delete(bool include_flagged = true) const noexcept;
         std::vector<form_stub*> get_forms_pending_flagging() const noexcept;
         inline result_code get_result_code() const noexcept { return this->result; }
         //
         void commit();
   };

   class form_renumber_request {
      friend class file_load_order;
      public:
         enum class error_code {
            none,
            no_active_file,
            is_hardcoded_form,       // you can't renumber hardcoded forms
            is_not_active_file_form, // you can't renumber forms that don't originate from the active file
            is_not_active_file_id,   // the desired form ID must be within the active file's form ID range
            desired_id_is_taken,
            desired_id_is_hardcoded, // you can't use IDs in the range reserved for hardcoded forms
            desired_id_is_none,      // you can't use 00000000 as a form ID
            cannot_load_user,        // unable to update a user form
         };
      protected:
         file_load_order& owner;
         form_stub&       target;
         bare_form_id_t   desiredID = 0;
         error_code       error     = error_code::none;
         //
         form_renumber_request(file_load_order& o, form_stub& target);
         form_renumber_request(form_renumber_request&&);
         form_renumber_request(const form_renumber_request&) = delete;
         form_renumber_request& operator=(const form_renumber_request&) = delete;
         //
      public:
         ~form_renumber_request();
         //
         inline error_code get_error_code() const noexcept { return this->error; }
         //
         bool commit();
   };
   #pragma endregion

   namespace load_order_interfaces {
      class form_load {
         friend class form_stub;
         public:
            file_load_order& owner;
            const form_stub& target_stub;
            tes_file_reading::file_reader* current_file = nullptr;
            bool is_winning_record = false;

            // TIP: This function only logs a warning if it has a warning code. Some helper functions can be 
            // called blindly to create and return warnings that only have a code if there's an actual problem.
            void log_load_warning(file_read_warning&);
            
         protected:
            form_load(file_load_order& o, const form_stub& t) : owner(o), target_stub(t) {}
      };
   }
}