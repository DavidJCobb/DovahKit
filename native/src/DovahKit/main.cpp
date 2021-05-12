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
//        - Could editing the record flags on a singleton form stub confuse the 
//          file_load_order with respect to which stub is canonical? Editing the 
//          record flags requires creating a new source file entry, no? And we use 
//          the stub with the most such entries as the canonical stub.
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
//              = Wrappers are now split into sub-lists based on their "pertinent pointer," 
//                which will be a form_stub* for form data and a QWidget* for UI items. We 
//                use light userdata, so the pointer itself is a key into the wrapper 
//                storage. This means that if any pointers die (e.g. form deletion), we 
//                need to zombify all wrappers in their lists.
//
//              = Lua utility classes
//
//                 = Note: it is theoretically possible to create a vector3 instance and 
//                   then set its x/y/z values to tables with operator-overload metamethods. 
//                   Native code will not *consistently* invoke these metamethods. Sometimes 
//                   they'll work; sometimes they won't. Consider this UB.
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
//              - Any further wrappers will depend on having UI implemented for 
//                the respective form types or wrapped data, so that we can actually 
//                verify that our scripted changes are being made and persisted 
//                properly -- and that the script is even reading data properly in 
//                the first place.
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
//                                - RefPickerButton: when a REFR is used as a parameter 
//                                  to a condition, does the CK make the REFR persistent? 
//                                  Does this depend on where the condition is?
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
//                   When changing a property's type from an array to a scalar, 
//                   zombify any existing wrappers for that array. A change back 
//                   from scalar to array should lead to new wrappers being created, 
//                   such that:
//
//                      local p = script.properties["foo"]
//                      p.type  = "Form[]"
//                      local a = p.value
//                      p.type  = "Form"
//                      p.type  = "Form[]"
//                      object_is_zombie(a) == true
//                   
//                   When changing a property's name, we should emit a warning if 
//                   there exists on the script another property which already has 
//                   that case-insensitive name.
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
//           - Scripts will need an API to queue the form-edit dialog for a form. See, we 
//             need to block the editor while a script is running, so if for example a 
//             script searches for forms matching some criteria, then the user won't simply 
//             be able to double-click forms in that listing to jump to viewing and editing 
//             those forms. The best we can do, for now, is allowing scripts to queue a 
//             dialog to open for a given form (with a hardcoded confirmation prompt for 
//             the user, like "Open this form for editing after the script finishes?" so 
//             they know what's going on).
//
//              - If we want to go the extra mile, then we could allow the editor and its 
//                scripts to work asynch from each other IF all main-thread form lookups 
//                hit a lock while a script is running, with scripts acquiring that lock 
//                at the start of execution, temporarily releasing it during the Lua debug 
//                hook, and releasing it for good at the end of execution. That would force 
//                a lot of changes to the frontend core.
//                
//                This is NOT necessary and probably not a good use of development time, 
//                so once script implementation is done we should move this to the bottom 
//                of the to-do list where we put the other speculative tasks.
//
//                 - That is:
//
//                    - Script thread acquires form lock when script execution starts.
//                    - Script thread releases form lock temporarily at start of debug 
//                      hook.
//                    - Script thread reacquires form lock at end of debug hook. If main 
//                      thread already has lock, script thread waits for main thread to 
//                      release.
//                    - Script thread releases form lock for good when script execution 
//                      ends.
//                    - Main thread acquires form lock when looking up forms, etc., 
//                      waiting on the script thread if that already has the lock.
//          
//           - UI widget ideas:
//
//              - A "canvas" widget that allows scripts to draw arbitrary raster data. 
//                A good use case for this would be a script that generates a render of 
//                a worldspace's heightmap. Bonus points if the widget can optionally be 
//                set to let the user save its contents as an image.
//
//                 - There are three "levels" of drawing API we can offer:
//
//                    - Simple pixel- and shape-drawing instructions
//
//                    - Layers, like in GIMP
//
//                    - Shapes and objects, like in PowerPoint (these can be built on top 
//                      of a layer implementation)
//
//                   A friend has strongly encouraged implementing layers at a minimum, 
//                   and frankly, I can see more than a few benefits -- for example, 
//                   being able to generate not only a heightmap for a worldspace, but 
//                   also being able to color in cells that are altered (or that have 
//                   objects added) by specific mods, where each mod gets its own layer.
//
//     = When writing script documentation for functions that return class instances, it 
//       must be specified whether they return tables or userdata (e.g. "vector3 table" 
//       versus "vector3 userdata"), as userdata do not support expandos in our particular 
//       implementation.
//
//     - Script execution window: save/load buttons; button to toggle the log; add a 
//       button to clear the log. Basically the only parts of the UI that are functional 
//       right now are the buttons to start and stop script execution, so we need to 
//       finish the rest.
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
//        - [[clipboard]]: The script will be able to access the user's keyboard, to 
//          copy and paste data.
//
//           - QLineEdit::copy
//           - QLineEdit::cut
//           - QLineEdit::paste
//       
//       Options that grant access to advanced or dangerous functionality should result 
//       in the user being shown a confirmation message in the script selection window, 
//       i.e. the user is shown a list of permissions that the script is requesting and 
//       must explicitly grant those permissions before the script can run.
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
//  - The (write_length_prefixed_string) function for saving has no way to handle 
//    the error of a string being too long. Should the form save functions handle 
//    that, so as to be able to report the error more intelligently?
//
//     - The backend should generate a save error identifying the current form, 
//       subrecord, and if possible, offset into the subrecord. Currently, we'd 
//       just produce corrupt data.
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
//  - Loading Papyrus data
//
//     - It seems like Papyrus data is coalesced? Maybe? Some elements, such as 
//       properties, can be flagged as "removed" using a "status" field.
//
//  - RefPickerWindow
//
//     - Current code is likely to break if the existing reference is not inside 
//       of an interior cell. We need to handle that case for when we work on the 
//       render window, so we may as well get to it sooner than later.
//
//     - Custom sorting for the comboboxes: always put NONE at the top; sort 
//       references with no editor ID at the bottom
//
//     - Re-sort the comboboxes when we detect the creation of a form and add the 
//       form to the comboboxes
//
//  - Container::_clone_impl
//
//  - Location::_clone_impl and other missing functions
//
//  - UI for editing Activators
//
//     - UI for editing Papyrus data
//
//        - Allow users to open the contents of a script file in an external editor. 
//          If the script file is stored in a BSA, then unpack it to %TEMP% and, if 
//          changes are made to it, prompt the user as to whether they want to copy 
//          it to the Data directory. See if it's possible to monitor how long the 
//          temporary file is open in any external editor; we should track changes 
//          for as long as it's open, and delete it once it's closed.
//
//  - UI for editing Factions
//
//  - UI for editing FormLists
//
//     - Make it possible to reorder items in the FormList window by dragging them. 
//       Ensure that this doesn't conflict with Object-Window-to-FormList drags (i.e. 
//       ensure it doesn't duplicate items, allow dragging to other windows, etc.).
//
//        - That said, it should be possible to drag a form from one FormList window 
//          to another to copy it to the latter FormList.
//
//  - Finish code for: DIAL, LCTN, MGEF, NPC_, QUST
//
//  - Test re-saving INFOs
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
//              - Edit RefPickerWindow to use this API.
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