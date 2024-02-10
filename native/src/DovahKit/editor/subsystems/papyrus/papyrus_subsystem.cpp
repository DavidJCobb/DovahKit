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
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
         // TODO
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         // TODO
      });
   }
   core::~core() {
      {
         auto& knowns = this->_known_scripts_by_name;
         for (auto& pair : knowns) {
            assert(pair.second != nullptr);
            delete pair.second;
         }
         knowns.clear();
      }
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
   
   void core::_update_superclass_of(known_script& subject) {
      auto* prior_root = subject.inheritance.root_class;
      subject._compute_root_class({});
      if (subject.inheritance.root_class == prior_root)
         return;
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
               auto data = file.readAll();
               _scan_pex(filename_sans_ext, (const uint8_t*)data.constData(), data.size(), true);
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
            auto& info_opt = is_loose ? script->info.loose : script->info.packed;
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
}