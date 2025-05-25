#include "./form_info_cache.h"
#include <bitset>
#include <cassert>
#include <type_traits>
#include "helpers/class_array.h"
#include "helpers/enum_flags.h"
#include "dovah/form_stub.h"
#include "editor/core.h" // DovahKitCore
#include "./cache_containers/entire_cache.h"

#include "dovah/data/dialogue/topic_subtype.h" // for SharedInfo topics
#include "dovah/forms/components/papyrus.h"
#include "dovah/forms/_all.h" // dovah::all_loaded_form_types + access to relevant loaded-form classes
#include "dovah/forms/Quest.h" // quest alias update logic

#include "editor/subsystems/papyrus/core.h"

#include "dovah/load_order_interfaces/form_load.h"
#include "./cacheable_traits/attached_scripts.h"
#include "./cacheable_traits/model_path.h"
#include "./cacheable_traits/quest_filter.h"
#include "./quest_vmad_skimmer.h"

#include "./threaded_builder.h"

// for benchmarks:
#include "helpers/performance.h"

namespace {
   using known_script_ptr = dovahkit::subsystems::papyrus::known_script_ptr;

   constexpr const auto& sharedinfo_topic_subtype = *dovah::dialogue::topic_subtype_by_signature('IDAT');

   using all_cacheable_form_data = cobb::class_array<
      dovahkit::subsystems::form_info_cache::cached_data::by_form::actor_base,
      dovahkit::subsystems::form_info_cache::cached_data::by_form::faction,
      dovahkit::subsystems::form_info_cache::cached_data::by_form::head_part,
      dovahkit::subsystems::form_info_cache::cached_data::by_form::magic_effect,
      dovahkit::subsystems::form_info_cache::cached_data::by_form::voicetype
   >;
}

namespace {
   using all_form_classes_of_interest = dovah::all_loaded_form_types::filter_types<[]<typename Current>() -> bool {
      {
         bool is_of_interest = all_cacheable_form_data::for_each_until_true([]<typename CurrentDataType>() -> bool {
            if constexpr (CurrentDataType::template form_class_is_of_interest<Current>) {
               return true;
            } else {
               return false;
            }
         });
         if (is_of_interest)
            return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::attached_scripts::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::model_path::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::quest_filter::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (Current::form_type == dovah::form_type::topic) { // for the sharedinfo list
         return true;
      }
      return false;
   }>;
}

namespace {
   using data_with_multiple_subrecords = all_cacheable_form_data::filter_types<[]<typename Data>() -> bool {
      return Data::subrecords_of_interest.size() > 1;
   }>;

   template<dovah::form_type FormType>
   constexpr bool data_with_multiple_subrecords_includes() {
      return data_with_multiple_subrecords::for_each_until_true<[]<typename T>() {
         return T::form_type_is_of_interest(FormType);
      }>();
   }

   template<dovah::form_type FormType>
   void _skim_record(
      dovahkit::subsystems::form_info_cache::entire_cache& cache,
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

      data_with_multiple_subrecords::as_tuple infos;

      // Quests require special handling, for their aliases' attached scripts.
      std::conditional_t<
         (FormType == dovah::form_type::quest),
         quest_vmad_skimmer,
         uint8_t // dummy type
      > quest_skimmer;

      std::conditional_t<
         (FormType == dovah::form_type::topic),
         bool,
         uint8_t // dummy type
      > topic_is_sharedinfo_topic = {};

      while (auto& subrecord = record.next_subrecord()) {
         const auto signature = subrecord.signature();

         if constexpr (data_with_multiple_subrecords_includes<FormType>()) {
            data_with_multiple_subrecords::for_each([&infos, &subrecord]<typename T>() {
               if constexpr (T::form_type_is_of_interest(FormType)) {
                  std::get<T>(infos).skim_subrecord(subrecord);
               }
            });
         }

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
                  cached_data::attached_scripts info(subrecord);
                  if (!info.empty())
                     cache.attached_scripts.threaded_insert(stub, std::move(info));
                  continue;
               }
            }
         }
         if constexpr (cacheable_traits::model_path::form_type_is_of_interest(FormType)) {
            //
            // Models: We only care about the MODL subrecord.
            //
            if (signature == 'MODL') {
               std::string raw;
               subrecord.read(raw);
               cache.model_paths.threaded_insert(stub, QString::fromStdString(raw));
               continue;
            }
         }
         if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(FormType)) {
            //
            // Quest filters: We only care about the FLTR subrecord.
            //
            if (signature == 'FLTR') {
               std::string raw;
               subrecord.read(raw);
               cache.quest_filters.threaded_insert(stub, QString::fromStdString(raw));
               continue;
            }
         }
         if constexpr (cached_data::by_form::faction::form_type_is_of_interest(FormType)) {
            //
            // Factions: For now, we only care about the DATA subrecord.
            //
            if (signature == 'DATA') {
               cached_data::by_form::faction info;
               info.skim_subrecord(subrecord);
               cache.by_form_type.factions.threaded_insert(stub, info);
            }
         } else if constexpr (cached_data::by_form::magic_effect::form_type_is_of_interest(FormType)) {
            //
            // Voicetypes: We only care about the DNAM subrecord.
            //
            if (signature == 'DNAM') {
               cached_data::by_form::magic_effect info;
               info.skim_subrecord(subrecord);
               cache.by_form_type.magic_effects.threaded_insert(stub, info);
            }
         } else if constexpr (cached_data::by_form::voicetype::form_type_is_of_interest(FormType)) {
            //
            // Voicetypes: We only care about the DNAM subrecord.
            //
            if (signature == 'DNAM') {
               cached_data::by_form::voicetype info;
               info.skim_subrecord(subrecord);
               cache.by_form_type.voicetypes.threaded_insert(stub, info);
            }
         } else if constexpr (FormType == dovah::form_type::topic) {
            using loaded_form_type = dovah::loaded_forms::Topic;
            //
            // SharedInfo topics
            //
            {
               constexpr const auto&  desired_subtype       = sharedinfo_topic_subtype;
               constexpr const size_t desired_subtype_index = dovah::dialogue::topic_subtype_index(desired_subtype);

               if (signature == 'SNAM') {
                  uint32_t subtype;
                  if (subrecord.read_signature(subtype)) {
                     topic_is_sharedinfo_topic = subtype == desired_subtype.signature;
                  }
               } else if (signature == 'DATA') {
                  subrecord.skip_bytes(sizeof(decltype(loaded_form_type::data)::flags));
                  subrecord.skip_bytes(sizeof(decltype(loaded_form_type::data)::category));
                  loaded_form_type::subtype_index index;
                  if (subrecord.read(index)) {
                     topic_is_sharedinfo_topic = index == desired_subtype_index;
                  }
               }
            }
         }
      }
      //
      // All subrecords have now been read.
      //
      if constexpr (FormType == dovah::form_type::quest) {
         if (!quest_skimmer.empty())
            cache.attached_scripts.threaded_insert(stub, quest_skimmer.bake());
      } else if constexpr (FormType == dovah::form_type::actor_base) {
         auto& dst        = cache.by_form_type.actor_bases;
         using value_type = std::decay_t<decltype(dst)>::value_type;
         dst.threaded_insert(stub, std::move(std::get<value_type>(infos)));
      } else if constexpr (FormType == dovah::form_type::head_part) {
         auto& dst        = cache.by_form_type.head_parts;
         using value_type = std::decay_t<decltype(dst)>::value_type;
         dst.threaded_insert(stub, std::move(std::get<value_type>(infos)));
      } else if constexpr (FormType == dovah::form_type::topic) {
         if (topic_is_sharedinfo_topic)
            cache.sharedinfo_topics.threaded_insert(stub);
      }
   }

   template<typename LoadedForm>
   void _update_form(
      dovahkit::subsystems::form_info_cache::core& core,
      dovahkit::subsystems::form_info_cache::entire_cache& cache,
      LoadedForm& loaded
   ) {
      using namespace dovahkit::subsystems::form_info_cache;

      dovah::form_stub& stub = loaded.stub;

      if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(LoadedForm::form_type)) {
         auto& dst = cache.quest_filters;

         auto value = QString::fromStdString(loaded.filter);

         QString prior;
         bool    changed;
         {
            std::optional<QString> result;
            if (value.isEmpty())
               result = dst.take(stub);
            else
               result = dst.take_and_replace(stub, value);
            
            if (changed = result.has_value())
               prior = result.value();
         }
         if (changed)
            emit core.cachedQuestFilterChanged(stub, prior, value);
      }

      {
         using signal_type = decltype(&dovahkit::subsystems::form_info_cache::core::cachedActorBaseChanged);
         auto _update_if_form = [&loaded, &core, &stub]<typename T>(
            data_cache<T>& dst,
            signal_type    signal
         ) {
            if constexpr (T::form_type_is_of_interest(LoadedForm::form_type)) {
               if (auto* item = dst.get(stub)) {
                  if (item->update(loaded)) {
                     emit (core.*signal)(stub);
                  }
               } else {
                  T info;
                  info.update(loaded);
                  dst.insert(stub, std::move(info));
                  emit(core.*signal)(stub);
               }
            }
         };

         _update_if_form(cache.by_form_type.actor_bases,   &core::cachedActorBaseChanged);
         _update_if_form(cache.by_form_type.factions,      &core::cachedFactionChanged);
         _update_if_form(cache.by_form_type.head_parts,    &core::cachedHeadPartChanged);
         _update_if_form(cache.by_form_type.magic_effects, &core::cachedMagicEffectChanged);
         _update_if_form(cache.by_form_type.voicetypes,    &core::cachedVoicetypeChanged);
      }

      if constexpr (cacheable_traits::model_path::form_type_is_of_interest(LoadedForm::form_type)) {
         auto& dst  = cache.model_paths;
         auto  path = QString::fromStdString(loaded.model.model_path);

         QString prior;
         bool    changed;
         {
            std::optional<QString> result;
            if (path.isEmpty())
               result = dst.take(stub);
            else
               result = dst.take_and_replace(stub, path);
            
            if (changed = result.has_value())
               prior = result.value();
         }
         if (changed)
            emit core.cachedModelPathChanged(stub, prior, path);
      }

      if constexpr (cacheable_traits::attached_scripts::form_class_is_of_interest<LoadedForm>) {
         auto& dst = cache.attached_scripts;
         auto& src = loaded.script_data;

         cached_data::attached_scripts new_info(src);
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
            changed = dst.erase(stub);
         } else {
            changed = dst.replace(stub, new_info);
         }
         if (changed)
            emit core.cachedScriptsChanged(stub);
      }

      if constexpr (LoadedForm::form_type == dovah::form_type::topic) {
         auto& dst = cache.sharedinfo_topics;
         
         bool is_sharedinfo_topic = loaded.subtype == sharedinfo_topic_subtype.signature;
         bool changed = false;
         if (is_sharedinfo_topic) {
            changed = dst.insert(stub);
         } else {
            changed = dst.erase(stub);
         }
         if (changed)
            emit core.cachedSharedInfoTopicChanged(stub, is_sharedinfo_topic);
      }
   }
}

namespace dovahkit::subsystems::form_info_cache {
   core::core() {
      this->_cache = new entire_cache;
      assert(this->_cache != nullptr && "FIC cache allocation must not fail.");

      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &core::clear);
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &core::buildAllData);

      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         all_form_classes_of_interest::for_each_until_true([this, stub]<typename Current>() -> bool {
            if (stub->form_type == Current::form_type) {
               auto loaded = stub->load().ptr_cast<Current>();
               assert(loaded);
               _update_form<Current>(*this, *this->_cache, *loaded);
               return true;
            }
            return false;
         });
      });

      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool will_be_flagged) {
         all_form_classes_of_interest::for_each_until_true([this, stub]<typename Current>() -> bool {
            if (stub->form_type != Current::form_type)
               return false;

            auto& cache = *this->_cache;

            if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(Current::form_type)) {
               auto result = cache.quest_filters.take(*stub);
               if (result.has_value()) {
                  emit this->cachedQuestFilterChanged(*stub, result.value(), {});
               }
            }
            
            {
               using change_signal_type = decltype(&dovahkit::subsystems::form_info_cache::core::cachedActorBaseChanged);
               auto _update_if_form = [this, stub]<typename T>(
                  data_cache<T>&     dst,
                  change_signal_type signal = nullptr
               ) {
                  if constexpr (T::form_type_is_of_interest(Current::form_type)) {
                     dst.take(*stub);
                  }
                  if constexpr (T::form_type_is_referred_to(Current::form_type)) {
                     dst.for_each([this, signal](const dovah::form_stub& stub, T& info) {
                        if (info.sever_outbound_references_to(&stub)) {
                           emit (this->*signal)(*const_cast<dovah::form_stub*>(&stub));
                        }
                     });
                     dst.take(*stub);
                  }
               };
               _update_if_form(cache.by_form_type.actor_bases, &core::cachedActorBaseChanged);
               _update_if_form(cache.by_form_type.factions);
               _update_if_form(cache.by_form_type.head_parts,  &core::cachedHeadPartChanged);
               _update_if_form(cache.by_form_type.magic_effects);
               _update_if_form(cache.by_form_type.voicetypes);
            }

            if constexpr (cacheable_traits::model_path::form_type_is_of_interest(Current::form_type)) {
               auto result = cache.model_paths.take(*stub);
               if (result.has_value()) {
                  emit this->cachedModelPathChanged(*stub, result.value(), {});
               }
            }
            if constexpr (cacheable_traits::attached_scripts::form_class_is_of_interest<Current>) {
               if (cache.attached_scripts.erase(*stub)) {
                  emit this->cachedScriptsChanged(*stub);
               }
            }

            if constexpr (Current::form_type == dovah::form_type::topic) {
               cache.sharedinfo_topics.erase(*stub);
            }

            return true;
         });
      });
   }
   core::~core() {
      if (auto*& p = this->_cache) {
         delete p;
         p = nullptr;
      }
   }

   template<dovah::form_type FormType>
   void core::_skim_record(dovah::form_stub& stub, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc) {
      if (!intfc.is_winning_record)
         return;

      ::_skim_record<FormType>(*this->_cache, stub, record);
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
      this->_cache->model_paths.reserve(counts.models / 1.5);
      this->_cache->quest_filters.reserve(counts.quests / 1.5);

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
      (*this->_cache) = {};
      emit this->cachedDataCleared();
   }

   //

   QString core::get_form_model_path(const dovah::form_stub& stub) const {
      if (auto* item = this->_cache->model_paths.get(stub))
         return *item;
      return {};
   }
   QString core::get_quest_filter(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::quest)
         return {};
      if (auto* item = this->_cache->quest_filters.get(stub))
         return *item;
      return {};
   }
   const cached_data::by_form::actor_base* core::get_actor_base_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::actor_base)
         return nullptr;
      return this->_cache->by_form_type.actor_bases.get(stub);
   }
   const cached_data::by_form::faction* core::get_faction_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::faction)
         return nullptr;
      return this->_cache->by_form_type.factions.get(stub);
   }
   const cached_data::by_form::head_part* core::get_head_part_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::head_part)
         return nullptr;
      return this->_cache->by_form_type.head_parts.get(stub);
   }
   const cached_data::by_form::magic_effect* core::get_magic_effect_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::magic_effect)
         return nullptr;
      return this->_cache->by_form_type.magic_effects.get(stub);
   }
   const cached_data::by_form::voicetype* core::get_voicetype_info(const dovah::form_stub& stub) const {
      if (stub.form_type != dovah::form_type::voicetype)
         return nullptr;
      return this->_cache->by_form_type.voicetypes.get(stub);
   }

   script_attach_state core::form_script_attachment(const dovah::form_stub& stub, std::string_view scriptname) const {
      auto* item = this->_cache->attached_scripts.get(stub);
      if (!item)
         return script_attach_state::not_present;

      for (auto& known : item->deleted)
         if (known->name_matches(scriptname))
            return script_attach_state::removed;

      for (auto& known : item->attached)
         if (known->name_matches(scriptname))
            return script_attach_state::attached;

      return script_attach_state::not_present;
   }
   bool core::quest_has_alias_with_script(const dovah::form_stub& stub, std::string_view scriptname) const {
      auto* item = this->_cache->attached_scripts.get(stub);
      if (!item)
         return false;

      for (auto& known : item->aliases)
         if (known->name_matches(scriptname))
            return true;

      return false;
   }

   std::vector<const subsystems::papyrus::known_script*> core::get_scripts_attached_to_form(const dovah::form_stub& stub) const {
      auto* item = this->_cache->attached_scripts.get(stub);
      if (!item)
         return {};
      
      std::vector<const subsystems::papyrus::known_script*> out;
      for (auto& known : item->attached) {
         bool is_removed = false;
         for (auto& removed : item->deleted) {
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
      auto* item = this->_cache->attached_scripts.get(quest);
      if (!item)
         return {};
      
      std::vector<const subsystems::papyrus::known_script*> out;
      for (auto& known : item->aliases) {
         out.push_back(known.get());
      }
      return out;
   }

   bool core::topic_is_sharedinfo_topic(const dovah::form_stub& topic) const {
      return this->_cache->sharedinfo_topics.contains(topic);
   }
   
   void core::for_all_form_model_paths(std::function<void(const dovah::form_stub&, QString)> functor) {
      this->_cache->model_paths.for_each(functor);
   }
   void core::for_all_quest_filters(std::function<void(QString)> functor) {
      this->_cache->quest_filters.for_each([&functor](const dovah::form_stub&, QString v) {
         (functor)(v);
      });
   }
   void core::for_all_head_parts(std::function<void(const cached_data::by_form::head_part&)> functor) {
      this->_cache->by_form_type.head_parts.for_each([&functor](const dovah::form_stub&, const auto& info) {
         (functor)(info);
      });
   }
}