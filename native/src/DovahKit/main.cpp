#include <QtWidgets/QApplication>

#if _DEBUG
   #include <QDebug>
   #include <QDirIterator>
   #undef NDEBUG
#endif

#include "ui/main_window.h"
#include "editor/core.h"

//
// CURRENT TASKS:
//
//  - Use std::filesystem::path instead of std::string for file paths and names in 
//    dovah::file_load_order, dovah::tes_file_reading::file_reader, and so on.
//
//  - When saving a new file, if the user enters the name of an existing file, then we 
//    should prompt for permission to overwrite. If the name that the user enters is 
//    the same name as one of the already-loaded files, we should fail.
//
//     - If the user converts a file across games, and the existing file's name also 
//       exists for the target game, then we should prompt to overwrite as well.
//
//  - Form duplication can fail in complex ways: even if we successfully duplicate the 
//    target form, we may fail to duplicate child or descendant forms. We should create 
//    a custom dialog box that can show multiple sets of error details for this case.
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
//  - When loading the contents of cell GRUPs, we should log a warning if a record is 
//    misplaced (e.g. WRLD/VTYP, WRLD/CELL/VTYP, etc.). We already log warnings for 
//    misplaced records in top groups.
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
//        - Could editing the record flags on a singleton form stub confuse the 
//          file_load_order with respect to which stub is canonical? Editing the 
//          record flags requires creating a new source file entry, no? And we use 
//          the stub with the most such entries as the canonical stub.
//
//     - NAVI
//
//        - If files define their own NAVI with a different form ID, then what form 
//          ID does the final loaded NAVI use? This doesn't matter for DovahKit, but 
//          I'm curious.
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
//  - Form deletion: some form types skip saving some subrecords if the form is flagged as 
//    deleted. Should we replicate this behavior? We'll want to continue testing in the 
//    Creation Kit to find all of these, if we care that much.
//
//     - REFR/DATA
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
//           = Collection implementation
//
//              - We can remove the separate "items are named" bool and just use 
//                the presence or absence of a "lookup by name" function instead.
//
//           = ALL DATA THAT DovahKit IS CAPABLE OF LOADING IN FULL SHOULD BE MADE 
//             ACCESSIBLE TO SCRIPTS BEFORE WE MOVE ON TO IMPLEMENTING UI ACCESS. 
//             THIS WILL ALLOW US TO IDENTIFY AND ADDRESS PAIN POINTS IN THE SCRIPT 
//             API BACKEND EARLIER IN DEVELOPMENT.
//
//              = Currently, we prevent scripts from receiving none-stubs by way of 
//                the (wrapper::should_expose_to_script) function. However, we may 
//                want to add some sort of script-wide pref that can be used to 
//                gain access to none-stubs, chiefly because a script may actually 
//                want to do something with them specifically.
//
//              - FLST wrapper
//
//                 - Define formlist:remove(x) where x can be a list index to delete, 
//                   or a form (where we then find the first occurrence and remove 
//                   it from the list, leaving no gaps). Assigning (nil) to list 
//                   entries should create gaps in the list. (This is a change in 
//                   the originally intended design, so comments in the collection 
//                   setter need to be updated as well.)
//
//              - Papyrus wrappers
//
//                 = PROPERTY WORK BLOCKED BY QUEST SUPPORT: WE NEED TO BE ABLE TO 
//                   RETURN QUEST ALIASES TO LUA, SINCE SCRIPT PROPERTIES CAN HOLD 
//                   THEM AS VALUES.
//
//                    - QUEST FORM SEEMS COMPLETE
//
//                    - Quest UI
//
//                       = Condition list UI
//
//                          - When working with a clone, react to the clone being 
//                            modified from outside (requires assistance from the 
//                            owning FormDialogWorkingCopyBase).
//
//                          - When working with a stub, react to the form being 
//                            modified while it's being operated on, if possible.
//
//                          - Stringifying condition arguments: the branch for 
//                            using package data is incomplete; it should check 
//                            the package for a data of that index and if one is 
//                            present, show the name. We can't finish this because 
//                            packages aren't implemented yet.
//
//                          - WE DON'T PROPERLY RETRIEVE THE CONDITION'S OWNING 
//                            QUEST FOR THE FOLLOWING CONTAINING-FORM TYPES:
//
//                             - INFO (get parent DIAL and check its QNAM)
//                             - PACK (check QNAM)
//                             - SCEN (check PNAM)
//
//                          - UI for editing a condition
//
//                             - Parameters
//
//                                - Shouldn't GetStageDone conditions only care about 
//                                  the selected quest's working copy if that quest is 
//                                  the condition's containing form? Otherwise, editing 
//                                  stages in one dialog can unexpectedly change conditions 
//                                  in other dialogs.
//
//                             - Run On: We should hide the "Package Data" option if there 
//                               is no owning package. Alternatively, can we grey out 
//                               individual options in a drop-down?
//
//                             - We have the GetWithinPackageLocation condition listed 
//                               as accepting package data of any type from its owning 
//                               package, but it actually specifically wants package 
//                               data of package-data-type "Location." However, in the 
//                               CK, checking "Use Pack Data" on the condition will get 
//                               the function to accept only package data of other 
//                               types; this feels like a bug.
//
//                       - Basic Data
//
//                          - A friend has requested being able to click a button 
//                            next to the event dropdown to jump directly to the 
//                            SM Event form for that event type.
//
//                       - Objectives
//
//                          - Objective-specific and target-specific content
//
//                          - Conditions UI
//
//                       - Aliases
//
//                          - Window for editing aliases
//
//                       - Dialogue
//
//                          - Support for Topics and TopicInfos
//
//                             - UI for these form types
//
//                                - The function for opening a form edit window 
//                                  needs to treat these form types as a special 
//                                  case and open the full "hierarchy" of forms, 
//                                  e.g. QUST/DIAL/INFO, as modeless/modal/modal.
//
//                          - Player tab
//
//                          - Favor tab
//
//                          - Combat tab
//
//                          - Detection tab
//
//                          - Service tab
//
//                          - Misc tab
//
//                       - Scenes
//
//                       - Scripts
//
//                    - Lua wrappers for Quest data (bare minimum: the form itself 
//                      and any aliases contained therein)
//
//                 - Fragment data for supported forms, and reimplementation of the 
//                   script-data access so that these work for scripts on aliases.
//
//                    - Requires implementing all form types that have these sorts 
//                      of script data, so we'll have to do it later.
//
//              - Provide special top-level accessors for default objects and NAVI 
//                data (when that's decoded). We should still allow scripts to look 
//                up the forms, but not have classes for them. (Why? The alternative 
//                is to not allow lookups to return singleton forms, but that leads 
//                to odd situations where a form ID is taken but you can't tell by 
//                what.)
//
//                 - This is the general approach that should be taken for any 
//                   singleton forms.
//
//              - We need to provide special accessors for GMST, since those don't 
//                map 1:1 to form IDs.
//
//              - For extra-data types, the wrappers should always act as though 
//                there is underlying data, and create and destroy it as appropriate. 
//                For example, if a weapon placed in the game world doesn't have any 
//                poison preapplied (no REFR/XPSN), (weapon.extra.poison) should not 
//                test as nil, and assigning to (weapon.extra.poison.type) or to 
//                (weapon.extra.poison.count) should immediately create an underlying 
//                XPSN. Setting both properties to nil, or the type to nil and the 
//                dose count to zero or negative, should immediately destroy the 
//                underlying object.
//
//                This approach is preferable to having to define "create" and "remove" 
//                member functions on the extra-data-root wrapper for every extra-data 
//                type. We can't allow (weapon.extra.poison = new_something()) because 
//                that would require allowing wrapped objects to exist without any 
//                underlying form, which isn't possible here (i.e. you can't call a 
//                "new" function for XPSN, pre-configure the poison, and *then* put it 
//                on a reference, unless we decide to store its properties in Lua and 
//                then synchronize them with any underlying wrapped object, which is... 
//                complicated.)
//
//        - PHASE 3: UI ACCESS
//          
//           - Done!
//
//     = When writing script documentation for functions that return class instances, it 
//       must be specified whether they return tables or userdata (e.g. "vector3 table" 
//       versus "vector3 userdata"), as userdata do not support expandos in our particular 
//       implementation.
//
//  - Extra data: room ref data: this is another multi-subrecord structure where the 
//    game will blindly "eat" subrecords without even checking their signature. We 
//    don't emulate that behavior here even though we emulate it (with warnings) in 
//    other places. We should emulate that behavior here.
//
//  - Papyrus fragment editor
//
//     - Consider using a QValidator or something similar to vary the field's look 
//       based on whether the script exists but isn't attached to the containing 
//       form, or whether the script doesn't exist. For example, we might show an 
//       icon next to the scriptname.
//
//        - Perhaps, for unattached scripts, clicking the icon could attach the 
//          script to the form -- after a confirmation prompt, of course.
//
//  - QUST/VMAD alias-script transplant handling
//
//     = Quests can specify script data to attach to aliases. The thing is, each 
//       piece of script data is prefixed not simply with an alias ID, but with a 
//       form ID and alias ID. This implies that one quest can attach scripts to 
//       aliases in different quests, and in-game testing confirms that that's the 
//       case.
//
//       We can't currently handle this, because we don't have any way to handle 
//       cases where the record for one form contains data for a different form.
//
//     - The way to handle this would be to create a concept of "donor forms," 
//       which "donate" data to some subject form. The subject form's stub would 
//       have a list of its lenders in the addenda. We would then need to define 
//       some function (Form::load_from_donor).
//
//       Then, we'd have to modify (form_stub::load) as follows:
//
//        - For each source file on the subject form, gather up a list of records 
//          to load; this list would be the subject's record, and any records 
//          from the same source file belonging to donor forms.
//
//        - Sort the records by file offset.
//
//        - Load them in order.
//
//       This would obviously complicate the loader quite a bit.
//
//     - Another issue is use info: how the hell do we even handle this case? If 
//       we delete the donor form, then we lose the data it was donating to the 
//       subject form, unless we forcibly load the subject form and flag it as 
//       edited at that time. I guess we could manage that if we had a use info 
//       flag for donor/recipient relationships. When we delete form A, we loop 
//       over every user B relying on the use info, force the Bs to load, and 
//       flag them as edited. In forcing the subject to load, we'd pull in all 
//       of the donated data from each donor.
//
//       That works when the donor and subject are both in the active file, but 
//       if data is being donated across files or within masters, I'm not sure 
//       how we'd handle that. Hell, I'm not sure how the *game* would handle 
//       a donor being flagged as deleted.
//
//       The interactions get a bit more complicated when saving: we'd end up 
//       move the donated data from the donor to the subject, and once that's 
//       done, we need to sever the donor/recipient relationship including 
//       within use info (again, ignoring cross-file and within-master donor 
//       stuff for now).
// 
//        - Not to mention: what if two forms donate to each other, forming a 
//          cyclical reference? What happens if a donor is overridden and the 
//          override doesn't donate?
//
//  - World editing
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
//           - Workaround for the CK not properly flagging all unique actors as persistent?
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
//  - Dialogue flowchart editor
//
//     - The CK editor doesn't let you (reliably) control how connecting lines are drawn 
//       between topics and infos. We should. Consider this case:
//
//                                                +---------------+
//                               +----------------+     Topic     |
//                               |                +---------------+
//       +-------------+         |
//       |   Topic     +---------+
//       +-------------+
//
//       If you were to right-click on the upper segment of that connector, we'd want to 
//       give you a context menu item to "add a bend." Doing that would turn the graph 
//       into this:
//
//                               +----------+     +---------------+
//                               +          +-----+     Topic     |
//                               |                +---------------+
//       +-------------+         |
//       |   Topic     +---------+
//       +-------------+
//
//       Essentially, we'd add a new bend or joint to the middle of that line segment, 
//       and displace one half of the line segment either up or down slightly.
//
// HORIZON TASKS:
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
//     - We'll need to be careful not to mishandle data from partial-flagged TopicInfos.
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
//           - There are uploads of all STRINGS files on NexusMods. Western versions of 
//             the game also ship with most/all Western languages, though not East Asian 
//             languages.
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
//        - We'd need to load the revision as a working copy, which means that our frontend 
//          can't do this if an edit dialog for the form is already open.
// 
//           - Another complication: not all edit dialogs use working copies!
//
//        - Reverting a form stub should flag it as edited.
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
   a.setWindowIcon(QIcon(":/DovahKit.ico"));
   //
   #if _DEBUG
   {
      static constexpr bool print_all_qt_resources = false;

      if constexpr (print_all_qt_resources) {
         qDebug() << "Dumping list of all Qt resources...";
         QDirIterator it(":", QDirIterator::Subdirectories);
         while (it.hasNext()) {
            qDebug() << it.next();
         }
         qDebug() << "All Qt resources dumped.";
      }
   }
   #endif
   //
   MainWindow w;
   w.show();
   //
   auto result = a.exec();
   //
   // Some subsystems have behavioral dependencies on one another, such that we don't want to 
   // count on construction/destruction order to resolve them. It seems safest to exit with a 
   // data-abandon step, same as if the user unloaded data (e.g. to switch to editing another 
   // file) while running the program.
   // 
   // [2/27/2024] Without this call, closing the program after loading data causes assertion 
   // failures within the Papyrus subsystem: the form-info-cache subsystem doesn't clear out 
   // refcounted pointers to known Papyrus scripts (it only does that on-data-abandon), and 
   // then when the Papyrus subsystem is destroyed and it deletes its known scripts, it will 
   // fail assertions as to their refcounts being zero. We could give the form-info-cache a 
   // destructor that severs those pointers, but guaranteeing that the subsystems are both 
   // destroyed in the right order is... It feels very "action at a distance"-y. It mirrors 
   // construction order, so we could have the Papyrus subsystem be what initially constructs 
   // the form-info-cache subsystem, or change the order in which DovahKitCore constructs 
   // both of them; but it feels easier to just forcibly abandon game data and rely on signals 
   // and slots so we don't have to worry about it.
   // 
   // If this results in exit being too slow, we could look into writing quick-exit handlers 
   // for subsystems as needed and then doing std::quick_exit here.
   //
   DovahKitCore::get().abandon_data();
   //
   return result;
}