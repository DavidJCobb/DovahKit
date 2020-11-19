#include <QtWidgets/QApplication>

#if _DEBUG
   #include <QDebug>
   #include <QDirIterator>
   #undef NDEBUG
#endif

#include "ui/main_window.h"

//
// CURRENT TASKS:
//
//  - Use std::filesystem::path instead of std::string for file paths and names in 
//    dovah::file_load_order, dovah::tes_file_reading::file_reader, and so on.
//
//  - tes_file_reading::subrecord::to_string is unintuitive; that should just be a 
//    template specialization of subrecord::read.
//
//  - FormsOfTypeCombobox won't automatically add newly-created forms. It doesn't 
//    listen for the DovahKitCore::formCreated signal.
//
//  - Cell View
//
//     - Implement the "Sort loaded at top" checkbox.
//
//        - Can't, until we have a render window that supports worldspaces. Once the 
//          render window is ready, we can update CellListModelItem::cellIsLoaded, 
//          and also update the list model proxy to prioritize that above all other 
//          sorting.
//
//  - The TESFilePicker widget should allow the user to browse through loose files and 
//    files in the loaded BSAs.
//
//  - When saving a new file, if the user enters the name of an existing file, then we 
//    should prompt for permission to overwrite. If the name that the user enters is 
//    the same name as one of the already-loaded files, we should fail.
//
//     - If the user converts a file across games, and the existing file's name also 
//       exists for the target game, then we should prompt to overwrite as well.
//
//  - Log window
//
//     - The log window should have a "message type" column, differentiating between 
//       notices from the initial stub build, notices from on-demand form loading, 
//       and notices from saves.
//
//     - The log window does not allow you to select entries, and offers no means to 
//       copy them.
//
//     - The log window should ideally use a table view instead of a list view, with 
//       columns for the filename (and potentially other details).
//
//  - We should log a warning when loading a form that is misplaced into the wrong 
//    GRUP.
//
//  - Currently, form stubs retain the GRUP they loaded from. This is used to identify 
//    a worldspace's persistent cell, as well as to sort REFRs into a cell's persistent 
//    and temporary child GRUPs when saving. Neither of these uses is going to be 
//    tenable in the long run.
//
//     = As far as the game is concerned, the first persistent-flagged CELL that it 
//       loads is the persistent cell for a WRLD.
//
//     = We need to investigate how exactly the CK sorts REFRs into persistent and 
//       temporary CELL GRUPs, and how this differs for interior versus exterior cells.
//
//     - If we get rid of the group type, then that frees up one byte. We have three 
//       bytes to spare already, so that leaves us with enough room to let a WRLD form 
//       stub explicitly specify its persistent CELL's form ID.
//
//     - If we make changes in this regard, then we need to update the form stub docs.
//
//  - Design flaw in (file_load_order::save_active_file): save error information is 
//    stored on the (save_error) member of the load order. This means that the notice 
//    code (cannot_save_right_now) is not reliable: if you attempt one save operation 
//    while another is in progress, then the in-progress save operation will overwrite 
//    the error information for the blocked save operation, possibly before the latter's 
//    caller can view it. We should rename and repurpose (tes_file_writing::write_config) 
//    as a "save request" or "save process" struct, and have it retain error information.
//
//  - DovahKit currently has a bit of a design flaw with respect to invalid references 
//    to unoccupied form IDs. Consider the case where I load a file with a SHOU/SNAM 
//    that refers to some form xx001234, but there is no form with that ID. Then, in 
//    DovahKit, I create a new form with ID xx001234 and then I load the SHOU. Well, 
//    DovahKit has no way to know that the SHOU wasn't originally referring to the 
//    new form. If the new form isn't of a type that SHOU/SNAM can reference, then 
//    the user still gets a warning, but if the new form is, say, a spell or word of 
//    power, then we have a situation where the SHOU's data has silently changed, 
//    probably in a way that'll break things.
//
//    There's really only one way to address this, and it's gonna be messy. We'd have 
//    to create form stubs not only for *defined* form IDs, as we do now, but also for 
//    any *referenced* form IDs. This would allow us to track use info for form IDs 
//    instead of just for forms, which would be necessary for knowing that a given 
//    form ID is referenced despite not being used on a real form; it would also allow 
//    us to know whether a given form-ID-sans-form used to be referenced but isn't any 
//    longer (because if an inbound reference is severed by user action, then use info 
//    will be updated appropriately).
//
//    The form stubs for referenced-but-undefined form IDs would need to be generated 
//    during the use info build step. They could use form_type::none as their form 
//    type, and be sorted into file_load_order::forms_by_type[form_type::none]. We'd 
//    then need to make two changes. First: when searching for an available form ID, 
//    if we find that the ID is in use by a stub with no inbound references and a 
//    "none" form type, then we need to consider the ID available; and second, when 
//    actually taking an action that would place a valid form in that ID, we need to 
//    delete the "none"-type stub (which in turn requires telling any referencing 
//    forms to sever their references to it, and those forms may be of types that 
//    DovahKit doesn't yet know how to edit, so that's a new potential point of 
//    failure for form creation and renumbering).
//
//    Another nice bonus: if a form refers to an undefined non-zero form ID, then we 
//    can generate a warning for that using the same code we already have for catching 
//    type-mismatched form-to-form references. We'd just need to edit the error text 
//    for the case where the referent's type is form_type::none.
//
//     - TASKS
//
//        - (file_load_order::find_first_free_form_id_in_active_file) should take a 
//          bool that will allow us to choose whether to avoid any none-stubs. Default 
//          behavior is to not even bother.
//
//           - Avoiding form stubs requires us to check the normal form map, not just 
//             the active file form map, as none-stubs shouldn't go in the active file 
//             form map.
//
//        = TEST THAT FORM CREATION PROPERLY DESTROYS ANY NONE-STUBS THAT ARE IN 
//          THEIR WAY.
//
//        = TEST THAT FORM RENUMBERING PROPERLY DESTROYS ANY NONE-STUBS THAT ARE IN 
//          THEIR WAY.
//
//     - It's tempting to do things like making (file_load_order) refuse to grant 
//       access to none-type stubs, but that's a non-starter. First of all, those 
//       stubs will already be accessible through the loaded form data of the 
//       referencing forms. Second of all, it could actually be useful to allow 
//       things like calling (file_load_order::for_each_form_of_type) with the 
//       "none" type, in order to get a list of all referenced-but-undefined form 
//       IDs.
//
//     = While I'm here: things like this may make it tempting to do form-to-form 
//       reference error checking in the initial use info build step, in order to 
//       catch and log errors as early as possible, but I'm actually extremely not 
//       a fan. Why? Because we still wouldn't be logging any other errors, and it'd 
//       add redundancy between the load code and the use info build code. If we 
//       really want to give the user the option to scour the file for every possible 
//       data error on load, we can implement it by just loading and unloading every 
//       form's full data (possibly with multi-threading if we can manage that) and 
//       letting the on-demand load functions log the same warnings they would at any 
//       other time.
//
//  - Add support for loading DOBJ and GMST records properly.
//
//     = MAINTAIN OUR INTERNAL DOCUMENTATION ON THIS, UNTIL WE'RE DONE WITH IT.
//
//     - Some of the GMSTs that we extracted with a script have names formatted like 
//       INI settings. Double-check that these really are GMSTs. Ctrl+F the listing 
//       for ":" to know which ones to check.
//
//     - Skyrim Special Edition adds more GMSTs. We'll need to find a list of those 
//       definitions, and we'll need to make it possible for GMST definitions to 
//       indicate what games they appear in (defaulting to "all").
//
//        - Our current list is every Skyrim Classic setting, pulled directly from 
//          the game engine with no GMST records loaded -- all pristine executable-
//          level defaults.
//
//     - GMST renumbering: test all error cases.
//
//     - The GMST loader needs to warn on the following, and currently doesn't:
//
//        - Settings that share form IDs with each other
//        - Settings that share form IDs with real forms
//           - These won't load reliably because we multi-thread file loading on a 
//             GRUP by GRUP basis. Fixing that would require more intensive changes, 
//             but if anything that's even more incentive to add a warning in the 
//             event that we load them successfully.
//
//     - Singleton form support
//
//        - Our current system isn't going to manage use info super well. If you 
//          check the use info of a form used by DOBJ, but there are multiple form 
//          stubs for DOBJ, then there may be multiple using DOBJs listed. That may 
//          get awkward.
//
//     - DOBJ records are coalesced into a singleton. That singleton subclasses the 
//       TESForm class and so it does have a form ID.
//
//        = If there is no DOBJ form and we fail to create one due to there being 
//          no form IDs available, then we don't emit any errors. That said, that 
//          should be impossible since DOBJ is hardcoded to form ID 0x00000031 by 
//          default.
//
//     - NAVI
//
//        - If files define their own NAVI with a different form ID, then what form 
//          ID does the final loaded NAVI use? This doesn't matter for DovahKit, but 
//          I'm curious.
//
//  - Clean up the save process.
//
//     - If the user is converting the active file between games, and the active file 
//       contains any forms or overrides whose types don't exist in the target game, 
//       then DovahKit should show an additional confirmation prompt warning of this 
//       before saving.
//
//        - The confirmation prompt should be a custom window that lists each of the 
//          relevant forms, along with access to their use info.
//
//           - IIRC the save window is modal, in part to prevent the user from making 
//             further edits during saving, so if we do this, then we need to make it 
//             possible to configure the use info dialog so that it doesn't allow the 
//             user to actually access/edit the listed forms. We'll want to block off 
//             that access when the use info dialog is opened from this particular  
//             confirmation prompt.
//
//        - If the user proceeds with the save operation, then the full warning 
//          should be written to a log window and to a log file. The log file should 
//          also list all users of these forms, and it should specify that those 
//          users may log their own errors.
//
//     - Logging save warnings from forms
//
//        - Form components need to receive the "form save" interface so that they can 
//          also log warnings.
//
//        - Audit existing form types and extra data types, and add any appropriate 
//          save warnings, particularly for converting between Classic and Special.
//
//        - Do we want to store "local" form IDs in the save warnings, as we do for read 
//          warnings?
//
//           - How much can we share between the two warning structs? It'd be nice if we 
//             could sort of centralize and standardize things instead of having bespoke 
//             warning and error codes and structs everywhere.
//
//  - Support for loading records flagged as "partial"
//
//     - If the flag is present on an injected record, then skip the record.
//
//     - If the flag is present on a non-injected non-override, then it should be stripped.
//
//     - The loaders for WRLD, CELL, and DIAL need to check for the flag (they have access 
//       to the record reader, so they should be capable of this) and if it's present, then 
//       they need to match the behavior of the appropriate TESForm::LoadPartial override 
//       in TESV.exe.
//
//        - Wondering if we should: change (EveryFormSubclass::load) into a virtual member 
//          function (_load_impl); add a (_load_partial_impl); and then add (Form::load) as 
//          a non-virtual function that checks the record flag and calls the appropriate 
//          underlying virtual member function.
//
//     - Form stubs need to either cache the "partial" flag, or store record flags for each 
//       entry in their file list. See, in order to save a child form, we also need to save 
//       its parent and ancestor forms; however, if those forms haven't actually been edited, 
//       we can simply save them as "partial" records to avoid actually overriding them. how 
//       do we determine whether we've saved the parent form? well, we need to check whether 
//       it was defined or overridden in the active file, but we also need to double-check 
//       that any such override was not already partial, and that requires access to the 
//       record flags.
//
//        - The "save" code for parent forms will *also* need to check the record flags, 
//          which means that the flags need to be set before we call (Form::save).
//
//  - Form renumbering
//
//     - Now that form stubs store all loaded files' offsets, we can identify active 
//       file forms without having to rely on their load order prefix. Accordingly, 
//       we should allow record injection.
//
//        = Do we even handle loading and re-saving injected records properly?
//
//        = Incidentally, we can tell whether a record is injected by checking whether 
//          its load order prefix matches that of any of the files in its stub's source 
//          file list. We should implement a getter for this with the signature 
//          (bool file_load_order::form_is_injected(const form_stub&) const noexcept).
//
//        - There are three functions used to check whether a form or form ID is 
//          from the active file. The first two in this list use the load order 
//          prefix, while the last uses the form stub's file list. In what situations 
//          are these functions used? Does anything need to change in order for us to 
//          handle injected records properly?
//
//           - file_load_order::is_active_file_formID
//           - file_load_order::is_defined_in_active_file
//           - file_load_order::is_defined_or_overridden_in_active_file
//
//        - Record injection needs to be extra careful to avoid form ID conflicts.
//
//        - We should also amend GMST renumbering to allow injecting those.
//
//  - Finalize for Skyrim Special Edition.
//
//     - Implement ESL support.
//
//        - Saving an ESL should warn if the active file would have any CELL records, 
//          whether they be new forms or overrides. Reportedly, CELLs in ESLs have 
//          issues, though I don't know the source or the specific problems offhand.
//
//           - GamerPoets here <https://youtu.be/g_urrHrGQOY?t=299> recommends against 
//             ESL-flagging files that have interior cells, but gives no explanation 
//             as to why. Per aers, CELLs in ESLs are always loaded as if they're in 
//             0xFE000xxx, and per Parapets there is some issue that occurs if an ESL 
//             edits a CELL that originates from another ESL. We'll probably want to 
//             only warn when saving an ESL that overrides another ESL's cells.
//
//  - Reverse-engineering
//
//     - A worldspace's persistent cell is the first persistent-flagged child cell to load.
//
//     - The game reuses a single NavMeshInfoMap for all NAVI, even ones that don't override 
//       a prior NAVI. However, non-overrides wouldn't go through the normal data-clearing 
//       process that happens when overrides occur (though NAVI may just have that as no-ops 
//       anyway). It's worth checking a few things, then: would a non-overriding NAVI change 
//       the form ID of the baseline NAVI, and is there any data that should be cleared in 
//       an override (that therefore wouldn't be cleared by a non-override)?
//
//  - Multiple-file form loading
//
//     - Examine all form types except SHOU (we already checked that one): double-check to 
//       see what data they clear when loading overrides, and modify our load code to act 
//       consistently with what we find.
//
//        - Use Info generation also needs to behave consistently.
//
//  - Support for special-case relationships between records
//
//     - WRLD can only have one persistent cell, and it's the first persistent-flagged 
//       child CELL to load.
//
//     - DIAL needs to maintain a list of all of its child INFOs outside of the loaded 
//       form. It's my understanding that INFO/PNAM is just used to positioning a 
//       TESTopicInfo within its TESTopic's info vector, which means that our existing 
//       setup (relying solely on use info to link a topic to its infos, and the infos 
//       to their siblings) is not adequate. The form stub for DIALs will need to be able 
//       to store a dedicated list of INFOs, and we'll need to build this list during 
//       stub generation. We can generalize this situation -- allow form stubs to 
//       optionally store an "ordered child form list."
//
//  - Support for reverting forms, and for removing an active file's overrides
//
//     - What if it were possible to right-click a form in the object window, mouse over 
//       a "Revisions" context menu item, and pick which file (i.e. which record/override) 
//       you want to work with?
//
//       Implementating this would require a few considerations:
//
//        - Form stubs would need to be able to store *which* file they've been told to 
//          load from. An int16_t (with -1 meaning "use latest file") should do the trick.
//
//        - Reverting a form stub should flag it as edited.
//
//        - If we "revert" a form stub, then we need to clear all of its outbound use 
//          info, and then use loaded_forms::Form::generateUseInfo on the selected file 
//          to rebuild use info as it existed for that revision.
//
//           - We'd also need to re-load the form's editor ID, along with any other data 
//             that gets stored on the stub during the initial stub-build process.
//
//       There are also UX considerations:
//
//        - Should a "reverted" form be treated as edited and written into the active file? 
//          This would effectively create either ITMs (when reverting to the last non-active 
//          record) or allow easy reversion of changes made by other mods.
//
//       But there are UX benefits:
//
//        - The backend tech needed for this could easily be used for undoing overrides 
//          that exist in the active file. Currently, the Creation Kit lets you do that via 
//          the Data menu, but it requires a full reload (i.e. you aren't "reverting an 
//          override" so much as you are "electing to not load a particular override").
//
//     - We'll want this for GMST as well, but that'll require different underlying code. 
//       This could possibly include deletion of a GMST form stub, with the caveat that a 
//       form could (incorrectly) have a reference to such a stub (same issue as with 
//       renumbering a GMST).
//
//  - Localized string support
//
//     - The UI needs to physically prevent the user from entering glyphs that are not 
//       available in the current language. Alternatively, textboxes with bad glyphs 
//       should be given a red outline, and the "OK" button should be greyed out.
//
//        - If the user clicks inside such a textbox, a speech bubble should appear 
//          listing the bad glyphs. ("You are editing a [LANGUAGE] file. The following 
//          symbols are not available in [LANGUAGE]: a L 7 Q v")
//
//        - I've written QTextEncodingValidator, but it may not be entirely sufficient 
//          for this. Or maybe it may. Hm.
//
//     - We should create a custom promoted widget for editing localized strings, so 
//       that if we implement STRINGS file editing in the future, we can add a "..." 
//       button that the user can click to edit localized string content.
//
//  - There is no UI path to view or edit use info or data for worldspaces.
//
//  - Support for REFR
//
//     - Test resaving REFR.
//
//     - All of the placed projectile records are just direct subclasses of REFR 
//       and load all of the same things. Implement them the same way we implemented 
//       ACHR.
//
//  - Code for deleting forms.
//
//     - Test the severing of outbound references to a deleted form.
//
//        - Test keyword lists in specific.
//
//     - ObjectReference friendly delete question: why are actors that are "deleted" 
//       in this manner flagged as persistent?
//
//        - Someone used git blame and found that it dates back to the initial Github 
//          commit, i.e. when xEdit was migrated from an older source control system.
//
//        - REFRs defined in ESP files are de facto persistent, so this would have to 
//          be a fix meant explicitly for ESMs, no? Or do ACHRs behave differently?
//
// THINGS TO LOOK INTO:
//
//  - Build a unit testing framework wherein we run automated correctness checks 
//    on loaded data, use info, etc., for pre-chosen forms and compare the results 
//    to data prepared in advance. We should run these tests periodically if not 
//    regularly, in order to catch unexpected regressions.
//
//     - TESTS TO RUN:
//
//        - Change a REFR's base form in-editor and ensure that we properly update 
//          use info. We know that the connection between the REFR and its old base 
//          form will be severed, and a new connection between the REFR and its new 
//          base form will be established, but we need to test to verify that the 
//          new connection has the appropriate flags. When you view the use info 
//          window on the new base form, the modified REFR should be listed in the 
//          bottom pane (for references), not the top pane (for general uses).
//
//        - Create a file that overrides a DIAL, an interior CELL, and a WRLD, and 
//          nothing else. Ensure that these forms, only these forms, and not any of 
//          their children are saved to the file.
//
//        - Create a new file that overrides an INFO within a master's DIAL, a CELL 
//          within a master's WRLD, a REFR within a different one of the master's 
//          WRLDs, and an ACHR within an interior cell. Ensure that the overridden 
//          forms and their parents/ancestors are both properly saved to the file.
//
// UPCOMING TASKS:
//
//  - Add a "Windows" menu to the main window. It should list all open windows and 
//    allow the user to bring them to the front.
//
//     - This includes Use Info and form-editing dialogs. But what if the user has 
//       too many of those open?
//
//        - One thing's for sure: scrolling menus are a pain. We should see if we 
//          can force a two-column one when items overflow, instead.
//
//     - Should we convert Use Info and form-editing dialogs to QMdiSubWindows?
//
//  - UI for editing Papyrus script data
//
//     - We have a loader for ACTI; we can write form save code and then build a UI 
//       for it, and then test Papyrus editing.
//
//     - Once this UI is working, retroactively add it to the existing UI (if any) 
//       for CELL, FACT, and WRLD, respectively.
//
// DISTANT TASKS:
//
//  - Refhandle usage tracking: the number of persistent references in ESMs, and the 
//    number of all references in non-ESMs, should be tracked and stored on each 
//    file_reader. This information should be accessible to UI code.
//
//     - We may want to do a few things, so that we can isolate file_reader objects 
//       away from frontend code while still being able to display file-specific 
//       stats (and make those stats available for editing):
//
//       a) Define a file_stats struct, which would also appear a field on the 
//          file_reader class. This struct can contain, among other things, stats 
//          on refhandle usage.
//
//       b) Give DovahKitCore an accessor that returns const file_stats&.
//
//     - If the load order exceeds the game's refhandle limit, we should display 
//       appropriate warnings to the user; we should also point out that exceeding 
//       the refhandle limit breaks the Creation Kit as well. (Good thing we're not 
//       using refhandles ourselves!)
//
//        - We don't have a warnings dialog at present, and that's something that 
//          would be valuable to have just in general.
//
//  - The user needs to be able to pick which game (Skyrim Classic or Skyrim Special) 
//    they want to open files from. Currently, we just always use the Skyrim Classic 
//    install path.
//
//     - We need a bool on file_load_order indicating whether it's dealing with Classic 
//       or Special. This will affect whether it respects the "light" flag in file 
//       headers, whether it reserves a load order slot for ESL forms, and so on.
//
//        - This bool need to be flipped to "Special" if the user loads files for 
//          Classic and chooses to save the current active file as an SSE file. If 
//          the current load order has 254 files in it, then the save operation 
//          should fail (as the 0xFE slot would then conflict).
//
//        - Form loading/saving will still depend on each given file's version.
//
//     - We need to support ESLs.
//
//  - Miscellaneous technicalities
//
//     - When saving WRLD/CELL/REFR, if the REFR is persistent, then it should be saved 
//       into the worldspace's persistent cell instead of into a normal exterior cell.
//
//        - Are we sure? On what basis does the CK itself do this? Test flagging a REFR 
//          (in a normal exterior cell) as persistent in xEdit and see whether the CK 
//          moves it when resaving. Test whether this occurs even when there isn't a 
//          persistent cell to start with, and test whether the CK always generates a 
//          persistent cell even when there are no persistent refs in the worldspace.
//
//        - If a WRLD doesn't have a persistent cell, and it contains any persistent 
//          REFRs, then a persistent cell must be created.
//
//     - REFRs that have the "deleted" flag set at save time need to be subject to the 
//       undelete-and-disable procedure, as deleted REFRs can crash the game on exit and, 
//       if subsequently overridden, can reportedly crash when loaded.
//
//        - Presumably, deleted NAVMs need to be handled the same way, with the added work 
//          of making them small or zero-size. However, no one seems to know how to then 
//          update the associated NAVIs. xEdit's online documentation states that deleted 
//          navmeshes cannot be automatically corrected and must be fixed manually.
//
//  - Support for DIAL and INFO
//
//     - DIAL/QNAM should use dialogue_form_id_t instead of form_id_t. Use info generation 
//       for this subrecord should use the "dialogue" use info entry flag.
//
//     - INFO/PNAM should use dialogue_form_id_t instead of form_id_t. Use info generation 
//       for this subrecord should use the "dialogue" use info entry flag.
//
//  - Form deletion: some form types skip saving some subrecords if the form is flagged as 
//    deleted. Should we replicate this behavior? We'll want to continue testing in the 
//    Creation Kit to find all of these, if we care that much.
//
//     - REFR/DATA
//
//  - form_id_t::set and its overrides on subclasses should take a form_stub&, not a 
//    form_stub*, as the pointer is not optional.
//
//  - Code for changing a reference's base form
//
//     - What do we do if this would lead to a change in the reference's own form type, 
//       as in the case of converting between REFR and ACHR, or REFR and a PHZD? Should 
//       we even allow those kinds of changes? We can't simply turn one loaded_form 
//       instance into an instance of a different class.
//
//        - We probably shouldn't allow those sorts of changes, to be honest. Base form 
//          swaps should not be allowed to change a reference's form type. We'll just 
//          need to be sure to indicate that in the UI.
//
//  - Lua scripting
//
//     = DEVELOPMENT ROADMAP
//
//        - PHASE 1: BASIC ENVIRONMENT
//          Lua scripts should run in a separate thread and should exchange messages 
//          with the main thread. The script thread may be made to wait on an operation 
//          occurring in the main thread, but the main thread should never wait on the 
//          script thread. The user should be presented with a modal window that acts 
//          as the root UI for scripts; this should include a button to forcibly kill 
//          the script early.
//
//        - PHASE 2: FORM ACCESS
//          Lua scripts need to be able to refer not only to forms, but to individual 
//          parts of forms. Scripts should not use form IDs as the sole identifier of a 
//          form, as a form ID could be reused if a form is deleted and then a new form 
//          is created (or an existing form is renumbered). Additionally, variables that 
//          point to parts of a form (e.g. a particular property on a particular script 
//          attached to a particular form) need to be properly managed.
//
//        - PHASE 3: UI ACCESS
//          Lua scripts should be able to spawn windows and widgets, and should be able 
//          to manipulate them and respond to important events by way of Qt signals and 
//          slots. The Lua VM should not be killed until after the script has run to 
//          completion and all script-spawned windows (that have been shown) have closed.
//
//     - The Lua VM should be started up when the script begins, and should be killed 
//       after the script has run to completion and all script-spawned windows (if any) 
//       have closed.
//
//     - Scripts should run in a second thread, and the user should have the option to 
//       forcibly terminate the script early, as a way of dealing with scripts that 
//       have frozen.
//
//       In order to terminate a script early, we need to set a debug hook function to 
//       run every few script instructions (i.e. a count hook set up with lua_sethook). 
//       This hook function will need to check an atomic "abort" bool stored outside 
//       of Lua (i.e. on a singleton); if the bool is true, then the hook function 
//       should raise an error within Lua. If that error isn't caught, then it'll 
//       end up halting the script properly.
//
//       So how do we make sure that the error isn't caught? Well, scripts can catch 
//       errors using pcall and xpcall, so we can just make sure to only provide a 
//       script with shimmed versions of those functions that have been rigged to 
//       never catch the bogus error that we raise when trying to abort a script. 
//       (Note that we need to check the content of the error AND the "abort" bool: 
//       the debug hook only runs every few instructions, so it's possible for a 
//       normal Lua error to occur before we see and react to the "abort" bool.)
//
//        - The debug hook only runs every few Lua instructions, so if the script 
//          calls some long-running C function, then the "abort" bool won't be 
//          checked until after that function runs. This means that if we provide 
//          any long-running C functions to the script, those need to also check 
//          the "abort" bool and terminate early if possible.
//
//          As an example, if we wanted to provide a sleep(ms) function, then we'd 
//          want to program it roughly like this:
//
//             for(int slept = 0; slept < ms; slept += 50) { // sleep in 50ms increments
//                Sleep(50);
//                if (abort_bool)
//                   return;
//             }
//
//     - The script execution window should consist of a status message and progress 
//       bar. Scripts should be able to set the status message, the progress bar 
//       bounds, the progress bar current value, and the progress bar state (i.e. 
//       it should be possible to recolor the progress bar to represent "running," 
//       "paused," and "stopped," as one can with the Windows taskbar button progress 
//       bar). Additionally, setting the progress bar bounds to a zero width should 
//       show an "indeterminate" animation.
//
//        - If we really want to go the extra mile, we can have a log panel that 
//          shows *every* call into any of the script APIs that we provide.
//
//     - Lua variables cannot be allowed to point directly to form_stubs or to data in 
//       loaded forms, as we can't (easily) update all Lua variables as form data is 
//       modified. Instead, the script singleton should retain a map of "handles" to 
//       form information, and Lua variables should hold handles.
//
//       Handles should NOT include a form's ID, because forms can be renumbered, and 
//       because two different forms could have the same ID if they exist at separate 
//       points in time. Form IDs (as well as form_stub pointers) should only be kept 
//       in the form information that a handle maps to.
//
//        - The backend doesn't allow you to delete a form_stub if its form is loaded, 
//          so the script singleton will need to abandon all loaded_form_ptrs before 
//          deleting a form.
//
//        - It also needs to be possible for a Lua variable to refer to an individual 
//          part of a form, such as a Papyrus property or a quest alias. This, too, 
//          needs to rely on handles rather than absolute indices or alias IDs, for 
//          similar reasons to form IDs. (This is along with the added complication 
//          that Papyrus properties are sequential. If variable `foo` refers to the 
//          zeroth Papyrus property and `bar` refers to the first, and a separate 
//          piece of code deletes the zeroth Papyrus property, then the property that 
//          `foo` referred to will cease to exist and the property that `bar` referred 
//          to will become the zeroth property. In this situation, `foo` needs to test 
//          as referring to nothing, and `bar` needs to test as still referring to the 
//          formerly-first, now-zeroth property.)
//
//           - This needs to be designed in a generic way, so that individual form 
//             classes have executive authority over their parts. I don't want to have 
//             to hardcode every form part (words in a shout, aliases or stages in a 
//             quest, attack data entries in a race, and so on) into the script core.
//
//     - Scripts should be able to include Greasemonkey-style comments at the top of 
//       the file as a way of specifying extended configuration options. Options that 
//       we could support can include:
//
//        - [[readonly]]: The script will not be allowed to modify any loaded forms. 
//          This will affect the editor's messaging surrounding the script; for example, 
//          the user will not need to be warned about terminating the script early, 
//          because doing so cannot leave the active file in an inconsistent state 
//          (since the script isn't doing anything). This attribute is advised for 
//          scripts that are just examining, searching, or otherwise analyzing data.
//
//        - [[fileaccess]]: The script will be able to create new files in a sandboxed 
//          directory on the user's system.
//
//        - [[shellexec]]: The script will be able to ask for permission to run programs 
//          on the user's system. This could be useful for some niche tasks, but also 
//          [insert link to TheZZAZZGlitch's "TECHNO" demonstration here]. Ideally we'd 
//          still require the script to actually get the user's permission to run any 
//          particular program by way of a hardcoded confirmation prompt, but I don't 
//          know for sure how practical that'd be.
//
//     - Since we don't reveal every single form through a unified interface as xEdit 
//       does, it will need to be possible for scripts to define forms (as in sheets 
//       of values, not as in game data) that the user can fill out to provide values 
//       to the script (e.g. to tell it what forms, as in game data, to operate on). 
//       We *could* create Lua wrappers for creating and managing Qt UI, but that 
//       feels like it'd be very involved; a data format for describing "script 
//       arguments" could work better but would require us to account for every 
//       possible case. Bad trade-off either way, it looks like.
//
//     - Scripts should be able to spawn UI windows and widgets, and to register Lua 
//       functions to run in response to Qt signals on these widgets.
//
//        - There should be a cap on how many windows a script can spawn.
//
//        - There should be a cap on how quickly a script can spawn windows.
//
//        - All script windows should have a modeless relationship with DovahKit, so 
//          that scripts cannot block access to the editor UI.
//
//        - We'll probably want to provide generic table, list, and tree views to Lua.
//
//        = Lua-spawned windows must exist on the main thread (QWidgets can only 
//          function there), which means that in order to allow Lua to influence and 
//          be influenced by the UI, we must pass messages across threads. This, of 
//          course, introduces the issue of concurrency -- of what to do with a signal 
//          that Qt emits while Lua is already processing another signal.
//
//          The only reliable way to prevent these sorts of issues is to just straight-
//          up disable all script-spawned windows while processing a Lua event (and if 
//          we don't want them showing the disabled graphics, then we can disable their 
//          repaints via QWidget::setUpdatesEnabled as well). That means that we'll 
//          need to be very selective about what signals we make available to Lua; 
//          specifically, we can only allow Lua to respond to an event if disabling the 
//          UI during that response would not interrupt the user. As an example, Lua 
//          cannot respond to each individual keypress in a QLineEdit (as would happen 
//          with the built-in QLineEdit::textEdited signal), but rather must be given a 
//          custom "change" event that fires when the widget loses focus after having 
//          been edited (compare to JS "onchange").
//
//  - If the user has any unsaved changes, the main window should show a confirmation 
//    prompt on exit. We already override MainWindow::closeEvent; we'll want to do what 
//    we need to do in there.
//
//     - Checking for unsaved changes is easy: just see if any active file forms are 
//       flagged as edited.
//
//  - The (activate_ref) extra-data object has unknown fields, which are likely to 
//    include at least one form ID.
//
//     - Other extra-data types with unknown fields:
//
//        - distant_data
//        - package_start_location
//
//  - Render Window
//
//     - When looking at a base form's Use Info, double-clicking a reference in the 
//       listing should open its parent cell in the Render Window and select it, instead 
//       of opening the reference's form-edit dialog.
//
//        - ...but double-clicking [ACHR:00000014]PlayerRef, in the use info for the 
//          base [NPC_:00000007]Player form, shouldn't do anything at all.
//
//  - If a worldspace contains two cells with the same grid coordinates, then we need 
//    some kind of handling, especially since a plug-in like that would crash the CK. 
//    It's worth noting, though, that this should only be possible if the user is hex-
//    editing ES[LPM] files to cause havoc in the first place. Possible options:
//
//     - Force the user to choose which cell to retain when loading.
//
//     - Give the user the option to discard either of the cells, or transplant one 
//       of them to another grid coordinate. (The latter could be especially useful 
//       if we allow cells to be swapped, though that would have TONS of ramifications 
//       for things like navmeshing and navmesh info maps.)
//
//     - Refuse to render either cell in the Render Window (just show some kind of 
//       error indicator), and otherwise don't bother solving the problem or giving 
//       the user the option to solve the problem.
//
//  - Phantom has requested camera path editing, and pointed me to the GECK wiki as the 
//    sole known source of information on that.
//
// HORIZON TASKS:
//
//  - Seasonal/event themes, like we did for CobbPos
//
//  - Mod merging could be useful, particularly if we write a co-save file that describes 
//    the mapping of forms from the source files to the destination file (so that later 
//    re-merges produce consistent results).
//
//     - Being able to selectively merge in content, instead of merging all content in 
//       all source files, could also be useful. OpusGlass accomplishes this by using 
//       xEdit's "copy as new record into" function, though they acknowledge that this 
//       is suboptimal in that it doesn't allow you to merge in updates to the source 
//       files after the fact; it's a one-time thing only.
//
//  - Localized string editing support
//
//     - If we wanted to implement STRINGS file editing in the future, that would entail 
//       creating a localized_string_store for the active file if one does not exist, and 
//       writing code to export localized strings into multiple files.
//
//        - This could help in the case of overriding forms from files that also use 
//          localized strings, e.g. overriding Skyrim.esm forms, but only if the user 
//          has *all* STRINGS files for those TES files on hand. If the user is missing 
//          the file for that particular language, then we'd still generate a placeholder 
//          string for that language. A user could purposely avoid language mishaps with 
//          overrides of base-game content if they're careful to grab the STRINGS files 
//          for all languages. Whether that's easy to do, and whether Bethesda would 
//          allow users to circulate that content among themselves otherwise, is unclear.
//
//        - The downside to this approach is that Skyrim does not use internationalized 
//          fonts. Each localization only ships the glyphs that its language needs. This 
//          means that while TES files might show mojibake for unsupported languages, 
//          I'm pretty sure that STRINGS files would show nothing at all, which may or 
//          may not be worse.
//
// FINALIZING TASKS:
//
//  - Handle different locales. Complicated by the fact that ES[LPM] files use various 
//    system locales instead of UTF-8 or specifying a locale explicitly.
//
// STUFF I'M PROBABLY NOT EVER GOING TO BOTHER WITH:
//
//  - Being able to load files as part of the current load order, without making them 
//    dependencies of the active file when saving; or, being able to prune the active 
//    file's dependency list when saving.
//
//     - Would require being able to check that no active file forms refer to content 
//       in a given dependency.
//
//     - Would require shuffling the form IDs used for GMSTs, if those form IDs are from 
//       a dependency.
//
//  - TESV.exe reacts to a file's endianness. If a file has all endianness swapped, such 
//    that it begins with a "4SET" record, then TESV.exe will byteswap all fields as 
//    they're loaded such that the file is still valid. The executable doesn't think in 
//    terms of "big-endian" versus "little-endian," but rather in terms of "same-endian" 
//    versus "different-endian:" it compares the signature of the TES4 record to an in-
//    memory constant; if that doesn't match, the game BSWAPs and tries again; and if 
//    that matches, then it will BSWAP everything.
//
//    This is probably done to account for different endianness on different platforms, 
//    i.e. the console releases.
//

int main(int argc, char* argv[]) {
   QApplication a(argc, argv);
   //
   #if _DEBUG // log all qt resources to the "output" tab in the debugger
      qDebug() << "Dumping list of all Qt resources...";
      QDirIterator it(":", QDirIterator::Subdirectories);
      while (it.hasNext()) {
         qDebug() << it.next();
      }
      qDebug() << "All Qt resources dumped.";
   #endif
   //
   MainWindow w;
   w.show();
   return a.exec();
}