#include "./form_info_cache.h"
#include <bitset>
#include <cassert>
#include <type_traits>
#include "helpers/enum_flags.h"
#include "editor/core.h" // DovahKitCore

#include "dovah/form_stub.h"

#include "dovah/forms/components/papyrus.h"
#include "dovah/forms/_all.h" // dovah::all_loaded_form_types + access to relevant loaded-form classes

#include "editor/subsystems/papyrus/core.h"

#include "./cacheable_traits/attached_scripts.h"
#include "./cacheable_traits/model_path.h"
#include "./cacheable_traits/quest_filter.h"
#include "./cacheable_trait.h"

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
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::attached_scripts::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::model_path::form_class_is_of_interest<Current>) {
         return true;
      }
      if constexpr (dovahkit::subsystems::form_info_cache::cacheable_traits::quest_filter::form_class_is_of_interest<Current>) {
         return true;
      }
      return false;
   }>;
}

namespace {
   template<dovah::form_type::type FormType>
   void _skim_record(
      dovahkit::subsystems::form_info_cache::cache_map_collection& cache,
      dovah::form_stub& stub,
      dovah::tes_file_reading::record& record
   ) {
      using namespace dovahkit::subsystems::form_info_cache;

      using seen_trait_mask = cobb::enum_flags<cacheable_trait, cacheable_trait_count>;
      seen_trait_mask seen;

      if constexpr (!cacheable_traits::attached_scripts::form_type_is_of_interest(FormType)) {
         seen |= cacheable_trait::attached_scripts;
      }
      if constexpr (!cacheable_traits::model_path::form_type_is_of_interest(FormType)) {
         seen |= cacheable_trait::model_path;
      }
      if constexpr (!cacheable_traits::quest_filter::form_type_is_of_interest(FormType)) {
         seen |= cacheable_trait::quest_filter;
      }

      while (auto& subrecord = record.next_subrecord()) {
         if constexpr (cacheable_traits::attached_scripts::form_type_is_of_interest(FormType)) {
            if (subrecord.signature() == 'VMAD') {
               cached_vmad_info info(subrecord);
               if (!info.empty()) {
                  cache.attached_scripts.threadedInsert(stub, std::move(info));
               }
               //
               seen |= cacheable_trait::attached_scripts;
            }
         }
         if constexpr (cacheable_traits::model_path::form_type_is_of_interest(FormType)) {
            if (subrecord.signature() == 'MODL') {
               std::string raw;
               subrecord.read(raw);
               cache.model_paths.threadedInsert(stub, QString::fromStdString(raw));
               //
               seen |= cacheable_trait::model_path;
            }
         }
         if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(FormType)) {
            if (subrecord.signature() == 'FLTR') {
               std::string raw;
               subrecord.read(raw);
               cache.quest_filters.threadedInsert(stub, QString::fromStdString(raw));
               //
               seen |= cacheable_trait::quest_filter;
            }
         }

         if (seen == seen_trait_mask::with_all_set())
            break;
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
            if (stub->formType == Current::form_type) {
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
            if (stub->formType != Current::form_type)
               return false;

            if constexpr (cacheable_traits::quest_filter::form_type_is_of_interest(Current::form_type)) {
               auto result = this->_cache.quest_filters.takeAndReport(*stub);
               if (result.has_value()) {
                  emit this->cachedQuestFilterChanged(*stub, result.value(), {});
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
   

   template<dovah::form_type::type FormType>
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
         static constexpr const auto form_type = (dovah::form_type::type)Current::form_type;

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
      this->_cache.clear();
      emit this->cachedDataCleared();
   }

   //

   QString core::get_form_model_path(const dovah::form_stub& stub) const {
      return this->_cache.model_paths.value(&stub);
   }
   QString core::get_quest_filter(const dovah::form_stub& stub) const {
      if (stub.formType != dovah::form_type::quest)
         return {};
      return this->_cache.quest_filters.value(&stub);
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
}