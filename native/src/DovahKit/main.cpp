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
//  - If the load order has been configured to load Skyrim Special Edition files, 
//    then the load order should be capped at 253 entries, not 254.
//
//     - It's tempting to go by the individual files' versions, but remember: we 
//       have to load Special files from a different location than Classic files. 
//       Don't bother mucking around with file versions; just put a flag on the 
//       file_load_order and use that.
//
//  - tes_file_reading::subrecord::to_string is unintuitive; that should just be a 
//    template specialization of subrecord::read.
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
//  - Clean up the save process.
//
//     - The (file_load_order) class should not have a bool member which indicates 
//       that ESL support is enabled; rather, it should have an enum indicating which 
//       game the current load order is for, and ESL support should be a property 
//       deducible from that game. We don't really need to change anything else here; 
//       toggling whether ESL support is enabled basically is the exact same operation 
//       as toggling what game we're processing for.
//
//        - Let's define a game enum in core.h.
//
//        - This will also simplify, somewhat, the process by which the frontend 
//          keeps track of what game it's currently operating on.
//
//        - In general, the entire save process needs to be reoriented around saving 
//          content for X game or Y game. It should still be possible for a frontend 
//          to specify individual options (e.g. form version) if it wants to, but the 
//          default -- the path of least resistance -- should be to just pick a game 
//          and then have all sensible configuration for that game set up automatically.
//
//           - This also means that the "write_config" struct used for saving needs to 
//             be made mandatory.
//
//     - If the user is converting the active file between games, and the active file 
//       contains any forms or overrides whose types don't exist in the target game, 
//       then DovahKit should show an additional confirmation prompt warning of this 
//       before saving.
//
//        - This, and all other warnings encountered during a save operation, should 
//          be listed in a log window and written to a log file.
//
//        - The warning should show a full list of affected forms, with access to 
//          their use info. In turn, the log file should list all users of these 
//          forms as well, and it should specify that those users may log their own 
//          errors.
//
//     - If the user is converting the active file between games, then the code for 
//       serializing a (form_reference_t) needs to check the form type of the form 
//       stub being referenced. If the referenced stub is of a type that doesn't 
//       exist in the target game, then we need to write form ID 0 instead.
//
//        - Consider the case of a VOLI form referenced by a FormList, in a file that 
//          we are converting from Skyrim Special to Skyrim Classic. We're not going 
//          to be serializing the VOLI itself, so we shouldn't leave a dangling form 
//          ID in the FormList. Serializing none isn't ideal (it might be better to 
//          skip the entire entirely) but it is technically valid and it's the easiest 
//          thing to implement.
//
//        - Don't bother implementing an automatic warning for this. The warning for 
//          the referenced forms themselves (i.e. the warning that they'll be deleted) 
//          oughta be enough.
//
//     - If the user is converting the active file between games, and the active file 
//       contains any forms or overrides whose types don't exist in the target game, 
//       then those forms' stubs need to be deleted from memory after a successful 
//       save operation. This will also entail severing references to them -- pretty 
//       much the same as if a user voluntarily deletes a form.
//
//        - The warning shown to the user needs to make it clear that we will not 
//          only skip these forms during saving, but also remove them from memory 
//          if the save operation completes successfully.
//
//        - Let's think in more general terms: any form_stub that wasn't serialized 
//          to the file (i.e. has no fixup data in the file_writer after a successful 
//          save) needs to be deleted.
//
//        = This needs to happen because after a successful save operation, we swap 
//          out the active file -- so, form data is unloaded (unless something else 
//          was already using it) and will be loaded from the newly-saved file next 
//          time it's requested. This means that forms that are not saved to the 
//          file can no longer be loaded for further editing.
//
//     - If the user is converting the active file between games, and a loaded form 
//       contains data that cannot be serialized in the target game, then it needs 
//       some way to report a warning. Form::save and Form::_save_impl should receive 
//       a second argument: a file_save_process& that can be used to log warnings. 
//       (The name is generic so that we can give it other duties in the future if we 
//       need to.) It would be perfectly acceptable for this to just be a wrapper or 
//       interface to file_writer.
//
//        - I don't want the backend to have to localize its error strings, but any 
//          strings that need to be shown to the user need to be localizable. Let's 
//          implement a WinAPI-style error code enum. I want this to also cover 
//          warnings, so we'll call it a "notice code." We'll define it in two parts:
//
//           - We'll use (using notice_code_t = uint32_t) in a common header, and all 
//             functions that send or receive notice codes will use that type.
//
//           - The actual (notice_code) enum will be a scoped enum defined in another 
//             file, which uses (notice_code_t) as its underlying type. This file will 
//             only be included directly in places that are returning specific values 
//             or checking for specific values.
//
//           - Let's use the high bit of the notice code to differentiate warnings 
//             from errors.
//
//          Doing things this way means that we can add new notice codes without 
//          having to recompile *quite* everything within ten square miles of the 
//          notice code type.
//
//        - What we want, then, is for forms to be able to log warnings; each warning 
//          should be a struct with a notice code and some optional generic details 
//          about the warning (e.g. other form IDs, etc.).
//
//           - Form IDs in this struct should be listed in both "global" and "remapped" 
//             format, i.e. the IDs as they exist in memory and the IDs as they would 
//             exist in the saved file. In fact, let's take that approach for all of 
//             the warnings we log, yeah?
//
//  - Finalize for Skyrim Special Edition.
//
//     - Implement ESL support.
//
//        - The max file count is enforced in (file_load_order_normalizer). We need 
//          to move the check to (file_load_order), and only enforce it if an active 
//          file is set.
//
//           - Skyrim Classic loads should always fail if we have 255 files i.e. if 
//             we break into slot 0xFF.
//
//           - Skyrim Special loads should always fail if we have 254 non-light files 
//             i.e. if they break into slot 0xFE, or if we have 4097+ light files i.e. 
//             if they exceed the light slot range.
//
//           - Loads should always fail if there is an active file and more than 254 
//             files prior to it such that the active file would encode its own forms 
//             as 0xFF when saved. This should be enforced regardless of game.
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
//  - 9/14/2020: Very rare crashes on exit. One access violation seen; two 0xC0000374 
//    seen (the latter is memory mismanagement). No consistent repro steps, and it's 
//    not like there's a lot you can do in the program as of this writing (the file 
//    load dialog is finished except for being unsorted). Let's keep an eye out for 
//    more issues like this. It's not consistent, which means it isn't the result of 
//    something we're doing consistently; it's random chance or maybe an edge-case in 
//    in the UI somewhere.
//
//     - Made some preemptive tweaks to memory management, including having the 
//       file_load_order only discard its file_readers after the form_stubs instead 
//       of before. For now, let's just try to stay alert for this.
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
//  - ESL Support
//
//     = The current state of ESL support is that we treat ESLs the same as ESPs and 
//       ESMs. They are not placed at load order slot 0xFE and do not share a load 
//       order slot, and we take no steps to prevent them from being used as masters  
//       for other files. In other words, we don't *have* ESL support.
//
//     - Currently, (file_load_order) stores all loaded files in (file_load_order::files). 
//       For ESL support, we'd need two separate lists. We'd also need to amend form ID 
//       resolution.
//
//     - ESLs are not allowed to be dependencies of other files. This means that we'd 
//       need to make some pretty big modifications to DovahKit. We have two options:
//
//       a) Do not allow the user to load an ESL unless it is both the only ESL to load 
//          and the active file.
//
//       b) Allow the user to load one or more ESLs without them being the active file, 
//          but take steps to prevent the user from modifying any forms that originate 
//          from an ESL, and take steps to prevent the user from modifying any other 
//          forms in such a way that they refer to ESL-sourced forms.
//
//           - If the active file is an ESL, then the user should be able to modify 
//             forms in that ESL only, and make forms defined or overridden in that ESL 
//             refer to other forms in that ESL only.
//
//              - This requires that it be possible to query whether two form_stubs 
//                belong to the same file, and query whether a form_stub belongs to an 
//                ESL file.
//
//           - When saving the active file, we'd need to exclude all loaded ESLs from 
//             the final list of masters to use. We'd also need the file-saving code 
//             to fail when serializing any references between forms that the UI should 
//             have prevented (in case the UI *doesn't* prevent them).
//
//              - This would require subrecord::_write_impl(const form_id_t&) to set 
//                error information on the owning file_writer. If we're doing that, 
//                then it'd be nice if subrecord::write could return a boolean so that 
//                a form can immediately abort a write if any error occurs.
//
//       Unless and until one of the above two approaches is implemented, DovahKit 
//       cannot be said to be compatible with ESLs even if we implement proper loading 
//       for ESLs.
//
// HORIZON TASKS:
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