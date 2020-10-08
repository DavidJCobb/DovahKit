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
//  - BSA support
//
//     - Refer to bsa_load_order.h for plans.
//
//  - Localized string support
//
//     - Each TES file that uses localized strings needs a LocalizedStringStore created 
//       for it. The LocalizedStringStore should act as the interface for loading the 
//       content of a localized string for any given language. LocalizedStringStore 
//       should be able to pull localization files from a supplied (bsa_load_order). It 
//       should be able to retain these files' contents in memory, closing them when 
//       asked to. (Essentially, you should be able to tell it when to load and unload 
//       any given language's file.)
//
//     - The relevant classes will look something like this:
//
//          struct LocalizedStringStore::entry {
//             std::array<std::string, 10> strings; // don't remember offhand how many languages; guessed 10
//             uint16_t availability = 0; // flags-mask indicating which languages are loaded
//          }
//
//       If you ask the LocalizedStringStore to supply an entry in a language whose file 
//       isn't open, then it should open that file. (Essentially, if it doesn't have the 
//       German file open and you ask it for a string in German, it should open that file 
//       and grab the string. You can then tell it to close German, and it will close the 
//       file while retaining any strings it has already loaded. You don't need to open 
//       every file in advance; it "knows," on a per-string basis, what it has loaded.)
//
//        - Actually, the strings should be an unordered_map of language names to text. 
//          Why? Because Skyrim specifies its language name as a string. No reason to 
//          believe it limits the values.
//
//           - Means the LocalizedStringStore also needs to remember what language names 
//             are available (i.e. have files) for its corresponding ES[LPM] file.
//
//     - To retrieve the content of a (localized_string) for any given language, you must 
//       retrieve the (form_stub)'s TES file and then grab the LocalizedStringStore for 
//       that file.
//
//        - When a form is overridden, its stub's file pointer isn't changed immediately 
//          but rather gets changed during saving. At this time, we must either update 
//          all STRINGS files for the active file, or we must flag the active file as 
//          not using localized strings.
//
//           - And of course, this means that if the active file is not using STRINGS 
//             files, then at save time, all (localized_string)s need to have content 
//             available in whatever language the user is saving in. We can avoid the 
//             need for yet another virtual handler on loaded_forms::Form if we just 
//             have subrecord::read(localized_string&) look up the localized text for 
//             the user's language so that it's pre-filled by the time we save.
//
//     - We should consider adding a "View" menu to the editor with an "Encoding" option. 
//       This option would control the encoding used for all files that aren't relying on 
//       STRINGS files. (Already-open form-editing dialogs don't need to update their 
//       content when this changes; just focus on everything else.) The default encoding 
//       should be set based on the user's language as indicated in Skyrim's INI.
//
//        - Perhaps it'd be better to have this only affect the active file, or to have 
//          it only affect non-DLC files, if for no other reason than to prevent all 
//          editor IDs in the base game from showing up as illegible mojibake. I think 
//          limiting it to the active file would be more intuitive.
//
//           - We also need to control the encoding used for editing, so maybe we should 
//             make this active-file-only and put it under "Edit," not "View."
//
//        - Reportedly, Skyrim Special TES files use UTF-8 for all languages except 
//          English, which is Win-1252. I need to download the SSE CK and save some 
//          files with non-ASCII Win-1252 characters, along with non-Win-1252 Unicode 
//          characters, and test that. If English doesn't use UTF-8, then the non-ASCII 
//          Win-1252 characters will have character codes inconsistent with Unicode, 
//          while the non-Win-1252 Unicode characters will be stripped or become 
//          mojibake. I would of course need to use a hex editor to check for the former.
//
//     - The UI needs to physically prevent the user from entering glyphs that are not 
//       available in the current language. Alternatively, textboxes with bad glyphs 
//       should be given a red outline, and the "OK" button should be greyed out.
//
//        - If the user clicks inside such a textbox, a speech bubble should appear 
//          listing the bad glyphs. ("You are editing a [LANGUAGE] file. The following 
//          symbols are not available in [LANGUAGE]: a L 7 Q v")
//
//     - We need to create a custom promoted widget for editing localized strings, so 
//       that if we implement STRINGS file editing in the future, we can add a "..." 
//       button that the user can click to edit localized string content.
//
//     - If we wanted to implement STRINGS file editing in the future, that would entail 
//       creating a LocalizedStringStore for the active file if one does not exist, and 
//       writing code to export localized strings into multiple files. A newly-created 
//       localized string would just have the same content in all languages, since all 
//       languages (except Czech, apparently?!) use UTF-8 unless invalid byte sequences 
//       are encountered. So for example if an English-speaker creates a new string that
//       says "Apple", we'd pre-fill the other languages with "Apple" as well.
//
//        - The benefit to this approach is that the STRINGS files could then later be 
//          edited to contain the correct content for other languages.
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
//          If we take "there isn't even any localization" as a failure mode, then it can 
//          certainly be said that when Bethesda's localization systems fail, they do not 
//          fail even a little bit gracefully.
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