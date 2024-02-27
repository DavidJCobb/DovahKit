#include "./form_info_cache.h"
#include <bitset>
#include <cassert>
#include <type_traits>
#include "helpers/enum_flags.h"
#include "editor/core.h" // DovahKitCore

#include "dovah/form_stub.h"
#include "dovah/forms/components/papyrus.h"

#include "editor/subsystems/papyrus/core.h"

#include "dovah/forms/_all.h"

#include "editor/form_data_cache_internals/threaded_builder.h"

// for benchmarks:
#include "helpers/performance.h"

namespace {
   using known_script_ptr = dovahkit::subsystems::papyrus::known_script_ptr;

   namespace vmad {
      using namespace dovah::loaded_forms::components::papyrus;
   }
}

namespace {
   constexpr const auto cache_model_paths_for_form_types = std::array{
      dovah::form_type::activator,
      dovah::form_type::container,
      dovah::form_type::door,
      dovah::form_type::flora,
      dovah::form_type::furniture,
      dovah::form_type::light,
      dovah::form_type::movable_static,
      dovah::form_type::statik,
      dovah::form_type::tree,
   };

   template<dovah::form_type_t FormType>
   constexpr const bool interested_in_model_path = []() -> bool {
      for (auto ft : cache_model_paths_for_form_types)
         if (ft == (dovah::form_type::type)FormType)
            return true;
      return false;
   }();

   template<typename LoadedForm>
   concept form_data_has_papyrus = requires (LoadedForm& f) {
      { f.script_data } -> std::same_as<dovah::loaded_forms::components::papyrus_attachment_data&>;
   };

   template<dovah::form_type::type FormType>
   constexpr const bool form_type_has_papyrus = []() -> bool {
      bool result = false;
      dovah::all_loaded_form_types::for_each_until_true([&result]<typename Current>() -> bool {
         if constexpr (Current::form_type == FormType) {
            result = true;
            return true;
         }
         return false;
      });
      return result;
   }();




   using all_form_classes_of_interest = dovah::all_loaded_form_types::filter_types<[]<typename Current>() -> bool {
      if constexpr (Current::form_type == dovah::form_type::quest) { // quest filter
         return true;
      }
      if constexpr (form_data_has_papyrus<Current>) {
         return true;
      }
      for (auto ft : cache_model_paths_for_form_types) {
         if ((dovah::form_type::type)Current::form_type == ft)
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

      enum class trait {
         attached_scripts,
         model_path,
         quest_filter,
      };
      using seen_trait_mask = cobb::enum_flags<trait, 3>;
      //
      seen_trait_mask seen;

      if constexpr (!form_type_has_papyrus<FormType>) {
         seen |= trait::attached_scripts;
      }
      if constexpr (!interested_in_model_path<FormType>) {
         seen |= trait::model_path;
      }
      if constexpr (FormType != dovah::form_type::quest) {
         seen |= trait::quest_filter;
      }

      while (auto& subrecord = record.next_subrecord()) {
         if constexpr (!form_type_has_papyrus<FormType>) {
            if (subrecord.signature() == 'VMAD') {
               cached_vmad_info info(subrecord);
               cache.attached_scripts.threadedInsert(stub, std::move(info));
               //
               seen |= trait::attached_scripts;
            }
         }
         if constexpr (interested_in_model_path<FormType>) {
            if (subrecord.signature() == 'MODL') {
               std::string raw;
               subrecord.read(raw);
               cache.model_paths.threadedInsert(stub, QString::fromStdString(raw));
               //
               seen |= trait::model_path;
            }
         }
         if constexpr (FormType == dovah::form_type::quest) {
            if (subrecord.signature() == 'FLTR') {
               std::string raw;
               subrecord.read(raw);
               cache.quest_filters.threadedInsert(stub, QString::fromStdString(raw));
               //
               seen |= trait::quest_filter;
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

      if constexpr (LoadedForm::form_type == dovah::form_type::quest) {
         auto& dst = cache.quest_filters;

         auto value = QString::fromStdString(loaded.filter);

         bool changed;
         if (value.isEmpty()) {
            changed = dst.eraseAndReport(stub);
         } else {
            changed = dst.replaceAndReport(stub, value);
         }
         if (changed)
            emit core.cachedQuestFilterChanged(stub, value);
      }

      if constexpr (interested_in_model_path<LoadedForm::form_type>) {
         auto& dst = cache.model_paths;

         auto path = QString::fromStdString(loaded.model.model_path);

         bool changed;
         if (path.isEmpty()) {
            changed = dst.eraseAndReport(stub);
         } else {
            changed = dst.replaceAndReport(stub, path);
         }
         if (changed)
            emit core.cachedModelPathChanged(stub, path);
      }

      if constexpr (form_data_has_papyrus<LoadedForm>) {
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
         if (stub->formType == dovah::form_type::quest) {
            if (this->_cache.quest_filters.eraseAndReport(*stub)) {
               emit this->cachedQuestFilterChanged(*stub, {});
            }
         }
         all_form_classes_of_interest::for_each_until_true([this, stub]<typename Current>() -> bool {
            if (stub->formType != Current::form_type)
               return false;

            if constexpr (interested_in_model_path<Current::form_type>) {
               if (this->_cache.model_paths.eraseAndReport(*stub)) {
                  emit this->cachedModelPathChanged(*stub, {});
               }
            }
            if constexpr (form_data_has_papyrus<Current>) {
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
      std::array<DovahKitEditorInternals::form_data_cache_builder, 8> builders;
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
            if constexpr (interested_in_model_path<form_type>) {
               ++counts.models;
            }
            if constexpr (form_type == dovah::form_type::quest) {
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
      return this->_cache.model_paths.value(stub.formID);
   }
   QString core::get_quest_filter(const dovah::form_stub& stub) const {
      if (stub.formType != dovah::form_type::quest)
         return {};
      return this->_cache.quest_filters.value(stub.formID);
   }

   bool core::form_has_script_attached(const dovah::form_stub& stub, std::string_view scriptname) const {
      auto it = this->_cache.attached_scripts.find(stub.formID);
      if (it == this->_cache.attached_scripts.end())
         return false;

      for (auto& known : it->deleted)
         if (known->name_matches(scriptname))
            return false;

      for (auto& known : it->attached)
         if (known->name_matches(scriptname))
            return true;

      return false;
   }
}