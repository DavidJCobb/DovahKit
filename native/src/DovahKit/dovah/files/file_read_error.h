#pragma once
#include <string>
#include <type_traits>

namespace dovah {
   class file_read_error {
      public:
         struct error_code {
            error_code() = delete;
            enum type {
               none = 0,
               //
               // (active_file_is_dependency)
               // The active file cannot be a master to any other file in the requested 
               // load order. If records in the active file are overridden, we will not 
               // be able to recover the active-file versions of those forms, causing 
               // complications when saving.
               //
               active_file_is_dependency = 1,
               //
               // (malformed_file)
               // We detected some kind of problem when trying to read a file. This 
               // could indicate something like a record that is too small for its 
               // contents.
               //
               malformed_file = 2,
               //
               // (bad_form_id_in_esl)
               // Files that have been flagged as light plug-ins can only contain a 
               // limited number of forms, which must exist within a specific range of 
               // local form IDs.
               //
               bad_form_id_in_esl = 3,
               //
               // (missing_master)
               // One of the files in the load order has a master that could not be 
               // loaded.
               //
               missing_master = 4,
               //
               // (missing_file)
               // One of the files in the load order doesn't exist.
               //
               missing_file = 5,
               //
               // (locked_file)
               // We were unable to open one of the requested files, or one of their 
               // masters, apparently because the file in question is locked.
               //
               locked_file = 6,
               //
               // (out_of_bounds_form_id)
               // A file contained a form with an out-of-bounds form ID, e.g. a file 
               // with three masters (whose local forms would therefore use load order 
               // prefix 04) containing a form with a load order prefix above 04; or a 
               // file contained a form whose form ID corresponds to a master that, 
               // for unknown reasons, failed to load.
               //
               out_of_bounds_form_id = 7,
               //
               // (too_many_files)
               // The load order has ended up containing too many files. This can happen 
               // if more than 254 files are selected for load, or if there end up being 
               // more than 254 files that need to load after forcing selected files' 
               // unselected masters to load.
               //
               too_many_files = 8,
               //
               // (cyclical_dependency_between_files)
               // The load order contains files whose lists of masters form a cyclical 
               // dependency.
               //
               cyclical_dependency_between_files = 9,
               //
               // (unknown_error)
               // An error was caught somewhere "above" where it actually happened, and 
               // no details are available. If this error code is ever actually seen, 
               // it indicates that I forgot to have an error check report specific error 
               // details.
               //
               unknown_error = 10,
               //
               // (filesystem_error)
               // Generic codes for filesystem errors e.g. "too many open files."
               //
               filesystem_error = 11,
               //
               // (insufficient_memory)
               // The file contains a record that is impossible to load or parse due to 
               // its massive size. When this occurs, it may be a sign that something 
               // went wrong during parsing, and that we're misreading unrelated data 
               // as a record length.
               //
               insufficient_memory = 12,
               //
               // (active_file_is_master_and_there_are_plugins)
               // The active file must be at the end of the load order. However, it is 
               // impossible to ensure this, because the active file is ESM-flagged 
               // and there are non-ESMs in the load order.
               //
               active_file_is_master_and_there_are_plugins = 13,
            };
         };
         using error_code_t = std::underlying_type_t<error_code::type>;
         //
         error_code_t code = error_code::none;
         std::string  file;
         std::string  dependency;
         std::string  message;
         uint32_t     formID     = 0;
         uint32_t     fileOffset = 0;
         //
         inline bool defined() const noexcept { return this->code != error_code::none; }
         const char* code_string() const noexcept;
   };
}