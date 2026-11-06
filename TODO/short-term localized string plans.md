
## Things I want or need for localized string editing and use info

* Use Info for localized strings, to see where any given string is used

  * Can be read-only for non-active-file localized strings

* The ability to edit active file strings

  * Use Info, to know what strings can be deleted
  
  * The ability to switch whether the active file uses localized strings (with forms and GMSTs referring by string ID) or non-localized strings (with forms and GMSTs storing data inline as an `std::string`)
  
  * The ability to edit the text of an existing string, per language
  
  * The ability to create a new string
  
    * Including the ability to take a string used in two places, and split it into two identical (and independently alterable) strings each used in just the one place
  
## Things we have and things we need

### Have

* Infrastructure

  * `string_file_bundle`, as read-only storage for non-active-file localized strings

  * `active_string_file_bundle`, as mutable storage for active-file localized strings

  * String use info (for both forms as users, and GMSTs as users)
  
  * Managing string use info
  
    * Start tracking form-to-string uses when we build form-stub use info
    
    * Start tracking GMST-to-string uses when we load GMSTs
    
    * Update form-to-string use info when a `managed_localized_string_use` is `set`
    
    * Update GMST-to-string use info when a GMST's value is changed
  
* Accessors

  * For-each functions for all localized strings belonging to a given TES file

### Need

* A process whereby an active file can be switched between using localized and non-localized strings.

  * Non-localized to localized: We must iterate over every form type that can contain l-strings, iterate over every form of those types, pull the forms' string data into our centralized collection, and then write IDs back into the forms. Then, we must set the "localized" flag on the active file's header.
    
    * Strings should be de-duplicated: if identical text appears for multiple l-string fields (that are of the same l-string type, i.e. STRINGS/DLSTRINGS/ILSTRINGS), then we should create just one l-string ID and have it be shared across those fields.
  
    * We must do the same for string-type GMSTs.
    
    * We must also build l-string Use Info for the active file.
  
  * Localized to non-localized: User must specify a target language, and that language must be present. We must iterate over every localized string ID (taking advantage of l-string Use Info, as described below), load the using forms and GMSTs, and replace their ID-uses with inlined `std::string`s. Then, we must clear the "localized" flag on the active file's header.
    
    * We must remember to update l-string Use Info for the active file (i.e. by clearing it en masse).

* When an active file l-string is deleted, we must sever Use Info between it and any forms and GMSTs that used it, amending those to use ID 0 (i.e. "no string") in its place.
