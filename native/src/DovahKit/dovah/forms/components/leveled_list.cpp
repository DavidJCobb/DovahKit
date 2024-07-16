#include "./leveled_list.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/leveled_list/leading_coed_bleedthrough.h"
#include "../../notices/form_save_errors/by_form_component/leveled_list/too_many_entries.h"

// For previewing the leveled list's contents.
#include <ctime>
#include <cstdlib>
#include <random>
#include "../_component_access.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::leveled_list;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_component::leveled_list;
   }
}

namespace dovah::loaded_forms::components {
   bool leveled_list::allows_form_type(form_type ft) const {
      if (this->_allowed_form_types.list == nullptr)
         return true;

      for (size_t i = 0; i < this->_allowed_form_types.count; ++i)
         if (this->_allowed_form_types.list[i] == ft)
            return true;
      return false;
   }
   [[nodiscard]] std::vector<form_type> leveled_list::legal_form_types() const {
      std::vector<form_type> out;
      
      const auto& src = this->_allowed_form_types;
      if (src.list && src.count) {
         out.resize(src.count);
         for (size_t i = 0; i < src.count; ++i)
            out[i] = src.list[i];
      }
      return out;
   }

   namespace {
      bool _is_leveled_list(dovah::form_stub& stub) {
         switch (stub.form_type) {
            case dovah::form_type::leveled_character:
            case dovah::form_type::leveled_item:
            case dovah::form_type::leveled_spell:
               return true;
         }
         return false;
      }
      bool _check_chance_none(const leveled_list& subject, std::mt19937& rng) { // return true if we should generate stuff
         uint8_t percentage = subject.chance_none.percentage;
         if (subject.chance_none.global) {
            // TODO
         }
         if (percentage > 0)
            if (rng() % 100 < percentage)
               return false;
         return true;
      }

      leveled_list::generated_preview_entry _generate_preview_single_impl(
         leveled_list::selection_mode mode,
         bool process_nested_lists,
         const leveled_list& subject,
         int16_t level,
         int16_t count,
         std::mt19937& rng
      ) {
         using selection_mode = leveled_list::selection_mode;

         if (!_check_chance_none(subject, rng))
            return {};

         leveled_list::generated_preview_entry out;

         //
         // TODO: Special Loot formula here!
         //

         int min_level = level;
         int max_level = level;
         {
            bool cumulative = subject.flags & leveled_list::flag::calculate_from_all_levels_below_player;
            if (
               (cumulative && (mode == selection_mode::vary_levels_only_when_cumulative || mode == selection_mode::always_vary_levels))
               ||
               (!cumulative && mode == selection_mode::always_vary_levels)
            ) {
               uint32_t level_difference_max = 0; // TODO: this varies by form type, and is based on GMSTs
               if (level_difference_max) {
                  min_level = level - level_difference_max;
               } else {
                  min_level = -1;
               }
            }
         }

         bool   has_exceeded_cap     = false;
         bool   within_desired_range = false;
         size_t eligible_range_start = -1;
         size_t eligible_range_size  = 0;
         size_t range_basis_level    = 0;
         for (size_t i = 0; i < subject.entries.size(); ++i) {
            auto& entry = subject.entries[i];
            if (entry.level > max_level) {
               if (has_exceeded_cap || mode != selection_mode::prefer_first_above_cap) {
                  break;
               }
               //
               // We're using the selection mode wherein we prefer the first list item we 
               // see that exceeds our upper bound (i.e. Very Hard difficulty); and we are 
               // *just now seeing* the first list item to exceed that bound.
               //
               if (min_level == max_level) {
                  //
                  // If we wanted an exact level match, then clear the range of previously 
                  // seen list items (so that we disregard any exact matches we actually 
                  // did find), and start a new range at our current list item.
                  //
                  eligible_range_start = -1;
                  eligible_range_size  = 0;
                  range_basis_level    = 0;
               }
               max_level = entry.level;
               //
               // Oh, and make sure we don't do all this twice.
               //
               has_exceeded_cap = true;
               //
               // Then, fall through.
               //
            }

            if (
               (entry.level <= range_basis_level) // Condition A
               ||
               (range_basis_level && within_desired_range) // Condition B
            ) {
               //
               // Expand the range of eligible list items to include the current list item.
               //
               ++eligible_range_size;
            } else {
               //
               // If we haven't reached the minimum usable level yet (!B), then every time 
               // we see a level higher than any we've seen before (!A), we should clear the 
               // range of eligible list items, and start a new range.
               //
               if (entry.level >= min_level)
                  within_desired_range = true;
               eligible_range_start = i;
               eligible_range_size  = 0;
               range_basis_level    = entry.level;
            }
         }

         if (eligible_range_start == -1 || eligible_range_size == 0)
            return {};

         size_t i = (rng() % (eligible_range_size + 1)) + eligible_range_start;
         assert(i < subject.entries.size());
         const auto& entry = subject.entries[i];
         
         out.form  = entry.form.get_form_stub();
         out.count = entry.count;
         if (entry.item_extra_data.has_value()) {
            auto& src = entry.item_extra_data.value();
            out.health = src.health;
            out.owner  = src.ownership.get_owner();
         }
         if (process_nested_lists && out.form) {
            if (_is_leveled_list(*out.form)) {
               auto  loaded = out.form->load();
               auto* nested = component_access::get_leveled_list(loaded);
               if (nested) {
                  auto inner = _generate_preview_single_impl(mode, process_nested_lists, *nested, level, count, rng);
                  out.form  = inner.form;
                  out.count = (out.count * inner.count) & 0xFFFF;
                  if (inner.health != 1 || inner.owner) {
                     out.health = inner.health;
                     out.owner  = inner.owner;
                  }
               }
            }
         }
         return out;
      }
      void _generate_preview_impl(leveled_list::selection_mode mode, const leveled_list& subject, int16_t level, int16_t count, std::vector<leveled_list::generated_preview_entry>& out, std::mt19937& rng) {
         if (count < 0)
            return;

         if (!(subject.flags & leveled_list::flag::calculate_from_all_levels_below_player)) {
            uint32_t highest = 0;
            for (auto& entry : subject.entries)
               if (entry.level > highest)
                  highest = entry.level;
            if (highest < level)
               level = highest;
         }

         auto _generate_one_from_this = [mode, &subject, level, count, &rng, &out]() {
            auto generated = _generate_preview_single_impl(mode, false, subject, level, count, rng);
            if (generated.count <= 0 || !generated.form)
               return;
            if (_is_leveled_list(*generated.form)) {
               auto  loaded = generated.form->load();
               auto* nested = component_access::get_leveled_list(loaded);
               if (nested)
                  _generate_preview_impl(mode, *nested, level, generated.count, out, rng);
            } else {
               // TODO: validate form type of `generated.form` and skip if not valid?
               out.push_back(generated);
            }
         };

         if (subject.flags & leveled_list::flag::calculate_for_each_item_in_count) {
            //
            // Each result object should be generated independently.
            //
            if (count == 0)
               return;
            for (size_t i = 0; i < count; ++i) {
               _generate_one_from_this();
            }
         } else {
            //
            // Generate a single result object with `count` many instances. If the chosen 
            // leveled list entry(s) is a nested leveled list, multiply the count of its 
            // result by this stack frame's `count` argument.
            //
            if (subject.flags & leveled_list::flag::use_all) {
               //
               // Individually generate every single item in the leveled list as a result,
               // if "chance none" passes.
               // 
               // We're not using `_generate_one_from_this` in this branch, because we're 
               // not generating a single item nor multiple items one at a time; as such, 
               // we need to run the "Chance None" roll ourselves.
               //
               if (_check_chance_none(subject, rng)) {
                  for (auto& entry : subject.entries) {
                     auto* stub = entry.form.get_form_stub();
                     if (!stub || !entry.count)
                        continue;
                     if (_is_leveled_list(*stub)) {
                        auto  loaded = stub->load();
                        auto* nested = component_access::get_leveled_list(loaded);
                        if (nested)
                           _generate_preview_impl(mode, *nested, level, entry.count, out, rng);
                        continue;
                     }
                     out.push_back(leveled_list::generated_preview_entry{
                        .form  = stub,
                        .count = (uint32_t)entry.count,
                     });
                     if (entry.item_extra_data.has_value()) {
                        auto& src = entry.item_extra_data.value();
                        auto& dst = out.back();
                        dst.health = src.health;
                        dst.owner  = src.ownership.get_owner();
                     }
                  }
               }
            } else {
               //
               // Normal, flagless leveled list behavior: pick a single entry to generate.
               //
               _generate_one_from_this();
            }
            //
            // In the "calculate for each item in count" branch, we generate items one at 
            // a time, `count` many times. In this branch, however, we've generated all of 
            // the items at once, so now we need to multiply their counts by `count`.
            //
            for (auto& item : out) {
               item.count *= count;
            }
         }
      }
   }
   std::vector<leveled_list::generated_preview_entry> leveled_list::generate_preview(selection_mode mode, int16_t level, int16_t count) const {
      std::vector<leveled_list::generated_preview_entry> out;

      if (level < 0)
         level = 0;
      if (count < 0)
         return out;

      std::mt19937 rng;
      rng.seed(std::time(nullptr));

      _generate_preview_impl(mode, *this, level, count, out, rng);
      return out;
   }

   void leveled_list::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      std::optional<std::vector<form_type>> legal_form_types; // for error reporting; instantiate only if needed

      switch (subrecord.signature()) {
         case 'LVLD':
            subrecord.read(this->chance_none.percentage);
            break;
         case 'LVLF':
            subrecord.read(this->flags);
            break;
         case 'LVLG':
            subrecord.read(this->chance_none.global);
            break;
         case 'LLCT':
            {
               length_type reserve = 0;
               if (subrecord.read(reserve))
                  this->entries.reserve(reserve);
            }
            break;
         case 'LVLO':
            {
               struct {
                  form_reference_t form = {};
                  uint16_t         count = 0;
                  uint16_t         level = 0;
               } data;

               subrecord.read(data.level);
               subrecord.skip_bytes(2);
               subrecord.read(data.form);
               subrecord.read(data.count);
               subrecord.skip_bytes(2);

               this->entries.push_back(entry{
                  .count = data.count,
                  .level = data.level,
               });

               auto& new_entry  = this->entries.back();
               auto* entry_form = data.form.get_form_stub();
               new_entry.form.unmanaged_set(entry_form);

               // Warn on illegal form types:
               if (entry_form && !this->allows_form_type(entry_form->form_type)) {
                  if (!legal_form_types.has_value()) {
                     legal_form_types = this->legal_form_types();
                  }
                  intfc.warn_if_ref_is_wrong_type(new_entry.form, legal_form_types.value(), subrecord.signature());
               }
            }
            break;
         case 'COED':
            if (this->entries.empty()) {
               specific_load_warnings::leading_coed_bleedthrough notice(intfc.target_stub);
               intfc.log_load_warning(notice);
               //
               // The game would skip a COED if there were no bleedthrough possibility and 
               // the COED preceded any LVLO. We'll skip it, too.
               //
               break;
            }
            {
               auto& entry = this->entries.back();
               auto& dst   = entry.item_extra_data;
               if (dst.has_value()) {
                  //
                  // The game skips COEDs after the first due to how it associates them with 
                  // LVLOs. Basically, Bethesda keeps track of the last-loaded LVLO via a 
                  // static pointer, and when they see a COED, they write into that LVLO and 
                  // then clear that pointer.
                  //
               } else {
                  dst.emplace().load(subrecord, intfc);
               }
            }
            break;
      }
   }

   void leveled_list::post_load() {
      // The game sorts these in ascending order on load, and the game's leveled list logic 
      // breaks if somehow they are not sorted.
      std::sort(this->entries.begin(), this->entries.end(), [](const auto& a, const auto& b) {
         return a.level < b.level;
      });
   }

   void leveled_list::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& LVLD = record.open_next_subrecord('LVLD');
      LVLD.write(this->chance_none.percentage);
      LVLD.close();
      auto& LVLF = record.open_next_subrecord('LVLF');
      LVLF.write(this->flags);
      LVLF.close();
      record.write_formID_subrecord('LVLG', this->chance_none.global, true);
      if (!this->entries.empty()) {
         if (this->entries.size() > std::numeric_limits<length_type>::max()) {
            auto notice = specific_save_errors::too_many_entries(
               *intfc.target_stub,
               this->entries.size()
            );
            intfc.throw_save_error(notice);
         }
         auto& LLCT = record.open_next_subrecord('LLCT');
         LLCT.write((uint8_t)this->entries.size());
         LLCT.close();

         for (const auto& entry : this->entries) {
            auto& LVLO = record.open_next_subrecord('LVLO');
            LVLO.write(entry.level);
            LVLO.skip_bytes(2);
            LVLO.write(entry.form);
            LVLO.write(entry.count);
            LVLO.skip_bytes(2);
            LVLO.close();
            if (auto& opt = entry.item_extra_data; opt.has_value())
               opt.value().save(record, intfc);
         }
      }
   }
   /*static*/ void leveled_list::generate_use_info(tes_subrecord_reader& subrecord, use_info_builder& uib) {
      bool      has_seen_lvlo = false;
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'LVLD':
            break;
         case 'LVLF':
            break;
         case 'LVLG':
            subrecord.read(uib.global);
            break;
         case 'LLCT':
            break;
         case 'LVLO':
            has_seen_lvlo = true;
            subrecord.read(formID);
            uib.owner.add_outbound_reference(formID);
            break;
         case 'COED':
            if (has_seen_lvlo) {
               has_seen_lvlo = false;
               structs::container_object_extra_data::generate_use_info(subrecord, uib.owner);
            }
            break;
      }
   }
   void leveled_list::clone_from(const leveled_list& original, loaded_forms::Form& my_containing_form) noexcept {
      this->_allowed_form_types = original._allowed_form_types;

      size_t t_size = this->entries.size();
      size_t o_size = original.entries.size();
      if (t_size < o_size) {
         this->entries.resize(o_size);
      }
      for (size_t i = 0; i < o_size; ++i) {
         auto& dst = this->entries[i];
         auto& src = original.entries[i];

         dst.form.set(my_containing_form, src.form);
         dst.count = src.count;
         dst.level = src.level;
         {
            auto& src_opt = src.item_extra_data;
            auto& dst_opt = dst.item_extra_data;
            if (src_opt.has_value()) {
               if (!dst_opt.has_value())
                  dst_opt.emplace();
               dst_opt.value().clone_from(src_opt.value(), my_containing_form);
            } else {
               if (dst_opt.has_value()) {
                  dst_opt.value().clear(my_containing_form);
                  dst_opt = {};
               }
            }
         }
      }
      if (t_size > o_size) {
         for (size_t i = o_size; i < t_size; ++i) {
            auto& dst = this->entries[i];
            dst.form.set(my_containing_form, nullptr);
            if (auto& opt = dst.item_extra_data; opt.has_value()) {
               opt.value().clear(my_containing_form);
               opt = {};
            }
         }
         this->entries.resize(o_size);
      }
   }
   void leveled_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
      //
      // Also remove entries that refer to the target.
      //
      bool any_removed = false;
      for (auto& entry : this->entries) {
         if (entry.form.get_form_stub() == &target) {
            entry.form.set(my_containing_form, nullptr);
            if (auto& opt = entry.item_extra_data; opt.has_value()) {
               opt.value().clear(my_containing_form);
               opt = {};
            }
            any_removed = true;
            continue;
         }
         if (entry.form == nullptr) { // we'll end up removing these too
            if (auto& opt = entry.item_extra_data; opt.has_value()) {
               opt.value().clear(my_containing_form);
               opt = {};
            }
            any_removed = true;
            continue;
         }
         if (auto& opt = entry.item_extra_data; opt.has_value()) {
            opt.value().sever_outbound_references_to(target, my_containing_form);
         }
      }
      if (any_removed) {
         std::erase_if(
            this->entries,
            [](const entry& e) -> bool {
               return e.form == nullptr;
            }
         );
      }
   }
   void leveled_list::clear(loaded_forms::Form& my_containing_form) {
      for (auto& entry : this->entries) {
         entry.form.set(my_containing_form, nullptr);
         if (auto& opt = entry.item_extra_data; opt.has_value()) {
            opt.value().clear(my_containing_form);
            opt = {};
         }
      }
      this->entries.clear();
   }
}