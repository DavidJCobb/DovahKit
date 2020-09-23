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
//  - Use Info dialog
//
//     - Consider collating references that are in the same cell together and then 
//       showing a count, like the CK does.
//
//        - Double-clicking a listing takes you to the ref, in the Render Window, 
//          but... if a listing represents multiple refs, what then?
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
//     - When saving WRLD, the OFST subrecord needs special handling.
//
//     - Record compression
//
//        - Test the "threshold" implementation.
//
//        - Write the code to decide when CELL records should be compressed.
//
//           - Not possible until we implement loading and saving cells in the first 
//             place.
//
// DISTANT TASKS:
//
//  - The user needs to be able to pick which game (Skyrim Classic or Skyrim Special) 
//    they want to open files from. Currently, we just always use the Skyrim Classic 
//    install path.
//
//  - Refhandle usage tracking: the number of persistent references in ESMs, and the 
//    number of all references in non-ESMs, should be tracked and stored on each 
//    file_reader. This information should be accessible to UI code.
//
//     - We may want to do a few things, so that we can isolate file_reader objects 
//       away from frontend code while still being able to display file-specific 
//       stats (and make those stats available for editing):
//
//       a) Define a file_stats struct, which would also appears a field on the 
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
//  - Code for creating new forms.
//
//     - We need to update the active file's nextFormID.
//
//        - If the active file has no masters, then use load order prefix 00; if the 
//          active file has any masters, use load order prefix FF.
//
//     - We'll probably want signals for when forms are created, so that UI controls 
//       that draw lists of forms don't have to rebuild their entire lists/models.
//
//        - Everything that listens for formModified will probably also need to listen 
//          for these.
//
//  - Code for deleting forms.
//
//     - How should we handle the case of a user deleting a form from one of the active 
//       file's masters (whether or not that form is currently overridden in the active 
//       file)?
//
//        = Bear in mind that when checking a form's origin, we can't rely on the file 
//          pointer, because overrides would use the active file as their file pointer. 
//          We'd have to check the form ID load order prefix to distinguish an active 
//          file's original forms from its overrides.
//
//        - "Deleting" an override doesn't un-override the form; it just overrides the 
//          form and sets the "deleted" flag.
//
//        - "Deleting" a REFR/ACHR/etc. from a master doesn't actually delete it; the 
//          reference just ends up loading at (0, 0, 0) in its parent world. Since 
//          this is the literal last thing that people would expect, we shouldn't even 
//          allow the "deletion" of references; we should offer a shortcut for disabling 
//          them and moving them underground, as xEdit does (and by "disable" I mean 
//          "set the player ref as an opposite enable state parent").
//
//        - It looks, then, like "deleting" a master record just creates a "delete me" 
//          override. Is there any case where that's even useful? Should we even allow 
//          it?
//
//           - Test whether deleting a master record allows the form to appear in-game. 
//             That is, run the test with forms that can be queried via the `help` 
//             console command: see if they still show up when they're deleted via an 
//             override.
//
//           - If we choose to disallow it by default, then we should have an options 
//             window with a "Let me do doofy things" option that allows the user to 
//             do it.
//
//     - DovahKitCore needs to offer signals for deleting forms: onFormDeleteImminent 
//       and onFormDeleteComplete, the former of which should take a form_stub argument. 
//       It also needs to offer an accessor that the UI must use to delete forms; the 
//       accessor should emit those signals. Form-editing dialogs need to listen for 
//       those signals and abandon their form_stub pointers and their loaded_form_ptrs 
//       in response to the former.
//
//        - Presumably we should only fire these signals when deleting forms out of 
//          the active file.
//
//        - Everything that listens for dataAbandonImminent will probably also need to 
//          listen for these.
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
//  - If the user has any unsaved changes, the main window should show a confirmation 
//    prompt on exit. We already override MainWindow::closeEvent; we'll want to do what 
//    we need to do in there.
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