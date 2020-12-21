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
//  - The log window should have a "message type" column, differentiating between 
//    notices from the initial stub build, notices from on-demand form loading, and 
//    notices from saves.
//
//  - Form duplication can fail in complex ways: even if we successfully duplicate the 
//    target form, we may fail to duplicate child or descendant forms. We should create 
//    a custom dialog box that can show multiple sets of error details for this case.
//
//  - Audit and document every process that can return notice codes, along with a full 
//    description of what notice codes can be returned and (in the case of any detailed 
//    notices) what other information may be present.
//
//     - The (editor_helpers::warning_or_error_to_string) function is missing several 
//       notice codes, presumably because they're handled in specific places. Fix this.
//
//  - Testing indicates that multithreading gives us diminishing returns. The "busiest" 
//    threads by far are the worldspace sub block threads, which *each* tend to have 
//    about 200 groups to process; however, doubling the number of threads didn't yield 
//    a significant improvement in Skyrim.esm's load time.
//
//    This would suggest that file_threaded_part_loader_base::heavy_duty_thread_count is 
//    superfluous. It's unused anyway, so maybe we should ditch it.
//
//  - Document the file read process in full, including the ways in which it is tangled.
//
//     - The (basic_reader) needs to be able to know about (file_reader) when used by 
//       one in order to allow (record) and (subrecord) to reach the (file_load_order), 
//       which is needed for form ID fixup and the loading of localized strings. I had 
//       wanted to separate things out with (basic_<element>) interfaces which subclass 
//       the (<element>) interfaces, but that's not feasible because the interfaces need 
//       to be able to return one another. Essentially, I need to be able to do something 
//       like the following, which C++ does not allow:
//
//          struct basic_foo {};
//          struct foo : public basic_foo {};
//
//          struct basic_bar {
//             basic_foo& get_my_foo();
//          };
//          struct bar : public basic_bar {
//             basic_foo& get_my_foo() = delete;
//             foo& get_my_foo();
//          };
//          
//       The deletion is needed in order to prevent the two versions of (get_my_foo) from 
//       being treated as overloads (which the compiler would then be unable to distinguish, 
//       since they differ only by return type); however, it seems that deleting the one 
//       unavoidably also deletes the other, presumably for the same reason. At least, 
//       according to IntelliSense.
//
//     - The (file_or_file_part_loader) class handles communication with (file_load_order), 
//       including error reporting and aborting a file load when any error is reported. These 
//       latter two tasks require it to be able to access the most pertinent instance of its 
//       own subclass, (file_loader): the former, to report the filename as part of the error; 
//       the latter, to carry out the abort. This means that every (file_loader) ends up with 
//       a pointer to itself. At least we're able to leverage the same (file_loader) pointer 
//       that (basic_reader) unfortunately has to offer.
//
//  - Rename (notice_code::game_conversion_form_cleanup_failed) to (unsaved_form_cleanup_failed), 
//    as there's nothing which says that a form can ONLY fail to save as the result of it 
//    being lost in a conversion between games.
//
//  - UI for editing Papyrus script data
//
//     - We have a loader for ACTI; we can write form save code and then build a UI 
//       for it, and then test Papyrus editing.
//
//     - Once this UI is working, retroactively add it to the existing UI (if any) 
//       for CELL, FACT, and WRLD, respectively.
//
//  - When loading the contents of cell GRUPs, we should log a warning if a record is 
//    misplaced (e.g. WRLD/VTYP, WRLD/CELL/VTYP, etc.). We already log warnings for 
//    misplaced records in top groups.
//
//  - Quick test: does saving a file as *.TES cause an assertion failure?
//
//     - The frontend doesn't allow it, but we should *probably* make sure it doesn't 
//       cause the backend to choke.
//
//  - On-demand form loading should emit a warning when loading a reference to a none-
//    stub. Remember to use the getter on form_stub; don't just check the form type, or 
//    we'll false-positive on the hardcoded "persistence forms."
//
//     - The issue is, we have to load the reference in order to clear it, so that's 
//       going to result in dumb errors.
//
//     - We actually already generate an error: this is detected as a type-mismatched 
//       reference.
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
//        - Audit existing form types and extra data types, and add any appropriate 
//          save warnings, particularly for converting between Classic and Special.
//
//  - Esoteric records
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
//     - Examine all form types except SHOU and FACT (we already checked those): check to 
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
//  - Localized string support
//
//     - The UI should physically prevent the user from entering glyphs that are not 
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
// THINGS TO LOOK INTO:
//
//  - Wrap-up and testing for partial record support
//
//     - If a GMST is flagged as "partial" and is an injected record, then it should fail 
//       to load. We only emit warnings for that sort of thing in (accept_form_stub), so 
//       GMSTs can't warn. The (accept_game_setting) function doesn't take a (form_stub) 
//       or a record header; the (file_or_file_part_loader) function would have to handle 
//       this, I suspect.
//
//     - When saving a form, we need to check whether it's been edited within the active 
//       file. If not, then we must only be saving it because it's the parent of an active 
//       file form, so we need to flag it as partial if it isn't flagged as such already.
//
//        - We need to save a parent form if any of its child forms need to be saved... But 
//          we can have multi-level relationships, e.g. WRLD/CELL/REFR. If the WRLD and CELL 
//          are both unedited and the REFR is edited or from the active file, then both 
//          the WRLD and the CELL should save as partial.
//
//     - Wondering if we should: change (EveryFormSubclass::load) into a virtual member 
//       function (_load_impl); add a (_load_partial_impl); and then add (Form::load) as 
//       a non-virtual function that checks the record flag and calls the appropriate 
//
//     - TESTopic::LoadPartial is a no-op and loads no data.
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
//  - Should we convert Use Info and form-editing dialogs to QMdiSubWindows?
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
//  - Lua scripting
//
//     = START MOVING A LOT OF THE EXPLANATIONS HERE TO A DOCUMENTATION TEXT FILE.
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
//           = DONE.
//
//        - PHASE 2: FORM ACCESS
//          Lua scripts need to be able to refer not only to forms, but to individual 
//          parts of forms. Scripts should not use form IDs as the sole identifier of a 
//          form, as a form ID could be reused if a form is deleted and then a new form 
//          is created (or an existing form is renumbered). Additionally, variables that 
//          point to parts of a form (e.g. a particular property on a particular script 
//          attached to a particular form) need to be properly managed.
//
//           = Wrapper implementation
//
//              - The current wrapper implementation will be slow once large numbers of 
//                wrapped objects are in play at a time. In order to return a wrapped 
//                object from a Lua API, we must scan the full list of extant wrappers 
//                to see if there already exists a wrapper for the object. This is 
//                because the internal weak-table of wrappers is structured like this:
//
//                   storage[autogenerated_key] = wrapper
//
//                We can potentially speed lookups up if we do something like this:
//
//                   storage.forms[formID][autogenerated_key] = form_data_wrapper
//                   storage.ui[autogenerated_key] = ui_data_wrapper
//
//                We'd have to manage that a bit more carefully -- detect when a 
//                table for a formID becomes empty and clear it, for example -- and 
//                instead of (storage) being weak, the individual maps for formIDs 
//                and the UI map would need to be weak.
//
//                 - If a form is renumbered, we'll have to renumber its weakmap as 
//                   well.
//
//                 - Since there are going to be multiple weakmaps, we could probably 
//                   save just a *little* bit on memory by having them share the same 
//                   metatable (it's the metatable that makes them weak).
//
//              - It needs to be possible to define additional wrapper types for each 
//                form type, all subclassing (form), and the script VM needs some kind 
//                of helper function that will make sure to return a wrapper of the 
//                correct type given a form stub. All code that currently returns forms 
//                will need to be modified to use that helper function.
//
//              - It needs to be possible to wrap individual components in a form.
//
//              - If the script deletes a form, then we can have the script VM flag 
//                the form's wrapper as "dead," but it'll still test as not being nil, 
//                and it'll still have callable member functions. The approach above 
//                should allow us to at least prevent further calls to its members, 
//                but can we either nil out the existing references or make them 
//                pretend to be nil?
//
//                 - The __eq metamethod will not save us.
//
//                 - This person used dirty hacks to loop over every single variable 
//                   in the running script and clear them as needed: <https://stackoverflow.com/a/14624223>
//
//           - The main thread needs to be able to send two kinds of messages to the 
//             script thread. "Urgent" messages would be things like form deletion, 
//             and the script thread must check for them at every opportunity: after 
//             sending any blocking message, during the debug hook, and when spinning 
//             (we'll get to that). "Normal" messages would be things like UI events, 
//             and the script thread should check for them only when spinning.
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
//       Options that grant access to advanced or dangerous functionality should result 
//       in the user being shown a confirmation message in the script selection window, 
//       i.e. the user is shown a list of permissions that the script is requesting and 
//       must explicitly grant those permissions before the script can run.
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
//        - UI widgets will need to be referred to using handles managed by our script 
//          singleton, similarly to form data.
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
//     - It'd be nice if scripts had access to a UI widget that would allow them to 
//       draw arbitrary rasters, like JS canvas (but maybe with a friendlier API).
//
//        - Sending each individual draw request across threads would be slow. It may 
//          be faster to maintain two copies of any given raster -- one within the 
//          widget, used for rendering, and one on the Lua thread which gets copied 
//          to the widget asynchronously. Each copy of the image could maintain a 
//          "last updated" time in ms, allowing us to know when a copy is needed. To 
//          avoid threading mishaps, the image data on the Lua thread would need a 
//          lock, to be used on writes and copies.
//
//        - Access to this widget should require permission, just because scripts 
//          could use it to draw rude things.
//
//  - World viewing
//
//     - Loading NIFs and being able to render them
//
//        - Use Qt's 3D drawing APIs for now. We can stress-test them and potentially 
//          dive into OpenGL or something later.
//
//        - The goal isn't to match the game exactly or even all that well. Really, we 
//          can just throw out all data besides verts, textures, and really basic shader 
//          properties. All we want is to be able to render the world *well enough* for 
//          basic viewing and editing in the future.
//
//     - Loading and unloading cells on-demand (along with their contained references, 
//       and the assets for those references)
//
//        = WE MUST IMPLEMENT SUPPORT FOR PARTIAL RECORDS FIRST.
//
//        - Requires a singleton to manage form and asset (un)loading. We should retain 
//          recently-unused forms and assets in memory for a brief while, to handle the 
//          case of the camera repeatedly panning across a cell boundary (e.g. due to 
//          rotation) and avoid having to repeatedly reload the same content.
//
//           - Singleton will also need to construct landscape meshes and water planes.
//
//           - Singleton needs to react to forms being created, modified, or destroyed.
//
//              - Creating/modifying/destroying references
//
//              - Modifying base forms used by references
//
//     - Rendering abstract elements
//
//        - Cell borders
//
//        - Navmeshes
//
//        - Collision-primitive references
//
//     - Reference selection and browsing
//
//        - References selected in the Cell View window should show bounding boxes in 
//          the Render Window. There should be both hotkeys and context menu options 
//          for rendering references intangible or invisible (compare to the "1" key 
//          in the Creation Kit).
//
//        - References in the Render Window should be clickable to control selection, 
//          show context menus, and similar.
//
//           - The Render Window should offer an API, available to the rest of the 
//             program, to allow the user to "pick" a reference, point in space, etc.. 
//             This should involve forcibly focusing the Render Window, and then when 
//             the user clicks on a valid target, returning focus to the API's caller 
//             along with results. We'll need this for "Pick Reference from Render 
//             Window" buttons akin to those in the CK.
//
//     - Camera controls
//
//        - Keyboard-and-mouse controls should mimic the Creation Kit, while gamepad 
//          controls should mimic Halo's Forge (classic controls, not Halo 5).
//
//  - World editing
//
//     - Requires backend R&D for correctness.
//
//        - Currently, form stubs retain information about the GRUP they loaded from, 
//          and we use this information both to identify a worldspace's persistent cell 
//          at run-time, and to decide which of a cell's two child GRUPs (temporary or 
//          persistent) to write any given REFR into during the save process. Both of 
//          these uses are incorrect. Most of the other bullet points here are going 
//          to be related to replacing this approach with an actually correct approach. 
//          We can implement world viewing before fixing this, though we'll mishandle 
//          any files with unusual/incorrect data re: persistence flags.
//
//           - If we get rid of the group type, then that frees up one byte. We have 
//             three bytes to spare already, so that leaves us with enough room to let 
//             a WRLD form stub explicitly specify its persistent CELL's form ID.
//
//              - We can kill two birds with one stone, here. Replace the group type 
//                with a pointer to a "form_stub_addenda" struct. This struct can be 
//                used to link a worldspace with its persistent cell, and a topic with 
//                its sequential list of topic infos.
//
//           - If we make changes in this regard, then we need to update the form stub 
//             documentation.
//
//        - A worldspace's persistent cell is the first loaded child cell that has the 
//          "persistent" flag.
//
//        - Under what circumstances does the Creation Kit write a persistent REFR into 
//          a worldspace's persistent cell rather than the REFR's "proper" parent cell?
//
//           - We'll need code to create persistent cells for worldspaces that don't 
//             have them, no?
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
//     - REFRs that have the "deleted" flag set at save time need to be subject to the 
//       undelete-and-disable procedure, as deleted REFRs can crash the game on exit and, 
//       if subsequently overridden, can reportedly crash when loaded.
//
//        - Presumably, deleted NAVMs need to be handled the same way, with the added work 
//          of making them small or zero-size. However, no one seems to know how to then 
//          update the associated NAVIs. xEdit's online documentation states that deleted 
//          navmeshes cannot be automatically corrected and must be fixed manually.
//
//     - Navmesh editing
//
//        - We can implement the ability to edit individual navmeshes within cells, but 
//          any such edits will be incorrect by virtue of our present inability to edit 
//          the NavMeshInfoMap. We need to reverse-engineer that class and its data, and 
//          figure out how to properly maintain it when edits to individual navmeshes are 
//          made.
//
//     - Changing a reference's base form
//
//        - What do we do if this would lead to a change in the reference's own form type, 
//          as in the case of converting between REFR and ACHR, or REFR and a PHZD? Should 
//          we even allow those kinds of changes? We can't simply turn one loaded_form 
//          instance into an instance of a different class.
//
//           - We probably shouldn't allow those sorts of changes, to be honest. Base form 
//             swaps should not be allowed to change a reference's form type. We'll just 
//             need to be sure to indicate that in the UI.
//
//  - If the user has any unsaved changes, the main window should show a confirmation 
//    prompt on exit. We already override MainWindow::closeEvent; we'll want to do what 
//    we need to do in there.
//
//     - Checking for unsaved changes is easy: just see if any active file forms are 
//       flagged as edited, being sure to ignore non-canonical stubs for singleton forms.
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
//  - An option to do comprehensive error checking on-demand. We'd do this by just forcing 
//    all forms to load briefly.
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
//        - What do we do if a form is *already* loaded?
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
// FINALIZING TASKS:
//
//  - Handle different locales. Complicated by the fact that ES[LPM] files use various 
//    system locales instead of UTF-8 or specifying a locale explicitly.
//
//  - Saving an ESL should warn if the active file would have any CELL records, whether they 
//    be new forms or overrides. Reportedly, CELLs in ESLs have issues, though I don't know 
//    the source or the specific problems offhand.
//
//     = PER AERS, THERE IS A BUG WITH ESLs THAT DEFINE NEW CELLS. THE RESULT IS THAT IF THE 
//       CELLS ARE OVERRIDDEN BY ANOTHER MOD, THE GAME WILL BECOME UNABLE TO LOAD TEMPORARY 
//       REFS INSIDE OF THE CELL, APPARENTLY BECAUSE PART OF THE CELL-LOADING CODE MISHANDLES 
//       LIGHT FORM IDs AND DOESN'T APPLY THE LIGHT INDEX (I.E. IT ALWAYS CHECKS FE000yyy 
//       INSTEAD OF FExxxyyy, WHiCH ALSO MEANS THAT THIS BUG WON'T AFFECT THE FIRST ESL FILE 
//       IN THE LOAD ORDER).
//
//        - We should probably warn on loading cells defined inside of ESLs, too, and double-
//          warn if we load an override for any of them.
//
//     - GamerPoets here <https://youtu.be/g_urrHrGQOY?t=299> recommends against ESL-flagging 
//       files that have interior cells, but gives no explanation as to why. Per aers, CELLs 
//       in ESLs are always loaded as if they're in 0xFE000xxx, and per Parapets there is some 
//       issue that occurs if an ESL edits a CELL that originates from another ESL. We'll 
//       probably want to only warn when saving an ESL that overrides another ESL's cells.
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