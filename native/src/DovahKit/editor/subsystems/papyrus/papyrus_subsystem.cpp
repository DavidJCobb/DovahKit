#include "./papyrus_subsystem.h"
#include <array>
#include <string_view>
#include <type_traits>
#include "helpers/string/strieq_ascii.h"

#include "dovah/data/papyrus/helpers/name_equals.h"
#include "dovah/data/papyrus/native_classes.h"
#include "dovah/form_types.h"

#include "editor/core.h"
#include "./known_script.h"

// For loading PEXs:
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include "dovah/files/bsa/bsa_archive.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/bsa/bsa_load_order.h"
#include "dovah/files/pex/parsers/class_info_collector.h"
#include "editor/subsystems/assets.h"

namespace {
   constexpr const auto all_form_type_inheritance = []() {
      constexpr size_t count = []() {
         size_t c = 0;
         c += 1; // dovah::form_type::location_alias
         c += 2; // dovah::form_type::reference_alias
         for (const auto& info : dovah::form_types)
            if (info.parent_type != dovah::form_type::none)
               ++c;
         return c;
      }();

      std::array<std::pair<dovah::form_type_t, dovah::form_type_t>, count> out = {};
      size_t i = 0;

      out[i++] = { dovah::form_type::location_alias,  dovah::form_type::alias };
      out[i++] = { dovah::form_type::reference_alias, dovah::form_type::alias };
      for (const auto& info : dovah::form_types) {
         if (info.parent_type == dovah::form_type::none)
            continue;
         out[i++] = { info.form_type, info.parent_type };
      }
      return out;
   }();

   constexpr dovah::form_type_t _superclass_of(dovah::form_type_t ft) {
      for (const auto& item : all_form_type_inheritance)
         if (item.first == ft)
            return item.second;
      return dovah::form_type::none;
   }
}


namespace {
   template<typename Functor>
   void _for_loose_files_with_ext(QString path, const char* desired_ext, Functor&& functor) {
      QDirIterator it(path);
      while (it.hasNext()) {
         auto path = it.next();
         auto info = QFileInfo(path);
         auto ext  = info.completeSuffix().toLower(); // great naming here
         if (ext != desired_ext)
            continue;

         auto file = QFile(path);
         if (!file.open(QIODevice::OpenModeFlag::ExistingOnly | QIODevice::OpenModeFlag::ReadOnly)) {
            continue;
         }
         auto name = info.completeBaseName().toStdString();
         functor(name, file);
      }
   }

   // Path should use backslashes as separators and not have leading or trailing slashes.
   template<typename Functor>
   void _for_bsa_files_with_ext(const dovah::bsa_load_order* bsa_order, const std::string& path, const char* desired_ext, Functor&& functor) noexcept {
      if (!bsa_order)
         return;

      const auto folder_hash = dovah::bs_hash(path.c_str(), nullptr);

      auto& bsa_list = bsa_order->get_archive_list(); // TODO: this is intended to give us const access to the BSAs, but since it's a vector of pointers, we have non-const access too
      for (auto it = bsa_list.rbegin(); it != bsa_list.rend(); ++it) {
         //
         // We iterate over BSAs in reverse order because files packed in the BSAs at the end 
         // of the load order will override files of the same name and path packed in BSAs 
         // earlier in the load order. We can early-out on a packed script if we go in reverse 
         // order and the scriptname is one we've already seen before.
         // 
         // Note, however, that we iterate over loose files *second*, because if a script is 
         // packed in a BSA but overridden by a loose file, we want to store information from 
         // both of those files. Why? So that if the loose file is deleted, we don't have to 
         // re-scan all BSAs to know the script's "new" data.
         //
         const auto* bsa = *it;
         if (!bsa->retains_filenames())
            //
            // If the archive only identifies files by hash, then there's no way to scan for 
            // all files with a given extension.
            //
            continue;

         const auto* folder_info = bsa->lookup_folder_info(folder_hash, path);
         if (!folder_info)
            continue;

         for (const auto& file_info : folder_info->files) {
            const auto& file_name = file_info.name;
            if (file_name.empty())
               continue;
            size_t name_len = file_name.size();
            if (name_len < 5) // size of "x.pex"
               continue;
            if (file_name[name_len - 4] != '.')
               continue;
            if (!cobb::strieq_ascii(std::string_view(file_info.name.c_str() + name_len - 3, 3), desired_ext))
               continue;

            auto* archived_file = bsa->read_contents_of(file_info);
            if (!archived_file)
               continue;
            //
            std::string filename_sans_ext = file_name;
            {
               auto i = filename_sans_ext.find_last_of('.');
               if (i != std::string::npos)
                  filename_sans_ext = filename_sans_ext.substr(0, i);
            }
            functor(filename_sans_ext, std::as_const(*archived_file));
            //
            delete archived_file;
         }
      }
   }
}

namespace dovahkit::subsystems::papyrus {
   core::core() {
      QObject::connect(&this->_loose_pex.watcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString& path) {
         if (path == this->_loose_pex.folder_path) {
            {
               auto dir = QDir(path);
               if (!dir.exists()) {
                  //
                  // We've been notified about the watched directory being deleted.
                  //
                  this->_on_all_loose_pexs_deleted();
                  this->_loose_pex.folder_exists = false;
                  return;
               }
            }
            //
            // Something in the scripts folder has changed.
            //
            this->_check_for_loose_pex_updates();
         } else {
            if (this->_loose_pex.folder_exists) {
               return;
            }
            //
            // The scripts folder did not previously exist. Check to see if it exists now, 
            // and if so, update our monitoring.
            //
            this->_begin_watching_loose_pexs();
            if (this->_loose_pex.folder_exists) {
               //
               // Folder created; treat every PEX therein as newly created.
               //
               this->_check_for_loose_pex_updates();
            }
         }
      });

      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
         this->index_all_pex_files();
         this->_begin_watching_loose_pexs();
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->_stop_watching_loose_pexs();
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonComplete, this, [this]() {
         this->_teardown();
      });
   }
   core::~core() {
      this->_teardown();
   }

   void core::_teardown() {
      emit this->pexTeardownImminent();
      {
         auto& knowns = this->_known_scripts_by_name;
         for (auto& pair : knowns) {
            assert(pair.second != nullptr);
            delete pair.second;
         }
         knowns.clear();
      }
      emit this->pexTeardownComplete();
   }

   /*static*/ std::string core::_normalize_scriptname(std::string_view name) {
      std::string out(name);
      for (auto& c : out)
         if (c >= 'A' && c <= 'Z')
            c += 0x20;
      return out;
   }
   /*static*/ std::string core::_normalize_scriptname(QString name) {
      return _normalize_scriptname(name.toStdString());
   }

   known_script* core::_scan_pex(const std::string& filename_sans_ext, const uint8_t* src_data, const size_t src_size, bool is_loose) {
      for (const auto& entry : dovah::papyrus::native_classes) {
         if (dovah::papyrus::helpers::name_equals(entry.name, filename_sans_ext))
            //
            // There are PEX files for built-in classnames, but we actually want to ignore those, 
            // preferring hardcoded handling for them.
            //
            return nullptr;
      }

      using parser_type         = dovah::pex::parsers::class_info_collector;
      using shared_string_table = parser_type::shared_string_table_type; // TODO: may not even need this; investigate ditching it

      shared_string_table shared_strings;
      parser_type parser(shared_strings);
      parser.desired_classname = filename_sans_ext;
      try {
         parser.read_file((const char*)src_data, src_size);
      } catch (const dovah::compiled_papyrus_script::read_exception&) {
         return nullptr;
      }
      if (parser.results.name.empty())
         return nullptr;

      std::string name(parser.results.name);
      std::string name_normalized = _normalize_scriptname(name);

      known_script* dst_script = nullptr;
      {
         auto prior_it = this->_known_scripts_by_name.find(name_normalized);
         if (prior_it != this->_known_scripts_by_name.end())
            dst_script = prior_it->second;
      }
      if (dst_script) {
         if (is_loose)
            return dst_script;
      } else {
         dst_script = new known_script;
         dst_script->name = name;
         this->_known_scripts_by_name[name_normalized] = dst_script;
      }

      auto& info = (is_loose ? dst_script->info.loose : dst_script->info.packed).emplace();
      info.docstring      = parser.results.docstring;
      info.extends.name   = parser.results.superclass;
      info.extends.target = nullptr;
      //
      info.flags.conditional = parser.results.flags.conditional;
      info.flags.hidden      = parser.results.flags.hidden;
      return dst_script;
   }

   known_script* core::_scan_changed_pex(
      known_script&  dst_script,
      const uint8_t* src_data,
      const size_t   src_size,
      bool& out_basic_info_changed,
      bool& out_hierarchy_changed
   ) {
      out_basic_info_changed = false;
      out_hierarchy_changed  = false;

      using parser_type         = dovah::pex::parsers::class_info_collector;
      using shared_string_table = parser_type::shared_string_table_type; // TODO: may not even need this; investigate ditching it

      shared_string_table shared_strings;
      parser_type parser(shared_strings);
      parser.desired_classname = _normalize_scriptname(dst_script.name);
      try {
         parser.read_file((const char*)src_data, src_size);
      } catch (const dovah::compiled_papyrus_script::read_exception&) {
         return nullptr;
      }
      if (parser.results.name.empty())
         return nullptr;

      if (dst_script.name != parser.results.name) { // if letter case changed
         out_basic_info_changed = true;
         dst_script.name = parser.results.name;
      }

      auto& info_opt = dst_script.info.loose;
      if (!info_opt.has_value()) {
         auto& info = info_opt.emplace();
         info.docstring      = parser.results.docstring;
         info.extends.name   = parser.results.superclass;
         info.extends.target = nullptr;
         //
         info.flags.conditional = parser.results.flags.conditional;
         info.flags.hidden      = parser.results.flags.hidden;

         if (dst_script.info.packed.has_value()) {
            auto& p_info = dst_script.info.packed.value();
            if (p_info.docstring != parser.results.docstring)
               out_basic_info_changed = true;
            if (_normalize_scriptname(p_info.extends.name) != _normalize_scriptname(parser.results.superclass))
               out_hierarchy_changed = true;
            if (p_info.flags.conditional != parser.results.flags.conditional)
               out_basic_info_changed = true;
            if (p_info.flags.hidden != parser.results.flags.hidden)
               out_basic_info_changed = true;
         } else {
            out_basic_info_changed = true;
            out_hierarchy_changed  = true;
         }
      } else {
         out_basic_info_changed = false;
         out_hierarchy_changed  = false;

         auto& info = info_opt.value();
         if (info.docstring != parser.results.docstring) {
            out_basic_info_changed = true;
            info.docstring = parser.results.docstring;
         }
         if (_normalize_scriptname(info.extends.name) != _normalize_scriptname(parser.results.superclass)) {
            out_hierarchy_changed = true;
            info.extends.name = parser.results.superclass;
            //
            // Do not modify `info.extends.target`; it's our caller's responsibility to keep that up to date.
            //
         }
         if (info.flags.conditional != parser.results.flags.conditional) {
            out_basic_info_changed = true;
            info.flags.conditional = parser.results.flags.conditional;
         }
         if (info.flags.hidden != parser.results.flags.hidden) {
            out_basic_info_changed = true;
            info.flags.hidden = parser.results.flags.hidden;
         }
      }
      return &dst_script;
   }
   
   void core::_update_superclass_of(known_script& subject) {
      subject._compute_root_class({});
      emit knownScriptChanged(subject);

      auto* root = subject.inheritance.root_class;
      if (root == nullptr)
         root = &subject;

      subject.for_each_descendant_class([this, root](known_script& current) {
         current.inheritance.root_class = root;
         emit knownScriptChanged(current);
      });
   }

   //

   const known_script* core::lookup_known_script(std::string_view scriptname) const {
      auto it = this->_known_scripts_by_name.find(_normalize_scriptname(scriptname));
      if (it == this->_known_scripts_by_name.end())
         return nullptr;
      return it->second;
   }
   const known_script* core::lookup_known_script(QString scriptname) const {
      auto it = this->_known_scripts_by_name.find(_normalize_scriptname(scriptname));
      if (it == this->_known_scripts_by_name.end())
         return nullptr;
      return it->second;
   }

   void core::index_all_pex_files() {
      auto& core   = DovahKitCore::get();
      auto& assets = dovahkit::subsystems::assets::get();

      const auto* bsa_order    = assets.get_bsa_load_order();
      const auto  current_game = core.get_current_game();

      std::filesystem::path game_folder;
      core.get_game_path(game_folder, current_game);

      {  // PEX files i.e. compiled scripts
         _for_bsa_files_with_ext(
            bsa_order,
            "scripts",
            "pex",
            [this](const std::string& filename_sans_ext, const dovah::bsa_archived_file& archived_file) {
               _scan_pex(filename_sans_ext, (const uint8_t*)archived_file.data(), archived_file.size(), false);
            }
         );
         _for_loose_files_with_ext(
            QString::fromStdWString(game_folder.c_str()) + "\\Data\\scripts\\",
            "pex",
            [this](const std::string& filename_sans_ext, QFile& file) {
               auto  data    = file.readAll();
               auto* scanned = _scan_pex(filename_sans_ext, (const uint8_t*)data.constData(), data.size(), true);
               if (scanned) {
                  assert(scanned->info.loose.has_value());
                  auto info = QFileInfo(file);
                  scanned->info.loose.value().file_metadata = {
                     .lastmod = info.lastModified(),
                     .size    = (size_t)info.size(),
                  };
               }
            }
         );
      }
      //
      // Now that all scripts have been loaded and are known to the model, it's possible 
      // to associate subclasses with their superclasses.
      //
      std::vector<known_script*> phantoms; // See comments in lambda below.
      for (auto& pair : this->_known_scripts_by_name) {
         auto* script = pair.second;
         if (!script->exists())
            return;

         auto _process_info = [this, script, &phantoms](bool is_loose) -> void {
            auto& info_opt = is_loose ? (std::optional<known_script::per_file_info>&)script->info.loose : script->info.packed;
            if (!info_opt.has_value())
               return;
            auto&       info = info_opt.value();
            const auto& name = info.extends.name;
            if (name.empty())
               return;

            auto* superclass = info.extends.target = this->_lookup_known_script(name);
            if (superclass) {
               if (is_loose) {
                  superclass->receive_loose_subclass({}, *script);
               } else {
                  superclass->receive_archived_subclass({}, *script);
               }
               return;
            }
            //
            // We don't create `known_script`s for native classes. The superclass `name` in question 
            // didn't refer to another known script, so check if it refers to a native class. If so, 
            // then set the new known script's underlying type.
            //
            for (const auto& native : dovah::papyrus::native_classes) {
               if (dovah::papyrus::helpers::name_equals(name, native.name)) {
                  info.extends.underlying_type = native.form_type;
                  return;
               }
            }

            //
            // If we got here, then the script specified a superclass that doesn't actually exist. 
            // We're gonna wanna instantiate dummies for those, so that if the user creates a loose 
            // file for one of them post-load, we can more easily fix up the inheritance hierarchies 
            // on everything that inherits from it.
            //
            for (auto* phantom : phantoms) {
               if (phantom->name_matches(name)) {
                  superclass = phantom;
                  break;
               }
            }
            if (!superclass) {
               superclass = new known_script;
               superclass->name = name;
               phantoms.push_back(superclass);
            }
            if (is_loose) {
               superclass->receive_loose_subclass({}, *script);
            } else {
               superclass->receive_archived_subclass({}, *script);
            }
         };

         _process_info(false);
         _process_info(true);
      }
      //
      for (auto* phantom : phantoms) {
         this->_known_scripts_by_name[_normalize_scriptname(phantom->name)] = phantom;
      }
      //
      // And in turn, now that every script knows its superclass, we can ensure that every script 
      // also knows its root class i.e. the user-defined class at the very top of the inheritance 
      // hierarchy. This will allow us to more quickly query what native class (i.e. form or alias 
      // type), if any, a given script derives from.
      //
      for (auto& pair : this->_known_scripts_by_name) {
         auto* script = pair.second;
         script->_compute_root_class({});
      };
      
      emit pexIndexingComplete();
   }

   void core::_begin_watching_loose_pexs() {
      QString data_folder_path;

      auto& watcher = this->_loose_pex.watcher;

      {
         auto& core = DovahKitCore::get();
         auto& assets = dovahkit::subsystems::assets::get();

         const auto* bsa_order = assets.get_bsa_load_order();
         const auto  current_game = core.get_current_game();

         std::filesystem::path game_folder;
         core.get_game_path(game_folder, current_game);
         data_folder_path = QString::fromStdWString(game_folder.c_str()) + "\\Data\\";
         this->_loose_pex.folder_path = data_folder_path + "scripts\\";
      }
      this->_loose_pex.folder_exists = watcher.addPath(this->_loose_pex.folder_path);

      if (this->_loose_pex.folder_exists) {
         //
         // If we were previously watching the Data folder, then we can stop now; the scripts folder 
         // exists.
         //
         watcher.removePath(data_folder_path);
      } else {
         //
         // The scripts folder doesn't exist yet. Watch the Data folder, so that if the scripts folder 
         // is created, we can start trying to monitor it.
         //
         watcher.addPath(data_folder_path);
      }
   }
   void core::_stop_watching_loose_pexs() {
      auto& watcher = this->_loose_pex.watcher;
      watcher.removePaths(watcher.directories());

      this->_loose_pex.folder_exists = false;
      this->_loose_pex.folder_path.clear();
   }
   void core::_check_for_loose_pex_updates() {
      std::unordered_map<std::string, known_script*> unseen_looses;
      for (auto& pair : this->_known_scripts_by_name) {
         assert(pair.second != nullptr);
         if (pair.second->info.loose.has_value())
            unseen_looses[pair.first] = pair.second;
      }

      std::vector<known_script*> new_scripts;
      std::vector<known_script*> basic_info_changes; // name capitalization, docstring, flags, but only if no hierarchy changes
      std::vector<known_script*> hierarchy_changes;  // base class changes

      _for_loose_files_with_ext(
         this->_loose_pex.folder_path,
         "pex",
         [this, &unseen_looses, &new_scripts, &basic_info_changes, &hierarchy_changes](const std::string& filename_sans_ext, QFile& file) {
            auto info = QFileInfo(file);
            auto meta = known_script::loose_file_metadata{
               .lastmod = info.lastModified(),
               .size    = (size_t)info.size(),
            };

            auto* existing = this->_lookup_known_script(filename_sans_ext);
            if (existing) {
               if (existing->info.loose.has_value()) {
                  unseen_looses.erase(_normalize_scriptname(existing->name)); // Mark this script as "seen."
                  if (meta == existing->info.loose.value().file_metadata) {
                     //
                     // File doesn't appear to have been changed. Ignore it.
                     //
                     return;
                  }
               }
            }

            bool hierarchy_changed  = false;
            bool basic_info_changed = false;

            known_script* scanned = nullptr;
            {
               auto data = file.readAll();
               if (existing) {
                  scanned = _scan_changed_pex(*existing, (const uint8_t*)data.constData(), data.size(), basic_info_changed, hierarchy_changed);
               } else {
                  scanned = _scan_pex(filename_sans_ext, (const uint8_t*)data.constData(), data.size(), true);
               }
            }
            if (existing) {
               if (hierarchy_changed) {
                  hierarchy_changes.push_back(existing);
               } else if (basic_info_changed) {
                  basic_info_changes.push_back(existing);
               }
            } else {
               if (scanned) {
                  new_scripts.push_back(scanned);
               }
            }
            if (scanned) {
               assert(scanned->info.loose.has_value());
               scanned->info.loose.value().file_metadata = meta;
               new_scripts.push_back(scanned);
            }
         }
      );

      //
      // Now, as before, we need to hook up class hierarchies for new scripts, and find phantoms 
      // referenced in them as well.
      //
      std::vector<known_script*> phantoms;
      //
      for (auto* script : new_scripts) {
         assert(script != nullptr);
         assert(script->info.loose.has_value());
         auto&       info = script->info.loose.value();
         const auto& name = info.extends.name;
         if (name.empty())
            continue;

         auto* superclass = info.extends.target = this->_lookup_known_script(name);
         if (superclass) {
            superclass->receive_loose_subclass({}, *script);
            continue;
         }
         //
         // We don't create `known_script`s for native classes. The superclass `name` in question 
         // didn't refer to another known script, so check if it refers to a native class. If so, 
         // then set the new known script's underlying type.
         //
         for (const auto& native : dovah::papyrus::native_classes) {
            if (dovah::papyrus::helpers::name_equals(name, native.name)) {
               info.extends.underlying_type = native.form_type;
               continue;
            }
         }

         //
         // If we got here, then the script specified a superclass that doesn't actually exist. 
         // We're gonna wanna instantiate dummies for those, so that if the user creates a loose 
         // file for one of them post-load, we can more easily fix up the inheritance hierarchies 
         // on everything that inherits from it.
         //
         for (auto* phantom : phantoms) {
            if (phantom->name_matches(name)) {
               superclass = phantom;
               break;
            }
         }
         if (!superclass) {
            superclass = new known_script;
            superclass->name = name;
            phantoms.push_back(superclass);
         }
         superclass->receive_loose_subclass({}, *script);
      }

      //
      // We also need to check whether any existing scripts with hierarchy changes have been 
      // changed to inherit from a phantom.
      //
      std::vector<known_script*> indirect_hierarchy_changes;
      //
      for (auto* script : hierarchy_changes) {
         assert(script != nullptr);
         assert(script->info.loose.has_value());
         auto&       info = script->info.loose.value();
         const auto& name = info.extends.name;

         auto* former_superclass = info.extends.target;
         if (former_superclass)
            former_superclass->abandon_loose_subclass({}, *script);

         if (name.empty()) {
            //
            // Script was edited to have no superclass.
            //
            continue;
         }
         //
         // Check for native base classes.
         //
         for (const auto& native : dovah::papyrus::native_classes) {
            if (dovah::papyrus::helpers::name_equals(name, native.name)) {
               info.extends.underlying_type = native.form_type;
               continue;
            }
         }

         //
         // Check for phantoms.
         //
         known_script* superclass = nullptr;
         for (auto* phantom : phantoms) {
            if (phantom->name_matches(name)) {
               superclass = phantom;
               break;
            }
         }
         if (!superclass) {
            superclass = new known_script;
            superclass->name = name;
            phantoms.push_back(superclass);
         }
         superclass->receive_loose_subclass({}, *script);
      }

      //
      // Now, we need to send appropriate signals for everything, including forgetting scripts.
      //

      for (auto* script : new_scripts) {
         emit knownScriptDiscovered(*script);
      }

      for (auto* script : basic_info_changes) {
         emit knownScriptChanged(*script);
      }
      for (auto* script : hierarchy_changes) {
         //
         // Recompute class hierarchies for the descendant classes of `script`, and emit `knownScriptChanged` 
         // on `script` and any altered descendants.
         //
         this->_update_superclass_of(*script);
      }

      //
      // Dealing with scripts that have lost their PEX file is a two-stage process. We want to 
      // only forget such a script if it's no longer attached to a form, has no archived PEX, 
      // and isn't a potential subclass to another form?
      // 
      // That last one is key. What if Scripts A and B both lose their PEX files, and B was a 
      // subclass of A? If we only do a single pass over the scripts, then we'll think that A 
      // should remain known by virtue of B, even though B is going to be forgotten. Thus, we 
      // need a two-pass approach. In the first pass, we ditch each unseen script's loose PEX 
      // info, severing its connection as a "potential subclass of [whatever] by virtue of a 
      // loose PEX." In the second pass, with that connection severed, we can reliably test 
      // whether each unseen script is indeed due to be forgotten.
      //
      for (auto& pair : unseen_looses) {
         auto* script = pair.second;
         assert(script != nullptr);

         auto* loose_super = script->info.loose.value().extends.target;
         script->info.loose.reset();
         if (loose_super) {
            loose_super->abandon_loose_subclass({}, *script);
         }
      }
      for (auto& pair : unseen_looses) {
         auto* script = pair.second;

         bool  retain = script->info.packed.has_value();
         if (!retain)
            retain = !script->is_unreferenced();

         if (retain) {
            //
            // Script will remain known. Fix up the inheritance hierarchy (e.g. root script 
            // pointers, etc.) for it and its descendants, and emit script-changed signals 
            // for each of them.
            //
            this->_update_superclass_of(*script);
         } else {
            emit knownScriptAboutToBeForgotten(*script);
            this->_known_scripts_by_name.erase(pair.first);
            delete script;
            emit knownScriptForgotten(pair.first);
         }
      }
   }
   void core::_on_all_loose_pexs_deleted() {
      //
      // This should all be essentially the same logic as `unseen_looses` in the general 
      // handler for loose PEX changes. We could almost copy and paste the code, except 
      // that we still need to put together a list of scripts to loop over for the second 
      // pass, and we may as well do that during the first pass.
      //
      std::unordered_map<std::string, known_script*> altered_looses;
      for (auto& pair : this->_known_scripts_by_name) {
         auto* script = pair.second;
         assert(script != nullptr);

         if (script->info.loose.has_value()) {
            auto* loose_superclass = script->info.loose.value().extends.target;
            script->info.loose.reset();
            if (loose_superclass) {
               loose_superclass->abandon_loose_subclass({}, *script);
            }

            altered_looses[pair.first] = pair.second;
         }
      }
      for (auto& pair : altered_looses) {
         auto* script = pair.second;

         bool  retain = script->info.packed.has_value();
         if (!retain)
            retain = !script->is_unreferenced();

         if (retain) {
            //
            // Script will remain known. Fix up the inheritance hierarchy (e.g. root script 
            // pointers, etc.) for it and its descendants, and emit script-changed signals 
            // for each of them.
            //
            this->_update_superclass_of(*script);
         } else {
            emit knownScriptAboutToBeForgotten(*script);
            this->_known_scripts_by_name.erase(pair.first);
            delete script;
            emit knownScriptForgotten(pair.first);
         }
      }
   }
}