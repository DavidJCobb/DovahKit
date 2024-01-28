#include "./DKAllKnownPapyrusScriptsModel.h"
#include <QDirIterator>
#include "helpers/string/strieq_ascii.h"
#include "dovah/files/bsa/bsa_archive.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/bsa/bsa_load_order.h"
#include "editor/subsystems/assets.h"
#include "editor/core.h"

namespace {
   constexpr const bool load_psc_files_too = false;
}

DKAllKnownPapyrusScriptsModel::~DKAllKnownPapyrusScriptsModel() {
   this->clear();
}

void DKAllKnownPapyrusScriptsModel::clear();

/*static*/ void DKAllKnownPapyrusScriptsModel::_scanPex(working_collection_type& dst_collection, const void* src_data, const size_t src_size) {
   dovah::compiled_papyrus_script pex;
   try {
      pex.read_file(src_data, src_size);
   } catch (const dovah::compiled_papyrus_script::read_exception&) {
      return;
   }

   if (pex.objects.empty())
      return;

   uint32_t flag_conditional = 0;
   uint32_t flag_hidden      = 0;
   for (const auto& flag_dfn : pex.user_flags) {
      if (cobb::strieq_ascii(flag_dfn.name, "hidden")) {
         flag_hidden = 1 << flag_dfn.bit_index;
      } else if (cobb::strieq_ascii(flag_dfn.name, "conditional")) {
         flag_conditional = 1 << flag_dfn.bit_index;
      }
      if (flag_hidden && flag_conditional)
         break;
   }

   for (const auto& script : pex.objects) {
      auto name       = QString::fromStdString(script.name);
      auto name_lower = name.toLower();
      if (dst_collection.contains(name_lower))
         continue;

      auto& dst = dst_collection[name_lower];
      dst.scriptname  = name;
      dst.docstring   = QString::fromStdString(script.docstring);
      dst.is_compiled = true;
      //
      if (flag_conditional && (script.user_flags & flag_conditional))
         dst.flags.conditional = true;
      if (flag_hidden && (script.user_flags & flag_hidden))
         dst.flags.hidden = true;
   }
}

namespace {
   template<typename Functor, typename FailFunctor>
   void _for_loose_files_with_ext(QString path, const char* desired_ext, Functor&& functor, FailFunctor&& functor_for_fail) {
      QDirIterator it(path);
      while (it.hasNext()) {
         auto path = it.next();
         auto info = QFileInfo(path);
         auto ext  = info.completeSuffix().toLower(); // great naming here
         if (ext != desired_ext)
            continue;

         auto file = QFile(path);
         if (!file.open(QIODevice::OpenModeFlag::ExistingOnly | QIODevice::OpenModeFlag::ReadOnly)) {
            functor_for_fail(info.baseName());
            // TODO: Retain the script but mark it as unreadable? Or see if there's an overridden file (i.e. in a BSA) we can read as a fallback?
            continue;
         }
         functor(file);
      }
   }

   // Path should use backslashes as separators and not have leading or trailing slashes.
   template<typename Functor>
   void _for_bsa_files_with_ext(const dovah::bsa_load_order* bsa_order, const char* path, const char* desired_ext, Functor&& functor) noexcept {
      if (!bsa_order)
         return;

      dovah::bs_hash folder_hash(path, nullptr);

      auto& bsa_list = bsa_order->get_archive_list(); // TODO: this is intended to give us const access to the BSAs, but since it's a vector of pointers, we have non-const access too
      for (auto it = bsa_list.rbegin(); it != bsa_list.rend(); ++it) {
         //
         // We iterate over BSAs in reverse order because files packed in the BSAs at the end 
         // of the load order will override files of the same name and path packed in BSAs 
         // earlier in the load order. We can early-out on a packed script if we go in reverse 
         // order and the scriptname is one we've already seen before. (This is also why we 
         // go over loose files before BSAs.)
         //
         const auto* bsa = *it;
         if (!bsa->retains_filenames())
            //
            // Files in this archive are identified only by hash; there's no way to recover the 
            // original file extension.
            //
            continue;

         const auto* folder_info = bsa->lookup_folder_info(folder_hash, path);
         if (!folder_info)
            continue;
         
         for (const auto& file_info : folder.files) {
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

            auto* archived_file = bsa.read_contents_of(file_info);
            if (!archived_file)
               continue;
            //
            functor(std::as_const(*archived_file));
            //
            delete archived_file;
         }
      }
   }
}
void DKAllKnownPapyrusScriptsModel::populate() {
   static_assert(false,
      "TODO: This method is conceptually flawed: what if script files are created after we populate?"
            "I think what we really need to do is use a QHash as the underlying storage, and be able "
            "to incrementally add stuff i.e. re-scan all loose files (but not BSAs; those should not "
            "be altered post-load) to look for additions or removals. Alongside the QHash, we'd need "
            "a QVector of scriptnames in order to allow for the consistent ordering that Qt models "
            "need; if we're careful, we can handle insertions and deletions individually instead of "
            "needing a model reset on each update scan."
   );

   auto& core   = DovahKitCore::get();
   auto& assets = dovahkit::subsystems::assets::get();

   const auto* bsa_order    = assets.get_bsa_load_order();
   const auto  current_game = core.get_current_game();

   this->beginResetModel();

   this->_scripts.clear();

   std::filesystem::path game_folder;
   core.get_game_path(game_folder, current_game);

   working_collection_type working;
   working.reserve(10015); // number of PEX files in Skyrim - Misc.bsa in Skyrim Classic
   //working.reserve(13730); // number of PSC files in a Skyrim Classic install with all DLCs, after unpacking scripts.rar

   {  // PEX files i.e. compiled scripts
      _for_loose_files_with_ext(
         QString::fromStdWString(game_folder.c_str()) + "\\Data\\scripts\\",
         "pex",
         [&working](QFile file) {
            auto data = file.readAll();
            _scanPex(working, data.constData(), data.size());
         },
         [&working](QString filename_sans_ext) {
            // TODO: Retain the script but mark it as unreadable? Or see if there's an overridden file (i.e. in a BSA) we can read as a fallback?
         }
      );
      _for_bsa_files_with_ext(
         bsa_order,
         "scripts",
         "pex",
         [&working](const dovah::bsa_archived_file& archived_file) {
            _scanPex(working, archived_file.data(), archived_file.size());
         }
      );
   }
   if constexpr (load_psc_files_too) { // PSC files i.e. script source code
      std::filesystem::path stem;
      if (current_game == dovah::game::skyrim_classic) {
         stem = "scripts/source/";
      } else {
         stem = "source/scripts/"; // TODO: wait, isn't this path actually an INI setting? can we pull it from there? if it's an INI setting, we should use it.
      }
      
      _for_loose_files_with_ext(
         QString::fromStdWString(game_folder.c_str()) + "\\Data\\scripts\\",
         "psc",
         [&working](QFile file) {
            static_assert(false, "TODO: Handle PSC file. Should we even bother loading it and attempting to parse it, or just store it as a scriptname with no known metadata?");
         },
         [&working](QString filename_sans_ext) {
            // TODO: Retain the script but mark it as unreadable? Or see if there's an overridden file (i.e. in a BSA) we can read as a fallback?
         }
      );
      _for_bsa_files_with_ext(
         bsa_order,
         stem.string().c_str(), // TODO: must convert forward slashes to backslashes and strip leading and trailing slashes first, or this won't match!!
         "psc",
         [&working](const dovah::bsa_archived_file& archived_file) {
            static_assert(false, "TODO: Handle PSC file. Should we even bother loading it and attempting to parse it, or just store it as a scriptname with no known metadata?");
         }
      );
   }

   this->_scripts.reserve(working.size());
   for (auto& item : working) {
      this->_scripts.push_back(std::move(item));
   }
   working.clear();
   this->_scripts.sort();

   this->endResetModel();
}