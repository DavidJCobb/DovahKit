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
//  - The frontend should not allow users to set the game's built-in files or 
//    official DLCs (i.e. the high-res texture packs) as the active file.
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
//  - Support for Skyrim Special Edition.
//
//     = Within an ESL file, forms defined by the ESL file do not use the 0xFE prefix. 
//       If an ESL has four masters (such that the last of them is prefixed 0x03), then 
//       the ESL's own forms will use prefix 0x04. (xEdit will display the prefix as 
//       0xFE; you must hex-edit the file to see the true prefix.)
//
//        = When the Creation Kit loads ESL files, it does not place them in slot 0xFE, 
//          but rather gives them the same load order prefix as a non-ESL. This occurs 
//          whether loading a single ESL or multiple.
//
//     = A file can have ESLs as masters, and will number them the same as non-ESL 
//       masters. If a file has one non-light master and one light master, then forms 
//       from the light master will use load order prefix 0x01, and can be referenced 
//       and overridden. An additional light master would use prefix 0x02.
//
//     = So there's two approaches we can take, then. The first is to allow the frontend 
//       to decide whether (file_load_order) honors light plugins or just treats them as 
//       non-light plugins; our particular frontend, being intended for editing, would 
//       opt out of supporting light-plugins, just as the Creation Kit does.
//
//       The second approach is to make the backend support light plugins for all tasks, 
//       and have it mass renumber the active file's form IDs when the active file gains 
//       or loses "light" status during a save. If we do this, then we need a lot of 
//       error checking and a lot of form ID fixup on save:
//
//        - A save operation needs to fail if the active file would end up with too many 
//          masters (non-light and light total).
//
//        - Light plug-in IDs need to be converted from 0xFE to master-list-relative; 
//          the file format's own handling of form IDs doesn't support light plug-ins.
//
//        - When the active file's "light" flag changes, we need to mass renumber all of 
//          its form IDs in memory, to change their load order prefix.
//
//       Mass renumbering is a particular pain point because that's even more boilerplate 
//       that needs to be added to every loaded form class. However, we could potentially 
//       combine it with the existing boilerplate for handling form deletion, if we allow 
//       loaded forms to return a vector of pointers to their form_id_t members. (Note, 
//       however, that there are form_id_t subclasses that override setter behavior, and 
//       they do so entirely on type -- they're not virtual and they have no internal 
//       members to disambiguate their types -- so we'd need multiple getters for each 
//       form_id_t type, or we'd need to add type information to form_id_t.)
//
//       It's also worth noting that we need mass form renumbering anyway if we want to 
//       offer the option to compact form IDs in preparation for saving an ESL, and that 
//       being able to renumber single forms would give us some parity with xEdit.
//
//        - If we want to implement editing, then making form_id_t retain type information 
//          (i.e. information indicating what use info flags should be applied as the ID 
//          is changed) is the best approach. Then, we can modify the loaded form hooks -- 
//          replace (Form::sever_outbound_references_to) with (Form::get_all_form_IDs) 
//          which would return (std::vector<form_id_t*>). The code for deleting forms 
//          could blindly loop over those IDs and call (form_id_t::set), while the code 
//          for renumbering form IDs could directly overwrite them (in addition to 
//          updating use info entries' keys, moving the form_stub within the file_load_order 
//          form maps, and so on).
//
//        - Renumbering forms should send DovahKitCore signals:
//
//           - void onFormRenumberImminent(form_stub*, uint32_t oldID, uint32_t newID);
//           - void onFormRenumberComplete(form_stub*, uint32_t oldID, uint32_t newID);
//
//          UI code needs to react responsibly to these.
//
//     = As such: when the backend is not being used for editing, it should be able to 
//       load an arbitrary number of ESLs (subject to the limit on those) and properly 
//       remap their form IDs during load. The question is whether we should support ESLs 
//       during editing, given that the file format itself does not.
//
//     = Proper ESL form ID mapping would require storing ESL files in a separate list 
//       from non-ESL files. This in turn requires that we audit every piece of code that 
//       accesses the (file_load_order::files) vector.
//
//     - Option to save an Special active file as a Classic file and vice versa.
//
//     - Option to save an active ESL as a non-ESL, vice versa.
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
//  - Code to save the current active file.
//
//     - File-save dialog
//
//        - Allow the user to select which game (Classic/Special) to save for.
//
//           - If they save for a different game than they loaded for, then where 
//             should we put the new file?
//
//           - If they save for a different game than they loaded for, then editing 
//             can't continue unless we support mixed Classic/Special load orders.
//
//              - We'd only have to treat the active file as an exception, rather 
//                than "fully" supporting mixed-game load orders.
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
//  - Localized string support
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
//     - The Lua VM should be started up when the script begins, and should be killed 
//       after the script has run to completion and all script-spawned windows (if any) 
//       have closed.
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
//     - Since we don't reveal every single form through a unified interface as xEdit 
//       does, it will need to be possible for scripts to define forms (as in sheets 
//       of values, not as in game data) that the user can fill out to provide values 
//       to the script (e.g. to tell it what forms, as in game data, to operate on). 
//       We *could* create Lua wrappers for creating and managing Qt UI, but that 
//       feels like it'd be very involved; a data format for describing "script 
//       arguments" could work better but would require us to account for every 
//       possible case. Bad trade-off either way, it looks like.
//
//  - Support for loading the contents of localized strings.
//
//     - BLOCKED by BSA loading: Skyrim stores its localized string files inside of 
//       Interface.bsa. We have a rough-draft class for localized_string_file and 
//       commented-out integration in file_reader, but that rough-draft class needs 
//       to be revised to store the localized string data persistently (since we 
//       can't just rely on a mapped_file anymore).
//
//     - We should not allow you to set something as the active file if it uses 
//       a localized string file, since we don't have the means to edit those yet. 
//       However, we should still be able to load localized strings just so that you 
//       can create overrides of forms that contain localized strings, and so you can 
//       see those strings in the editor.
//
//  - BSA loading
//
//     - TESFilePicker should allow the user to browse through loose files and files in 
//       the loaded BSAs.
//
//  - If the user has any unsaved changes, the main window should show a confirmation 
//    prompt on exit. We already override MainWindow::closeEvent; we'll want to do what 
//    we need to do in there.
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
//  - Ability to convert files bidirectionally between Skyrim Special and Skyrim Classic.
//
//     - Requires being able to warn the user about data loss in cases where fields 
//       don't exist in the target version.
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