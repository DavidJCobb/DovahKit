#include "./form_info_cache.h"
#include <bitset>
#include <cassert>
#include <type_traits>
#include "helpers/enum_flags.h"
#include "editor/core.h" // DovahKitCore

#include "dovah/form_stub.h"

#include "dovah/forms/components/papyrus.h"
#include "dovah/forms/_all.h" // dovah::all_loaded_form_types + access to relevant loaded-form classes
#include "dovah/forms/Quest.h" // quest alias update logic

#include "editor/subsystems/papyrus/core.h"

#include "dovah/load_order_interfaces/form_load.h"
#include "./cacheable_traits/attached_scripts.h"
#include "./cacheable_traits/faction_info.h"
#include "./cacheable_traits/head_part_info.h"
#include "./cacheable_traits/model_path.h"
#include "./cacheable_traits/quest_filter.h"
#include "./cacheable_traits/voicetype_info.h"
#include "./quest_vmad_skimmer.h"

#include "./threaded_builder.h"

// for benchmarks:
#include "helpers/performance.h"

namespace {
   using known_script_ptr = dovahkit::subsystems::papyrus::known_script_ptr;

   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

namespace {
   using all_form_classes_of_interest = dovah::all_loaded_form_types::filter_types<[]<typename Current>() -> bool {
      //
      // TODO: We could look into ways to go harder into metaprogramming here. For example, I 
      // chose to use structs for each trait rather than namespaces so that they could be put 
      // in a cobb::class_array and iterated over. Would still have to figure out what approach 
      // to take for the actual "skim," "update," etc., functions, as well as for accessors.
      // 
      // For now, handwritten branches work just fine.
      //
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::attached_scripts::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::faction_info::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::head_part_info::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::model_path::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::quest_filter::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::voicetype_info::form_class_is_of_interest<Current>) {
         return true;
      }
      return false;
   }>;
}

namespace {
   template<dovah::form_type FormType>
   void _skim_record(
      dovahkit::subsystems::form_info_cache::cache_map_collection& cache,
      dovah::form_stub& stub,
      dovah::tes_file_reading::record& record
   ) {
      using namespace dovahkit::subsystems::form_info_cache;

      //
      // The `seen` mask is used to exit the subrecord-reading loop early. The idea is that 
      // when all bits are set, we know we've seen all data of interest. (Above, we set the 
      // bits for data that is not of interest, i.e. cached-info types that we know the current 
      // form type will never have.)
      //
      // ---------------------------------------------------------------------------------------
      //
      // The branches below (in the subrecord loop, for each cached-info type) generally fall 
      // into two categories:
      // 
      //  - Situations where we definitely only care about one subrecord for a given type of 
      //    cached info.
      // 
      //  - Situations where we (may in the future) want to read multiple subrecords for a 
      //    given type of cached info.
      // 
      // In the former case, we can just instantiate the cached info and threaded-insert it 
      // within that specific branch. In the latter case, we'll want to have a variable scoped 
      // to outside of the branch where we handle a subrecord, and only threaded-insert the info 
      // after the subrecord-reading loop.
      // 

      // Quests require special handling, for their aliases' attached scripts.
      std::conditional_t<
         (FormType == dovah::form_type::quest),
         quest_vmad_skimmer,
         uint8_t // dummy type
      > quest_skimmer;

      std::conditional_t<
         (FormType == dovah::form_type::head_part),
         cached_head_part_info,
         uint8_t // dummy type
      > head_part_info = {};

      while (auto& subrecord = record.next_subrecord()) {
         const auto signature = subrecord.signature();

         if constexpr (cacheable_traits::attached_scripts::form_type_is_of_interest(FormType)) {
            //
            // Papyrus: for most forms, we only care about the VMAD subrecord, but for quests, we 
            // need to read additional subrecords as well in order to properly handle scripts that 
            // are attached to aliases.
            //
            if constexpr (FormType == dovah::form_type::quest) {
               //
               // Quests require special handling, for their aliases' attached scripts.
               //
               bool consumed = quest_skimmer.skim_subrecord(stub, subrecord);
               if (consumed) {
                  continue;
               }
            } else {
               if (signature == 'VMAD') {
                  cached_vmad_info info(subrecord);
                  if (!info.empty())
                     cache.attached_scripts.threadedInsert(stub, std::move(info));
               }
            }
         }
         if constexpr (cacheable_traits::faction_info::form_type_is_of_interest(FormType)) {
            //
            // Factions: For now, we only care about the DATA subrecord.
            //
            if (signature == 'DATA') {
               cached_faction_info info;
               info.skim_subrecord(subrecord);
               cache.factions.threadedInsert(stub, info);
            }
         }
         if constexpr (cacheable_traits::head_part_info::form_type_is_of_interest(FormType)) {
            //
            // HeadParts: We need to read multiple subrecords.
            //
            head_part_info.skim_subrecord(subrecord);
         }
         if constexpr (cacheable_traits::model_path::form_type_is_of_interest(FormType)) {
            //
            // Models: We only care about the MODL subrecord.
            //
            if (signature == 'MODL') {
               std::string raw;
               subrecord.read(raw);
               cache.model_paths.threadedInsert(stub, QString::fromStdString(raw));
            }
         }
         if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(FormType)) {
            //
            // Quest filters: We only care about the FLTR subrecord.
            //
            if (signature == 'FLTR') {
               std::string raw;
               subrecord.read(raw);
               cache.quest_filters.threadedInsert(stub, QString::fromStdString(raw));
            }
         }
         if constexpr (cacheable_traits::voicetype_info::form_type_is_of_interest(FormType)) {
            //
            // Voicetypes: We only care about the DNAM subrecord.
            //
            if (signature == 'DNAM') {
               cached_voicetype_info info;
               info.skim_subrecord(subrecord);
               cache.voicetypes.threadedInsert(stub, info);
            }
         }
      }

      if constexpr (FormType == dovah::form_type::quest) {
         if (!quest_skimmer.empty())
            cache.attached_scripts.threadedInsert(stub, quest_skimmer.bake());
      }
      if constexpr (FormType == dovah::form_type::head_part) {
         cache.head_parts.threadedInsert(stub, std::move(head_part_info));
      }
   }

   template<typename LoadedForm>
   void _update_form(
      dovahkit::subsystems::form_info_cache::core& core,
      dovahkit::subsystems::form_info_cache::cache_map_collection& cache,
      LoadedForm& loaded
   ) {
      using namespace dovahkit::subsystems::form_info_cache;

      auto& stub = loaded.stub;

      if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(LoadedForm::form_type)) {
         auto& dst = cache.quest_filters;

         auto value = QString::fromStdString(loaded.filter);

         QString prior;
         bool    changed;
         if (value.isEmpty()) {
            auto result = dst.takeAndReport(stub);
            changed = result.has_value();
            if (changed)
               prior = result.value();
         } else {
            changed = dst.replaceTakeAndReport(stub, value, prior);
         }
         if (changed)
            emit core.cachedQuestFilterChanged(stub, prior, value);
      }
      
      if constexpr (cacheable_traits::faction_info::form_type_is_of_interest(LoadedForm::form_type)) {
         auto& dst_list = cache.factions;
         auto  dst_it   = dst_list.find(&stub);
         if (dst_it != dst_list.end()) {
            auto& dst = *dst_it;
            if (dst.update(loaded)) {
               emit core.cachedFactionChanged(stub);
            }
         } else {
            cached_faction_info info;
            info.update(loaded);
            dst_list.insert(&stub, std::move(info));
            emit core.cachedFactionChanged(stub);
         }
      }

      if constexpr (cacheable_traits::head_part_info::form_type_is_of_interest(LoadedForm::form_type)) {
         auto& dst_list = cache.head_parts;
         auto  dst_it   = dst_list.find(&stub);
         if (dst_it != dst_list.end()) {
            auto& dst = *dst_it;
            if (dst.update(loaded)) {
               emit core.cachedHeadPartChanged(stub);
            }
         } else {
            cached_head_part_info info;
            info.update(loaded);
            dst_list.insert(&stub, std::move(info));
            emit core.cachedHeadPartChanged(stub);
         }
      }

      if constexpr (cacheable_traits::voicetype_info::form_type_is_of_interest(LoadedForm::form_type)) {
         auto& dst_list = cache.voicetypes;
         auto  dst_it   = dst_list.find(&stub);
         if (dst_it != dst_list.end()) {
            auto& dst = *dst_it;
            if (dst.update(loaded)) {
               emit core.cachedVoicetypeChanged(stub);
            }
         } else {
            cached_voicetype_info info;
            info.update(loaded);
            dst_list.insert(&stub, std::move(info));
            emit core.cachedVoicetypeChanged(stub);
         }
      }

      if constexpr (cacheable_traits::model_path::form_type_is_of_interest(LoadedForm::form_type)) {
         auto& dst = cache.model_paths;

         auto path = QString::fromStdString(loaded.model.model_path);

         QString prior;
         bool    changed;
         if (path.isEmpty()) {
            auto result = dst.takeAndReport(stub);
            changed = result.has_value();
            if (changed)
               prior = result.value();
         } else {
            changed = dst.replaceTakeAndReport(stub, path, prior);
         }
         if (changed)
            emit core.cachedModelPathChanged(stub, prior, path);
      }

      if constexpr (cacheable_traits::attached_scripts::form_class_is_of_interest<LoadedForm>) {
         auto& dst = cache.attached_scripts;
         auto& src = loaded.script_data;

         cached_vmad_info new_info(src);
         if constexpr (std::is_same_v<LoadedForm, dovah::loaded_forms::Quest>) {
            //
            // Quest aliases require special handling.
            //
            auto& papyrus = dovahkit::subsystems::papyrus::core::get();
            //
            for (auto* alias : loaded.aliases) {
               if (!alias)
                  continue;

               std::vector<std::string_view> attached;
               std::vector<std::string_view> removed;
               for (auto& script : alias->script_data.scripts) {
                  if (script.status == dovah::loaded_forms::components::papyrus::script_status::removed) {
                     removed.push_back(script.name);
                  } else {
                     attached.push_back(script.name);
                  }
               }
               for (auto& name : attached) {
                  bool retained = true;
                  for (auto& rmv : removed) {
                     if (dovah::papyrus::helpers::name_equals(name, rmv)) {
                        retained = false;
                        break;
                     }
                  }
                  if (!retained)
                     continue;

                  new_info.aliases.push_back(
                     papyrus.know_script_via_vmad_scan(name)
                  );
               }
            }
         }
         
         bool changed;
         if (new_info.empty()) {
            changed = dst.eraseAndReport(stub);
         } else {
            changed = dst.replaceAndReport(stub, new_info);
         }
         if (changed)
            emit core.cachedScriptsChanged(stub);
      }
   }
}

namespace dovahkit::subsystems::form_info_cache {
   core::core() {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &core::clear);
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &core::buildAllData);

      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         all_form_classes_of_interest::for_each_until_true([this, stub]<typename Current>() -> bool {
            if (stub->form_type == Current::form_type) {
               auto loaded = stub->load().ptr_cast<Current>();
               assert(loaded);
               _update_form<Current>(*this, this->_cache, *loaded);
               return true;
            }
            return false;
         });
      });

      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool will_be_flagged) {
         all_form_classes_of_interest::for_each_until_true([this, stub]<typename Current>() -> bool {
            if (stub->form_type != Current::form_type)
               return false;

            if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(Current::form_type)) {
               auto result = this->_cache.quest_filters.takeAndReport(*stub);
               if (result.has_value()) {
                  emit this->cachedQuestFilterChanged(*stub, result.value(), {});
               }
            }

            if constexpr (cacheable_traits::faction_info::form_type_is_of_interest(Current::form_type)) {
               this->_cache.factions.takeAndReport(*stub);
            }

            if constexpr (cacheable_traits::head_part_info::form_type_is_of_interest(Current::form_type)) {
               this->_cache.head_parts.takeAndReport(*stub);
               //
               // Don't emit a "head part changed" signal for this. HeadParts with blank info 
               // would be mistaken for HeadParts that have no restrictions and are available 
               // on all actors. Instead, clients should also hook formDeletionImminent and 
               // handle loss of the HeadPart on their own.
               //
            }
            if constexpr (cacheable_traits::head_part_info::form_type_is_referred_to(Current::form_type)) {
               auto& map = this->_cache.head_parts;
               for (auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
                  auto& info = it->second;
                  if (info.sever_outbound_references_to(stub)) {
                     emit this->cachedHeadPartChanged(*(dovah::form_stub*)it->first);
                  }
               }
            }

            if constexpr (cacheable_traits::model_path::form_type_is_of_interest(Current::form_type)) {
               auto result = this->_cache.model_paths.takeAndReport(*stub);
               if (result.has_value()) {
                  emit this->cachedModelPathChanged(*stub, result.value(), {});
               }
            }
            if constexpr (cacheable_traits::attached_scripts::form_class_is_of_interest<Current>) {
               if (this->_cache.attached_scripts.eraseAndReport(*stub)) {
                  emit this->cachedScriptsChanged(*stub);
               }
            }

            return true;
         });
      });
   }
   

   template<dovah::form_type FormType>
   void core::_skim_record(dovah::form_stub& stub, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc) {
      if (!intfc.is_winning_record)
         return;

      ::_skim_record<FormType>(this->_cache, stub, record);
   }

   void core::buildAllData() {
      std::array<threaded_builder, 8> builders;
      uint32_t count = 0;
      
      auto& editor = DovahKitCore::get();

      struct {
         size_t all    = 0;
         size_t models = 0;
         size_t quests = 0;
      } counts;

      size_t model_path_count   = 0;
      size_t quest_filter_count = 0;

      auto bench_whole = cobb::benchmark();
      auto bench_prep  = cobb::benchmark();
      bench_whole.begin();
      bench_prep.begin();

      all_form_classes_of_interest::for_each([&builders, &editor, &counts]<typename Current>() {
         static constexpr const auto form_type = Current::form_type;

         editor.for_each_form_of_type(form_type, [&builders, &counts](dovah::form_stub* stub) {
            builders[counts.all % builders.size()].add_to_queue(
               &core::_static_skim_record<form_type>,
               stub
            );
            ++counts.all;
            if constexpr (cacheable_traits::model_path::form_type_is_of_interest(form_type)) {
               ++counts.models;
            }
            if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(form_type)) {
               ++counts.quests;
            }
            return false;
         });
      });
      this->_cache.model_paths.reserve(counts.models / 1.5);
      this->_cache.quest_filters.reserve(counts.quests / 1.5);

      bench_prep.end();
      qDebug("Time to prep form-info-cache scan: %u ms", bench_prep.milliseconds());
      
      for (auto& b : builders)
         b.start();
      for (auto& b : builders)
         b.wait_for();

      bench_whole.end();
      qDebug("Time to complete form-info-cache scan: %u ms", bench_whole.milliseconds());
      
      emit this->cachedDataBuilt();
   }

   void core::clear() {
      qDebug("[dovahkit::subsystems::form_info_cache::core::clear] Clearing all cached form info...");
      this->_cache.clear();
      emit this->cachedDataCleared();
   }

   //

   QString core::get_form_model_path(const dovah::form_stub& stub) const {
      return this->_cache.model_paths.value(&stub);
   }
   QString core::get_quest_filter(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::quest)
         return {};
      return this->_cache.quest_filters.value(&stub);
   }
   const cached_faction_info* core::get_faction_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::faction)
         return nullptr;
      return this->_cache.factions.valuePointer(stub);
   }
   const cached_head_part_info* core::get_head_part_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::head_part)
         return nullptr;
      return this->_cache.head_parts.valuePointer(stub);
   }
   const cached_voicetype_info* core::get_voicetype_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::voicetype)
         return nullptr;
      return this->_cache.voicetypes.valuePointer(stub);
   }

   script_attach_state core::form_script_attachment(const dovah::form_stub& stub, std::string_view scriptname) const {
      auto it = this->_cache.attached_scripts.find(&stub);
      if (it == this->_cache.attached_scripts.end())
         return script_attach_state::not_present;

      for (auto& known : it->deleted)
         if (known->name_matches(scriptname))
            return script_attach_state::removed;

      for (auto& known : it->attached)
         if (known->name_matches(scriptname))
            return script_attach_state::attached;

      return script_attach_state::not_present;
   }
   bool core::quest_has_alias_with_script(const dovah::form_stub& stub, std::string_view scriptname) const {
      auto it = this->_cache.attached_scripts.find(&stub);
      if (it == this->_cache.attached_scripts.end())
         return false;

      for (auto& known : it->aliases)
         if (known->name_matches(scriptname))
            return true;

      return false;
   }

   std::vector<const subsystems::papyrus::known_script*> core::get_scripts_attached_to_form(const dovah::form_stub& stub) const {
      auto it = this->_cache.attached_scripts.find(&stub);
      if (it == this->_cache.attached_scripts.end())
         return {};
      
      std::vector<const subsystems::papyrus::known_script*> out;
      for (auto& known : it->attached) {
         bool is_removed = false;
         for (auto& removed : it->deleted) {
            if (known == removed) {
               is_removed = true;
               break;
            }
         }
         if (is_removed)
            continue;
         out.push_back(known.get());
      }
      return out;
   }
   std::vector<const subsystems::papyrus::known_script*> core::get_scripts_attached_to_quest_aliases(const dovah::form_stub& quest) const {
      auto it = this->_cache.attached_scripts.find(&quest);
      if (it == this->_cache.attached_scripts.end())
         return {};
      
      std::vector<const subsystems::papyrus::known_script*> out;
      for (auto& known : it->aliases) {
         out.push_back(known.get());
      }
      return out;
   }
}