#include "./DKPapyrusModel.h"
#include <type_traits>
#include <QDirIterator>
//
#include "helpers/string/strieq_ascii.h"
//
#include "dovah/files/bsa/bsa_archive.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/bsa/bsa_load_order.h"
#include "editor/subsystems/assets.h"
#include "editor/core.h"

/*

   The design of the model is as follows:

   A script can be defined in two places: a BSA-archived ("packed") file or a loose file. 
   We have a read lock on BSAs while game data is loaded, so those files are pretty much 
   immutable. Loose files, however, can be created, edited, or deleted out from under us. 
   There are ways for us to detect and react to those changes, to keep the model up to 
   date in real-time.

   As such, each `Script` in the model can store information for one packed file and one 
   loose file. This information includes the docstring, superclass, and flags. In turn, 
   this means that a known script's superclass can *change* if a loose file changes.

   There's another complication as well: a script can subclass a missing file, and that 
   file can be created post-load; or a script can subclass a file which is deleted and 
   then recreated (or simply undeleted). This means that we need to track not only the 
   scripts that actually exist, but also any referenced-but-missing scripts as well.

   ---

   The model is designed for upward traversal: given a scriptname, you can query whether 
   it inherits from any other script, or whether it's attachable to a given form type or 
   alias type.

   Scripts keep track of potential subclasses so that when a script's own superclass is 
   changed, the script can update its potential subclasses as necessary. (Additionally, 
   if: a script exists only as a loose file, has no subclasses, and is attached to no 
   forms or aliases; and that loose file is deleted; then we should delete the script 
   from the model, and of course this requires being able to quickly tell that the 
   script is unreferenced.) Information for downward traversal, then, is maintained 
   only to ensure the integrity of the model.

*/

namespace {
   struct form_type_scriptname {
      dovah::form_type_t form_type;
      const char* scriptname;

      dovah::form_type_t extends = dovah::form_type::none;
   };

   // NOTE: We're only counting ones built into the game engine itself, not ones added by SKSE.
   constexpr const auto all_form_type_scriptnames = std::array{
      form_type_scriptname{ dovah::form_type::alias,                "Alias" },
      form_type_scriptname{ dovah::form_type::reference_alias,      "ReferenceAlias",      dovah::form_type::alias },
      form_type_scriptname{ dovah::form_type::location_alias,       "LocationAlias",       dovah::form_type::alias },

      form_type_scriptname{ dovah::form_type::active_magic_effect,  "ActiveMagicEffect" },

      form_type_scriptname{ dovah::form_type::none,                 "Form" },
      //                    //                                      //
      form_type_scriptname{ dovah::form_type::activator,            "Activator" },
      form_type_scriptname{ dovah::form_type::actor,                "Actor",               dovah::form_type::reference },
      form_type_scriptname{ dovah::form_type::actor_base,           "ActorBase" },
      form_type_scriptname{ dovah::form_type::ammo,                 "Ammo" },
      form_type_scriptname{ dovah::form_type::apparatus,            "Apparatus",           dovah::form_type::misc_item },
      form_type_scriptname{ dovah::form_type::armor,                "Armor" },
      form_type_scriptname{ dovah::form_type::association_type,     "AssociationType" },
      form_type_scriptname{ dovah::form_type::book,                 "Book" },
      form_type_scriptname{ dovah::form_type::cell,                 "Cell" },
      form_type_scriptname{ dovah::form_type::combat_class,         "Class" },
      form_type_scriptname{ dovah::form_type::constructible_object, "ConstructibleObject", dovah::form_type::misc_item },
      form_type_scriptname{ dovah::form_type::container,            "Container" },
      form_type_scriptname{ dovah::form_type::door,                 "Door" },
      form_type_scriptname{ dovah::form_type::effect_shader,        "EffectShader" },
      form_type_scriptname{ dovah::form_type::enchantment,          "Enchantment" },
      form_type_scriptname{ dovah::form_type::encounter_zone,       "EncounterZone" },
      form_type_scriptname{ dovah::form_type::explosion,            "Explosion" },
      form_type_scriptname{ dovah::form_type::faction,              "Faction" },
      form_type_scriptname{ dovah::form_type::flora,                "Flora",               dovah::form_type::activator },
      form_type_scriptname{ dovah::form_type::formlist,             "FormList" },
      form_type_scriptname{ dovah::form_type::furniture,            "Furniture",           dovah::form_type::activator },
      form_type_scriptname{ dovah::form_type::global,               "GlobalVariable" },
      form_type_scriptname{ dovah::form_type::hazard,               "Hazard" },
      form_type_scriptname{ dovah::form_type::idle,                 "Idle" },
      form_type_scriptname{ dovah::form_type::imagespace_modifier,  "ImageSpaceModifier" },
      form_type_scriptname{ dovah::form_type::impact_data_set,      "ImpactDataSet" },
      form_type_scriptname{ dovah::form_type::ingredient,           "Ingredient" },
      form_type_scriptname{ dovah::form_type::key,                  "Key",                 dovah::form_type::misc_item },
      form_type_scriptname{ dovah::form_type::keyword,              "Keyword" },
      form_type_scriptname{ dovah::form_type::location_ref_type,    "LocationRefType",     dovah::form_type::keyword },
      form_type_scriptname{ dovah::form_type::leveled_character,    "LeveledActor" },
      form_type_scriptname{ dovah::form_type::leveled_item,         "LeveledItem" },
      form_type_scriptname{ dovah::form_type::leveled_spell,        "LeveledSpell" },
      form_type_scriptname{ dovah::form_type::light,                "Light" },
      form_type_scriptname{ dovah::form_type::location,             "Location" },
      form_type_scriptname{ dovah::form_type::magic_effect,         "MagicEffect" },
      form_type_scriptname{ dovah::form_type::message,              "Message" },
      form_type_scriptname{ dovah::form_type::misc_item,            "MiscObject" },
      form_type_scriptname{ dovah::form_type::music_type,           "MusicType" },
      form_type_scriptname{ dovah::form_type::reference,            "ObjectReference" },
      form_type_scriptname{ dovah::form_type::outfit,               "Outfit" },
      form_type_scriptname{ dovah::form_type::package,              "Package" },
      form_type_scriptname{ dovah::form_type::perk,                 "Perk" },
      form_type_scriptname{ dovah::form_type::potion,               "Potion" },
      form_type_scriptname{ dovah::form_type::projectile,           "Projectile" },
      form_type_scriptname{ dovah::form_type::quest,                "Quest" },
      form_type_scriptname{ dovah::form_type::race,                 "Race" },
      form_type_scriptname{ dovah::form_type::scene,                "Scene" },
      form_type_scriptname{ dovah::form_type::scroll,               "Scroll" },
      form_type_scriptname{ dovah::form_type::shout,                "Shout" },
      form_type_scriptname{ dovah::form_type::sound,                "Sound" },
      form_type_scriptname{ dovah::form_type::sound_category,       "SoundCategory" },
      form_type_scriptname{ dovah::form_type::soul_gem,             "SoulGem",             dovah::form_type::misc_item },
      form_type_scriptname{ dovah::form_type::spell,                "Spell" },
      form_type_scriptname{ dovah::form_type::statik,               "Static" },
      form_type_scriptname{ dovah::form_type::talking_activator,    "TalkingActivator",    dovah::form_type::activator },
      form_type_scriptname{ dovah::form_type::texture_set,          "TextureSet" },
      form_type_scriptname{ dovah::form_type::topic,                "Topic" },
      form_type_scriptname{ dovah::form_type::topic_info,           "TopicInfo" },
      form_type_scriptname{ dovah::form_type::reference_effect,     "VisualEffect" },
      form_type_scriptname{ dovah::form_type::voicetype,            "VoiceType" },
      form_type_scriptname{ dovah::form_type::weapon,               "Weapon" },
      form_type_scriptname{ dovah::form_type::word_of_power,        "WordOfPower" },
      form_type_scriptname{ dovah::form_type::worldspace,           "Worldspace" },
   };

   // Helpers, for faster lookups to see if one form type is a subclass of another.
   struct form_type_inheritance {
      dovah::form_type_t subclass;
      dovah::form_type_t superclass;
   };
   constexpr const auto all_form_type_inheritance = []() {
      constexpr size_t count = []() {
         size_t c = 0;
         for (const auto& mapping : all_form_type_scriptnames)
            if (mapping.extends != dovah::form_type::none)
               ++c;
         return c;
      }();

      std::array<form_type_inheritance, count> out = {};
      for (const auto& mapping : all_form_type_scriptnames) {
         if (mapping.extends == dovah::form_type::none)
            continue;

         for (size_t i = 0; i < out.size(); ++i) {
            if (out[i].subclass == dovah::form_type::none) {
               out[i].subclass   = mapping.form_type;
               out[i].superclass = mapping.extends;
               break;
            }
         }
      }
      return out;
   }();

   constexpr dovah::form_type_t _superclass_of(dovah::form_type_t ft) {
      for (const auto& item : all_form_type_inheritance)
         if (item.subclass == ft)
            return item.superclass;
      return dovah::form_type::none;
   }
}

namespace {
   int _compare_name(QString name_a, QString name_b) {
      size_t size_a = name_a.size();
      size_t size_b = name_b.size();

      size_t shortest = size_a;
      if (shortest > size_b)
         shortest = size_b;

      for (size_t i = 0; i < shortest; ++i) {
         auto a = name_a[(uint)i].unicode();
         auto b = name_b[(uint)i].unicode();
         //
         // Mimic game's case-insensitivity.
         //
         if (a >= 'A' && a <= 'Z')
            a += 0x20;
         if (b >= 'A' && b <= 'Z')
            b += 0x20;

         if (a < b)
            return -1;
         if (a > b)
            return 1;
      }
      if (size_a < size_b)
         return -1;
      if (size_a > size_b)
         return 1;

      return 0;
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

      const auto folder_hash = dovah::bs_hash(path, nullptr);

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

         for (const auto& file_info : folder_info.files) {
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
            functor(file_name, std::as_const(*archived_file));
            //
            delete archived_file;
         }
      }
   }
}

#pragma region DKPapyrusModel::Script
int DKPapyrusModel::Script::compare_name(QString n) const {
   return ::_compare_name(this->name, n);
}
bool DKPapyrusModel::Script::name_matches(QString n) const {
   size_t size = this->name.size();
   if (n.size() != size)
      return false;
   for (size_t i = 0; i < size; ++i) {
      auto a = this->name[(uint)i].unicode();
      auto b = n[(uint)i].unicode();
      //
      // Mimic game's case-insensitivity.
      //
      if (a >= 'A' && a <= 'Z')
         a += 0x20;
      if (b >= 'A' && b <= 'Z')
         b += 0x20;

      if (a != b)
         return false;
   }
   return true;
}
const DKPapyrusModel::Script* DKPapyrusModel::Script::superclass() const {
   if (this->info.loose.has_value())
      return this->info.loose.value().extends.target;
   if (this->info.packed.has_value())
      return this->info.packed.value().extends.target;
   return nullptr;
}
DKPapyrusModel::Script* DKPapyrusModel::Script::superclass() {
   return const_cast<Script*>(std::as_const(*this).superclass());
}

bool DKPapyrusModel::Script::is_of_type(dovah::form_type_t desired) const {
   auto ut_opt = this->underlying_type();
   if (!ut_opt.has_value())
      return false;
   auto my_type = ut_opt.value();
   if (my_type == desired)
      return true;
   for (auto ft = _superclass_of(desired); ft != dovah::form_type::none; ft = _superclass_of(ft))
      if (my_type == ft)
         return true;
   return false;
}
bool DKPapyrusModel::Script::is_of_type(QString supername) const {
   if (this->name_matches(supername))
      return true;

   const auto* sc = this;
   while (sc = sc->superclass())
      if (sc->name_matches(supername))
         return true;
   
   return false;
}
std::optional<dovah::form_type_t> DKPapyrusModel::Script::underlying_type() const {
   if (this->inheritance.root_class)
      return this->inheritance.root_class->underlying_type();

   if (this->info.loose.has_value())
      return this->info.loose.value().extends.underlying_type;
   if (this->info.packed.has_value())
      return this->info.packed.value().extends.underlying_type;

   return {};
}

bool DKPapyrusModel::Script::is_unreferenced() const {
   //
   // We use `is_unreferenced` to check if a Script is safe to delete.
   //
   {
      auto& list_set = this->inheritance.potential_subclasses;
      if (!list_set.loose.empty())
         return false;
      if (!list_set.packed.empty())
         return false;
   }
   //
   // TODO: Once we start tracking what forms have what scripts attached, we'd want to 
   // return `false` here if any form or alias has this script attached.
   //
   return true;
}

void DKPapyrusModel::Script::receive_subclass(Script& subclass, bool loose) {
   auto& list_set = this->inheritance.potential_subclasses;
   auto& list     = loose ? list_set.loose : list_set.packed;
   //
   auto i = list.indexOf(&subclass);
   if (i >= 0)
      return;
   list.push_back(&subclass);
}
void DKPapyrusModel::Script::abandon_loose_subclass(Script& subclass) {
   auto& list = this->inheritance.potential_subclasses.loose;
   auto  i    = list.indexOf((Script*) &subclass); // ugh, what a dumb cast
   if (i < 0)
      return;
   list.remove(i);
}

void DKPapyrusModel::Script::_compute_root_class() {
   this->inheritance.cyclical   = false;
   this->inheritance.root_class = nullptr;

   QVector<Script*> seen;
   for (auto* script = this; script; script = script->superclass()) {
      if (script->inheritance.cyclical || seen.contains(script)) {
         this->inheritance.cyclical = true;
         break;
      }
      if (auto* already_computed_root = script->inheritance.root_class) {
         seen.push_back(already_computed_root);
         break;
      }
      seen.push_back(script);
   }
   if (!this->inheritance.cyclical && seen.size() > 1) {
      this->inheritance.root_class = seen.back();
   }
}
void DKPapyrusModel::Script::_update_descendants_root_class() {
   auto& list_set = this->inheritance.potential_subclasses;
   for (auto* child : list_set.loose) {
      if (child->inheritance.root_class == this->inheritance.root_class)
         continue;
      if (child->superclass() != this)
         continue;
      child->inheritance.root_class = this->inheritance.root_class;
      child->_update_descendants_root_class();
   }
   for (auto* child : list_set.packed) {
      if (child->inheritance.root_class == this->inheritance.root_class)
         continue;
      if (child->superclass() != this)
         continue;
      child->inheritance.root_class = this->inheritance.root_class;
      child->_update_descendants_root_class();
   }
}
#pragma endregion

//
// A `script_collection` stores scripts sorted alphabetically, and keeps track of how 
// many scripts begin with an underscore or with each letter. It uses the latter info 
// for faster lookups.
//
#pragma region DKPapyrusModel::script_collection
/*static*/ size_t DKPapyrusModel::script_collection::_which_count(QString name) {
   auto leading = name[0].unicode();
   if (leading >= 'A' && leading <= 'Z')
      leading += 0x20;

   if (leading == '_')
      return 0;
   else if (leading >= 'a' && leading <= 'z')
      return ('a' - 1);

   return std::tuple_size_v<decltype(counts)>;
}

const DKPapyrusModel::Script* DKPapyrusModel::script_collection::at(size_t i) const {
   if (i >= this->scripts.size())
      return nullptr;
   return this->scripts[i];
}
DKPapyrusModel::Script* DKPapyrusModel::script_collection::at(size_t i) {
   if (i >= this->scripts.size())
      return nullptr;
   return this->scripts[i];
}

void DKPapyrusModel::script_collection::clear() {
   for (auto* script : this->scripts)
      delete script;
   this->scripts.clear();
   this->counts = {};
}

size_t DKPapyrusModel::script_collection::index_of(QString name) const {
   if (name.isEmpty())
      return index_of_none;

   size_t skip = 0;
   {
      auto leading = _which_count(name);
      if (leading < this->counts.size())
         for (size_t i = 0; i < leading; ++i)
            skip += this->counts[i];
   }
   for (size_t i = skip; i < this->scripts.size(); ++i) {
      auto* script = this->scripts[i];
      assert(script != nullptr);

      auto cmp = script->compare_name(name);
      if (cmp == 0)
         return i;
      if (cmp > 0)
         break;
   }
   return index_of_none;
}
void DKPapyrusModel::script_collection::insert(Script* script) {
   size_t start_at    = 0;
   size_t which_count = _which_count(script->name);
   if (which_count < this->counts.size())
      for (size_t i = 0; i < which_count; ++i)
         start_at += this->counts[i];

   size_t insert_at = this->scripts.size();
   for (size_t i = start_at; i < this->scripts.size(); ++i) {
      auto cmp = this->scripts[i]->compare_name(script->name);
      assert(cmp != 0);
      if (cmp > 0) {
         insert_at = i;
         break;
      }
   }
   this->scripts.insert(insert_at, script);
   if (which_count < this->counts.size())
      this->counts[which_count]++;
}
const DKPapyrusModel::Script* DKPapyrusModel::script_collection::lookup(QString name) const {
   size_t i = this->index_of(name);
   if (i != index_of_none)
      return this->scripts[i];
   return nullptr;
}
DKPapyrusModel::Script* DKPapyrusModel::script_collection::lookup(QString name) {
   return const_cast<Script*>(std::as_const(*this).lookup(name));
}

void DKPapyrusModel::script_collection::take(Script& n) {
   size_t i = this->index_of(n.name);
   assert(i != index_of_none);
   assert(this->scripts[i] == &n);
   this->scripts.remove(i);

   auto wc = _which_count(n.name);
   if (wc < this->counts.size())
      this->counts[wc]--;
}
#pragma endregion

#pragma region DKPapyrusModel
DKPapyrusModel::DKPapyrusModel(QObject* parent) {
   auto& editor = DovahKitCore::get();

   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
      //
      // TODO: Multi-thread the BSA scan?
      //
      this->populate_initial();
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->beginResetModel();
      this->_data.clear();
      this->endResetModel();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub*, bool will_be_flagged) {
      //
      // TODO: When a form is deleted, discard all information about what Papyrus scripts it has attached.
      //       (This to-do item will become relevant once we start tracking that in the first place...)
      //
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub*) {
      //
      // TODO: When a form is modified, re-check all information about what Papyrus scripts it has attached.
      //       (This to-do item will become relevant once we start tracking that in the first place...)
      //
   });
}
DKPapyrusModel::~DKPapyrusModel() {
   this->_data.clear();
}


void DKPapyrusModel::_update_superclass_of(Script& subject) {
   auto* prior_root = subject.inheritance.root_class;
   subject._compute_root_class();
   if (subject.inheritance.root_class == prior_root)
      return;
   {
      auto i   = this->_data.index_of(subject.name);
      auto qmi = this->index(i, 0, {});
      emit dataChanged(qmi, qmi);
   }

   auto* root = subject.inheritance.root_class;
   if (root == nullptr)
      root = &subject;

   [this, root](this auto&& recurse, Script& current) -> void {
      auto& list_set = current.inheritance.potential_subclasses;
      for (auto* child : list_set.loose) {
         if (child->inheritance.root_class == root)
            continue;
         if (child->superclass() != &current)
            continue;
         child->inheritance.root_class = root;
         {
            auto i   = this->_data.index_of(child->name);
            auto qmi = this->index(i, 0, {});
            emit dataChanged(qmi, qmi);
         }
         recurse(*child);
      }
      for (auto* child : list_set.packed) {
         if (child->inheritance.root_class == root)
            continue;
         if (child->superclass() != &current)
            continue;
         child->inheritance.root_class = root;
         {
            auto i   = this->_data.index_of(child->name);
            auto qmi = this->index(i, 0, {});
            emit dataChanged(qmi, qmi);
         }
         recurse(*child);
      }
   }(subject);
}

static_assert(false, "TODO: Use Win32 to watch the compiled script folder: https://learn.microsoft.com/en-us/windows/win32/fileio/obtaining-directory-change-notifications"
                     "      but bear in mind that we have to do that on a worker thread with thread sync, because that API waits/blocks until a change is detected.");
void DKPapyrusModel::_on_loose_file_created(QString scriptname) {
   QString path;
   {
      auto& core = DovahKitCore::get();

      const auto current_game = core.get_current_game();

      std::filesystem::path game_folder;
      core.get_game_path(game_folder, current_game);

      path = QString::fromStdWString(game_folder.c_str()) + "\\Data\\scripts\\" + scriptname + ".pex";
   }
   QFile file(path);
   if (!file.open(QIODevice::OpenModeFlag::ExistingOnly | QIODevice::OpenModeFlag::ReadOnly)) {
      return;
   }

   auto data = file.readAll();
   Script* loaded = _scan_pex(this->_data, scriptname.toStdString(), data.constData(), data.size(), true);

   _update_superclass_of(*loaded);
}
void DKPapyrusModel::_on_loose_file_edited(QString scriptname);
void DKPapyrusModel::_on_loose_file_deleted(QString scriptname) {
   auto index = this->_data.index_of(scriptname);
   if (index == script_collection::index_of_none)
      return;

   auto* script = this->_data.at(index);
   assert(script != nullptr);

   if (script->info.packed.has_value()) {
      if (script->info.loose.has_value()) {
         auto super_loose  = script->info.loose.value().extends.target;
         auto super_packed = script->info.packed.value().extends.target;

         script->info.loose.reset();
         if (super_loose && super_loose != super_packed) {
            //
            // Hypothetically, a loose file could override an archived PEX with one that 
            // has a different superclass, such that the script effectively changes super-
            // class upon deletion of the loose PEX. We should handle this case.
            //
            super_loose->abandon_loose_subclass(*script);
            _update_superclass_of(*script); // among other things, ensure descendants are updated too
         } else {
            //
            // `_update_superclass_of` sends data-changed for us.
            //
            auto qmi = this->index(index, 0, {});
            emit dataChanged(qmi, qmi);
         }
      }
      return;
   }

   script->info.loose.reset();
   //
   // This script wasn't packed in a BSA; it only ever existed as a loose file, 
   // and that file is now deleted. If the script is unreferenced, then discard 
   // it from the model.
   //
   if (script->is_unreferenced()) {
      this->beginRemoveRows({}, index, index);
      this->_data.take(*script);
      delete script;
      this->endRemoveRows();
   } else {
      _update_superclass_of(*script);
   }
}

DKPapyrusModel::Script* DKPapyrusModel::_scan_pex(script_collection& dst, const std::string& filename, const void* src_data, const size_t src_size, bool is_loose) {
   dovah::compiled_papyrus_script pex;
   try {
      pex.read_file(src_data, src_size);
   } catch (const dovah::compiled_papyrus_script::read_exception&) {
      return nullptr;
   }

   auto* src_script = pex.lookup_object(filename);
   if (!src_script)
      return nullptr;

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

   Script* dst_script = nullptr;

   QString name       = QString::fromStdString(src_script->name);
   Script* dst_script = dst.lookup(name);
   if (is_loose) {
      if (!dst_script) {
         dst_script = new Script;
         dst_script->name = name;
         dst.insert(dst_script);
      }
   } else {
      if (dst_script)
         return dst_script;
   }

   auto& info = (is_loose ? dst_script->info.loose : dst_script->info.packed).emplace();
   info.docstring      = QString::fromStdString(src_script->docstring);
   info.extends.name   = QString::fromStdString(src_script->superclass);
   info.extends.target = nullptr;
   //
   info.flags.conditional = (flag_conditional && (src_script->user_flags & flag_conditional));
   info.flags.hidden      = (flag_hidden      && (src_script->user_flags & flag_hidden));
   return dst_script;
}

/*static*/ std::optional<dovah::form_type_t> DKPapyrusModel::type_of_hardcoded_scriptname(QString name) {
   for (const auto& mapping : all_form_type_scriptnames)
      if (_compare_name(name, mapping.scriptname) == 0)
         return mapping.form_type;
   return {};
}

void DKPapyrusModel::populate_initial() {
   auto& core   = DovahKitCore::get();
   auto& assets = dovahkit::subsystems::assets::get();

   const auto* bsa_order    = assets.get_bsa_load_order();
   const auto  current_game = core.get_current_game();

   this->beginResetModel();

   this->_data.clear();

   std::filesystem::path game_folder;
   core.get_game_path(game_folder, current_game);

   {  // PEX files i.e. compiled scripts
      _for_bsa_files_with_ext(
         bsa_order,
         "scripts",
         "pex",
         [this](const std::string& filename_sans_ext, const dovah::bsa_archived_file& archived_file) {
            _scan_pex(this->_data, filename_sans_ext, archived_file.data(), archived_file.size(), false);
         }
      );
      _for_loose_files_with_ext(
         QString::fromStdWString(game_folder.c_str()) + "\\Data\\scripts\\",
         "pex",
         [this](const std::string& filename_sans_ext, QFile file) {
            auto data = file.readAll();
            _scan_pex(this->_data, filename_sans_ext, data.constData(), data.size(), true);
         }
      );
   }
   //
   // Now that all scripts have been loaded and are known to the model, it's possible 
   // to associate subclasses with their superclasses.
   //
   QVector<Script*> phantoms; // See comments in lambda below.
   //
   for (auto* script : this->_data.list()) {
      if (!script->exists())
         continue;

      auto _process_info = [this, script, &phantoms](std::optional<Script::Info>& info_opt) -> void {
         if (!info_opt.has_value())
            return;
         auto& info = info_opt.value();
         auto  name = info.extends.name;
         if (name.isEmpty())
            return;

         auto* superclass = info.extends.target = this->_data.lookup(name);
         if (superclass) {
            superclass->receive_subclass(*script, &info_opt == &script->info.loose);
            return;
         }

         for (const auto& mapping : all_form_type_scriptnames) {
            if (_compare_name(name, mapping.scriptname) == 0) {
               info.extends.underlying_type = mapping.form_type;
               break;
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
            superclass = new Script;
            superclass->name = name;
            phantoms.push_back(superclass);
         }
         superclass->receive_subclass(*script, &info_opt == &script->info.loose);
      };

      _process_info(script->info.packed);
      _process_info(script->info.loose);
   }
   for (auto* phantom : phantoms) {
      this->_data.insert(phantom);
   }
   //
   // And in turn, now that every script knows its superclass, we can ensure that every script 
   // also knows its root class i.e. the user-defined class at the very top of the inheritance 
   // hierarchy. This will allow us to more quickly query what native class (i.e. form or alias 
   // type), if any, a given script derives from.
   //
   for (auto* script : this->_data.list()) {
      script->_compute_root_class();
   }

   this->endResetModel();
}

bool DKPapyrusModel::script_is_of_type(QString subject, dovah::form_type_t desired) const {
   const auto* script = this->_data.lookup(subject);
   if (!script)
      return false;
   return script->is_of_type(desired);
}
bool DKPapyrusModel::script_is_of_type(QString subject, QString desired) const {
   for (const auto& mapping : all_form_type_scriptnames)
      if (_compare_name(subject, mapping.scriptname) == 0)
         return script_is_of_type(subject, mapping.form_type);

   const auto* script = this->_data.lookup(subject);
   if (!script)
      return false;
   return script->is_of_type(desired);
}

#pragma region QAbstractItemModel overrides
   QModelIndex DKPapyrusModel::index(int row, int column, const QModelIndex& parent) const {
      if (!this->hasIndex(row, column, parent))
         return {};
      auto* entry = this->_data.at(row);
      if (entry)
         return this->createIndex(row, column, (void*)entry);
      return {};
   }
   QModelIndex DKPapyrusModel::parent(const QModelIndex& index) const {
      return {};
   }
   int DKPapyrusModel::rowCount(const QModelIndex& parent) const {
      return this->_data.size();
   }
   int DKPapyrusModel::columnCount(const QModelIndex& item) const {
      return 1;
   }
   
   QVariant DKPapyrusModel::data(const QModelIndex& index, int role) const {
      if (!index.isValid() || index.model() != this)
         return {};
      auto item = (const Script*)index.internalPointer();
      switch (role) {
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            return item->name;
      }
      return {};
   }
   Qt::ItemFlags DKPapyrusModel::flags(const QModelIndex& index) const {
      if (!index.isValid())
         return {};
      return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
   }
#pragma endregion
#pragma endregion