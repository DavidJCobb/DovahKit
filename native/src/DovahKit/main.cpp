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
//  - If any loaded files are for SSE, then the file_load_order should cap the load 
//    order at 253 entries, not 254.
//
//     - To identify a file's version, check the version value on the TES4 record. 
//       (But of course, file_header and file_reader have already been amended to 
//       retain that information.)
//
//  - Add a menu bar item to edit the author and description of the current active 
//    file. If no active file is loaded, then the menu item should be greyed out.
//
//  - tes_file_reading::subrecord::to_string is unintuitive; that should just be a 
//    template specialization of subrecord::read.
//
//  - Generate proper Use Info for hardcoded forms.
//
//     - During use info generation, when we're setting up outbound use info, we 
//       should have a special handler for any hardcoded form_stub whose file is 
//       the file_load_order::hardcoded_forms_file; these are hardcoded forms that 
//       have not been overridden in any loaded files. Our special handler should 
//       construct outbound references for these hardcoded forms, by hand. We 
//       mainly only need this for 14:PlayerRef using 07:Player as its base form.
//
//  - Use Info dialog
//
//     - Only allow the user to open one per form, as with form-editing dialogs.
//
//     - The form ID in the title bar should be uppercase.
//
//     - The tables need to be sortable.
//
//     - For listed refs that are in exterior cells, consider showing the grid 
//       coordinates whenever the cell is unnamed. We can do that now that the 
//       group_stub has those.
//
//     - Consider collating references that are in the same cell together and then 
//       showing a count, like the CK does.
//
//        - Double-clicking a listing takes you to the ref, in the Render Window, 
//          but... if a listing represents multiple refs, what then?
//
//     - Victor would like to be able to filter the Use Info window to just certain 
//       form types. We should think about potential UI designs that would allow 
//       for this, e.g. a drop-down of form type signatures and a textbox for 
//       filtering form and editor IDs.
//
//        - Actually, that UI sounds about perfect, and it should be pretty compact 
//          too. Oughta fit on one line.
//
// THINGS TO LOOK INTO:
//
//  - Build a unit testing framework wherein we run automated correctness checks 
//    on loaded data, use info, etc., for pre-chosen forms and compare the results 
//    to data prepared in advance. We should run these tests periodically if not 
//    regularly, in order to catch unexpected regressions.
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
//     - Define Form::save overrides for all loaded-form classes, and then consider 
//       making the member function on Form virtual.
//
//        - Alternatively, use the non-virtual interface idiom: have (Form::save) 
//          be a non-virtual function which writes whatever subrecords are needed, 
//          and have some virtual (Form::_save_impl) that subclasses are meant to 
//          override. That way, overrides don't need to call super, and the function 
//          that we want overridden can be made pure while still having superclass-
//          provided behavior.
//
//     - NOTE: COPYING RECORD DATA FROM THE SOURCE FILE IS ONLY APPROPRIATE WHEN 
//       RESAVING THE SAME FILE WITH THE SAME MASTERS. IF ANY MASTERS IN THE ACTIVE 
//       FILE DIFFER FROM THOSE OF THE ORIGINAL, THEN THE COPIED DATA WILL END UP 
//       HAVING BAD FORM IDs. FIXUP WOULD HAVE TO BE CODED PER-FORM AND AT THAT 
//       POINT, WE MAY AS WELL WRITE FULL SAVE CODE.
//
//        - This unfortunately also means that we actually can't ship DovahKit with 
//          a minimum of form types and patch it incrementally, as hoped. Damn.
//
//     - The UI should offer a file-save dialog that lets the user choose what game 
//       they want to save for (Classic or Special) and whether they want to use 
//       any special file header flags (e.g. ESM-flagged ESPs).
//
//        - There should also be a filename field, which should be greyed out unless 
//          the active file is implicit/invisible (i.e. nameless)
//
//        - The game and flags should default to those of the source file, if any. 
//          If the active file is implicit/invisible, then choose the game based on 
//          the current load order.
//
//        - Show the load order as a panel on the righthand side.
//
//           - Perhaps at some point in the future, we can let the user exclude files 
//             that the active file neither overrides nor references; however, we'd 
//             have to check the latter very carefully, and we'd need to introduce a 
//             form ID fixup step when saving.
//
//     - DovahKitCore needs to provide two signals, onSaveImminent and onSaveComplete, 
//       so that the UI can abandon any form pointers prior to a save and reacquire 
//       them afterward.
//
//        - Form-editing dialogs need to store form_stub pointers in addition to 
//          loaded_form_ptrs.
//
//     - Saving should fail immediately with an error message if there is no active 
//       file. The file_load_order class creates an invisible/implicit active file 
//       (i.e. one with no name or forms) at the end of the load order if there's 
//       room for it; it does not do this if there is no room (i.e. 254 other files 
//       in the load order, such that an active file would be 0xFF).
//
//        - Remember to cap at 253 instead if any SSE files are loaded.
//
//     - Each dovah::form_stub to be saved needs its file pointer and file offset 
//       updated. Currently, file_writer retains fixup data including the offset 
//       within the new file, but we never actually perform the update (in part 
//       because we're still just saving to temporary files).
//
//        - Similarly, the active file needs to have its cobb::mapepd_file replaced 
//          once the output file has been written, in tandem with replacing the 
//          form_stubs' file data.
//
//     - Each dovah::form_stub to be saved needs its "edited" flag cleared. Make sure 
//       to use the setter rather than directly manipulating bits.
//
//     - form_stub::load should not attempt to load any form data from the active file 
//       while a save operation is in progress. To that end, we should add a function 
//       {bool file_load_order::is_form_loading_blocked(form_stub&) const noexcept} 
//       which takes a form_stub, checks if a save operation is in progress, checks if 
//       the received stub is flagged as edited or hails from the active file, and if 
//       so, returns false; otherwise, true.
//
//        - We should probably run those checks in the reverse order because since 
//          this is explicitly meant to deal with multi-threading, I suspect we'll 
//          want to use an atomic bool for the "we're currently saving" check.
//
//        - When we're saving files, we can update form_stubs as we write their form 
//          data to the targeted file; HOWEVER, we cannot clear the "edited" flag 
//          until after we've changed the file pointer to the active file.
//
//     - When saving WRLD, the OFST subrecord needs special handling.
//
//     - Records that exceed a certain length should be zlib-compressed. What length 
//       threshold should we use?
//
//        - I've added a (compression_policy) enum to (file_writer), but it currently 
//          isn't used by anything. For (compression_policy::threshold), I'm thinking 
//          maybe 0x200 bytes would be a good threshold, but we could always set up 
//          some debug logging to determine the mean/median/mode/standard deviation/etc. 
//          of uncompressed record sizes to try and figure out a good "outlier" size.
//
//  - Use Info window
//
//     - Should show tooltips on table items so you can view longer editor IDs.
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
//       a) Rename the current file_header class to file_overview_reader.
//
//       b) Define a file_header struct, move all header-related members from 
//          file_reader to this new file_header, and then have add a file_header 
//          field to file_reader.
//
//       c) Define a file_stats struct, which would also appears a field on the 
//          file_reader class. This struct can contain, among other things, stats 
//          on refhandle usage.
//
//       d) Give DovahKitCore accessors that return file_header& and file_stats& 
//          references.
//
//     - If the load order exceeds the game's refhandle limit, we should display 
//       appropriate warnings to the user; we should also point out that exceeding 
//       the refhandle limit breaks the Creation Kit as well. (Good thing we're not 
//       using them ourselves!)
//
//  - Code for creating new forms.
//
//     - When we have an invisible/implicit active file, we don't set its nextFormID. 
//       We may want to amend that decision.
//
//        - How is nextFormID stored in the file? Is it relative to the master list? 
//          It's stored as a uint32_t, but Bethesda could've gotten away with a 3-byte 
//          number since they never intended for us to define forms outside of the 
//          active file, so...
//
//        - Everything that listens for formModified will probably also need to listen 
//          for these.
//
//     - We'll probably want signals for when forms are created, so that UI controls 
//       that draw lists of forms don't have to rebuild their entire lists/models.
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
//  - Render Window
//
//     - When looking at a base form's Use Info, double-clicking a reference in the 
//       listing should open its parent cell in the Render Window and select it, instead 
//       of opening the reference's form-edit dialog.
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