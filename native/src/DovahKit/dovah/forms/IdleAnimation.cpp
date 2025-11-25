#include "IdleAnimation.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/idle/event_name_too_long.h"
#include "../notices/form_load_warnings/by_form_type/idle/filename_too_long.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::idle;
   }
}

namespace dovah::loaded_forms {
   [[nodiscard]] /*static*/ std::string IdleAnimation::correct_behavior_graph_path(std::string_view path) {
      constexpr const std::string_view extension  = ".hkx";
      constexpr const std::string_view old_folder = "animations";

      auto stristr = [](const std::string_view str, const std::string_view& needle) -> const char* {
         if (str.size() < needle.size())
            return nullptr;
         for (size_t i = 0; i < str.size() - needle.size(); ++i) {
            size_t j = 0;
            for (; j < needle.size(); ++j) {
               char a = needle[j];
               char b = str[i];
               if (b >= 'A' && b <= 'Z')
                  b += 0x20;
               if (a != b)
                  break;
            }
            if (j == needle.size())
               return &str[i];
         }
         return nullptr;
      };

      if (path.empty())
         return {};

      if (stristr(path, extension))
         return std::string(path);

      // This looks like an attempt to correct in-development animation paths to 
      // Havok behavior graphs, or something along those lines. It is expected to 
      // have the following effects:
      //
      //    foo\animations\bar.nif -> foo\Behaviors\0_master.hkx
      //
      //    bar_animations\bar.nif -> bar\Behaviors\0_master.hkx
      //
      //    japanimations\baz.nif  -> ja\Behaviors\0_master.hkx
      // 
      //    foo\bar.nif            -> foo\bar.nif\Behaviors\0_master.hkx
      // 
      //    animations\bar.nif     -> animations\bar.nif\Behaviors\0_master.hkx
      //
      // The prior games didn't use IDLE/DNAM (they used IDLE/MODL to specify a 
      // NIF with a Gamebryo animation inside), so perhaps this code was used at 
      // some transitional stage between Gamebryo idles and Haovk-based idles as 
      // we know them today.
      std::string adjusted;
      {
         const char* match = stristr(path, old_folder);
         if (match && match > path.data()) {
            adjusted = path.substr(0, match - path.data());
         } else {
            adjusted = path;
         }
      }
      adjusted += "\\Behaviors\\0_Master.hkx";
      return adjusted;
   }

   void IdleAnimation::set_behavior_graph_path(std::string_view path, bool do_corrections) {
      auto& stored = this->_hierarchy.behavior_graph;
      stored.corrected = correct_behavior_graph_path(path);
      stored.verbatim  = do_corrections ? stored.corrected : path;
      {
         auto& list = this->_as_action_root.active;
         auto  size = list.size();
         if (size >= 1) {
            if (size > 1) {
               for (size_t i = 1; i < size; ++i)
                  list[i].action.set(*this, nullptr);
               list.resize(1);
            }
            auto& item = list[0];
            item.behavior_graph_path = stored.corrected;
         }
      }
   }
   void IdleAnimation::make_action_root(std::string_view behavior_graph, form_stub& action) {
      assert(action.form_type == form_type::action);

      this->_hierarchy.behavior_graph.verbatim = behavior_graph;
      this->_hierarchy.behavior_graph.corrected = correct_behavior_graph_path(behavior_graph);

      auto& list = this->_as_action_root.active;
      auto  size = list.size();
      if (size == 0) {
         list.resize(1);
      } else if (size > 1) {
         for (size_t i = 1; i < size; ++i)
            list[i].action.set(*this, nullptr);
         list.resize(1);
      }
      auto& item = list[0];
      item.behavior_graph_path = behavior_graph;
      item.action.set(*this, &action);

      this->_hierarchy.parent.set(*this, &action);
      this->_hierarchy.previous_sibling.set(*this, nullptr);

      this->data.flags |= flag::parent;
   }
   void IdleAnimation::make_loose() {
      auto& list = this->_as_action_root.active;
      if (!list.empty()) {
         for (auto& item : list)
            item.action.set(*this, nullptr);
         list.clear();
      }

      this->_hierarchy.parent.set(*this, nullptr);
      this->_hierarchy.previous_sibling.set(*this, nullptr);
      this->data.flags &= ~flag::parent;
   }
   void IdleAnimation::make_loose(std::string_view behavior_graph) {
      this->_hierarchy.behavior_graph.verbatim  = behavior_graph;
      this->_hierarchy.behavior_graph.corrected = correct_behavior_graph_path(behavior_graph);
      this->make_loose();
   }
   void IdleAnimation::make_child(std::string_view behavior_graph, form_stub& parent_idle, form_stub* previous_sibling_idle) {
      assert(parent_idle.form_type == form_type::idle);
      assert(!previous_sibling_idle || previous_sibling_idle->form_type == form_type::idle);

      this->_hierarchy.behavior_graph.verbatim  = behavior_graph;
      this->_hierarchy.behavior_graph.corrected = correct_behavior_graph_path(behavior_graph);
      {
         auto& list = this->_as_action_root.active;
         if (!list.empty()) {
            for (auto& item : list)
               item.action.set(*this, nullptr);
            list.clear();
         }
      }
      this->_hierarchy.parent.set(*this, &parent_idle);
      this->_hierarchy.previous_sibling.set(*this, previous_sibling_idle);
      this->data.flags &= ~flag::parent;
   }

   std::vector<const IdleAnimation::action_root_candidacy*> IdleAnimation::get_candidicacies_for_action_root(std::string_view behavior_graph, form_stub& action) const {
      std::vector<const action_root_candidacy*> eligible;
      for (const auto& item : this->_as_action_root.masters) {
         if (item.behavior_graph_path != behavior_graph)
            continue;
         if (item.action != &action)
            continue;
         eligible.push_back(&item);
      }
      for (const auto& item : this->_as_action_root.active) {
         if (item.behavior_graph_path != behavior_graph)
            continue;
         if (item.action != &action)
            continue;
         eligible.push_back(&item);
      }
      return eligible;
   }
   void IdleAnimation::for_each_action_root_candidacy(std::function<void(const std::string_view, form_stub* action)> functor) {
      for (const auto& item : this->_as_action_root.masters)
         functor(item.behavior_graph_path, item.action.get_form_stub());
      for (const auto& item : this->_as_action_root.active)
         functor(item.behavior_graph_path, item.action.get_form_stub());
   }
   bool IdleAnimation::is_better_action_root_candidate_than(const IdleAnimation& other_idle, const std::string_view graph, form_stub& action) const {
      if (&other_idle == this)
         return false;

      auto this_eligible = this->get_candidicacies_for_action_root(graph, action);
      if (this_eligible.empty())
         return false;
      auto that_eligible = other_idle.get_candidicacies_for_action_root(graph, action);
      if (that_eligible.empty())
         return false;

      const auto& this_candidacy = *this_eligible.back();
      const auto& that_candidacy = *that_eligible.back();

      const auto* this_file = this_candidacy.anam_subrecord.source_file;
      const auto* that_file = that_candidacy.anam_subrecord.source_file;
      assert(this_file != nullptr);
      assert(that_file != nullptr);
      if (this_file == that_file) {
         //
         // Same file. The last-seen subrecord wins.
         //
         auto& this_anam = this_candidacy.anam_subrecord;
         auto& that_anam = that_candidacy.anam_subrecord;
         if (this_anam.offsets.of_record > that_anam.offsets.of_record)
            return true;
         if (this_anam.offsets.of_subrecord > that_anam.offsets.of_subrecord)
            return true;
         return false;
      }
      //
      // Different files. The last-loaded file wins.
      //
      auto& lo = this->stub.get_owning_load_order();
      return lo.index_of_file(*this_file) > lo.index_of_file(*that_file);
   }

   void IdleAnimation::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);

      auto _try_store_action_root_candidacy = [this, &record, &intfc](
         tes_file_reading::subrecord& subrecord,
         std::string_view graph,
         form_stub* action
      ) {
         if (!action || action->form_type != form_type::action)
            return;
         auto& item = this->_as_action_root.masters.emplace_back();
         item.behavior_graph_path = graph;
         item.action.unmanaged_set(action);
         item.anam_subrecord.source_file = intfc.current_file;
         subrecord.back_to_start(); // HACK: no accessor for subrecord start pos, so rewind to start of subrecord body and use record.current_offset()
         item.anam_subrecord.offsets = {
            .of_record    = record.header_pos(),
            .of_subrecord = record.current_offset(),
         };
      };
      
      if (!intfc.is_winning_record) {
         //
         // Bethesda didn't properly implement "Rule of One" override behavior for 
         // "action root" idles. Specifically:
         // 
         //  - Instead of waiting until after the IDLE is fully loaded, and then 
         //    checking whether it specifies an AACT as its parent, they instead 
         //    run that check immediately upon reading the IDLE/ANAM subrecord. 
         //    When the parent is indeed an action, the idle will immediately be 
         //    registered as the root idle for that action within its containing 
         //    behavior graph (identified by whatever was read from DNAM at the 
         //    time).
         // 
         //  - Bethesda implements the Rule of One by loading every subrecord, and 
         //    just having forms manually clear their data between overrides. The 
         //    "clear" function for TESIdleForm doesn't unregister the idle as an 
         //    action root.
         // 
         // As a result, an idle can potentially be the root for multiple actions, 
         // possibly across multiple behavior graphs, and even if the idle is also 
         // the child of another idle. Any ANAM subrecord that specifies an action 
         // as the parent form represents a potential "action root candidacy," no 
         // matter whether the subrecord is in the winning record or a losing one. 
         // This even extends to the hypothetical case of a malformed IDLE record 
         // having multiple ANAM subrecords.
         // 
         // Whether an IDLE is the root for a given action in a given graph thus 
         // depends on every ANAM subrecord that that IDLE has ever had; and if any 
         // two IDLEs are trying to be the root of the same action in the same 
         // graph, then the one that wins is the one whose ANAM was read later. As 
         // such, for each idle, we need to store every candidacy: graph, action, 
         // and enough information to be able to compare any two candidacies to see 
         // which was loaded later.
         //
         std::string behavior_graph;
         while (auto& subrecord = record.next_subrecord()) {
            if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
               continue;
            switch (subrecord.signature()) {
               case 'DNAM':
                  if (subrecord.read(behavior_graph)) {
                     if (behavior_graph.size() > max_filename_length)
                        behavior_graph.resize(max_filename_length);
                     behavior_graph = correct_behavior_graph_path(behavior_graph);
                  }
                  break;
               case 'ANAM':
                  {
                     form_reference_t parent;
                     if (subrecord.read(parent)) {
                        _try_store_action_root_candidacy(subrecord, behavior_graph, parent.get_form_stub());
                     }
                  }
                  break;
            }
         }
         return;
      }
      
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               break;
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
            case 'DNAM':
               {
                  auto& paths = this->_hierarchy.behavior_graph;
                  auto& path  = this->_hierarchy.behavior_graph.verbatim;
                  if (subrecord.read(paths.verbatim)) {
                     auto size = paths.verbatim.size();
                     if (size > max_filename_length) {
                        paths.corrected = paths.verbatim.substr(0, max_filename_length);
                        //
                        specific_load_warnings::filename_too_long notice(
                           this->stub,
                           size
                        );
                        intfc.log_load_warning(notice);
                     } else {
                        paths.corrected = paths.verbatim;
                     }

                     paths.corrected = correct_behavior_graph_path(paths.corrected);
                  }
               }
               break;
            case 'ENAM':
               if (subrecord.read(this->animation_event)) {
                  auto size = this->animation_event.size();
                  if (size > max_event_name_length) {
                     specific_load_warnings::event_name_too_long notice(
                        this->stub,
                        size
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;
            case 'ANAM':
               {
                  bool had_parent = false;
                  if (auto& form = this->_hierarchy.parent; subrecord.read(form)) {
                     intfc.warn_if_ref_is_wrong_type(form, std::array{ form_type::idle, form_type::action }, subrecord.signature());
                     had_parent = true;
                  }
                  if (auto& form = this->_hierarchy.previous_sibling; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::idle, subrecord.signature());

                  if (had_parent) {
                     _try_store_action_root_candidacy(subrecord, this->_hierarchy.behavior_graph.corrected, this->_hierarchy.parent.get_form_stub());
                  }
               }
               break;
            case 'DATA':
               subrecord.read(this->data.loop_time_range.min);
               subrecord.read(this->data.loop_time_range.max);
               subrecord.read(this->data.flags);
               subrecord.read(this->data.anim_group_section);
               subrecord.read(this->data.replay_delay);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void IdleAnimation::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file()) {
         auto& lo = uib.stub()->get_owning_load_order();
         while (auto& subrecord = record.next_subrecord()) {
            if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
               continue;
            switch (subrecord.signature()) {
               case 'ANAM':
                  {
                     form_id_t id;
                     if (subrecord.read(id)) {
                        form_stub* parent = lo.get_form(id);
                        if (parent && parent->form_type == form_type::action) {
                           uib.add_outbound_reference(id);
                        }
                     }
                  }
                  break;
            }
         }
         return;
      }

      form_id_t parent;
      form_id_t sibling;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'ANAM':
               subrecord.read(parent);
               subrecord.read(sibling);
               break;
         }
      }
      uib.add_outbound_reference(parent);
      uib.add_outbound_reference(sibling);
   }
   void IdleAnimation::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (IdleAnimation*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      copy->animation_event = this->animation_event;
      copy->data = this->data;
      {
         if (&copy->stub == &this->stub) {
            //
            // We're creating or committing a working copy, so we need to preserve 
            // "heritage" for action root data.
            //
            auto _copy_candidacies = [copy](
               const std::vector<action_root_candidacy>& src,
               std::vector<action_root_candidacy>& dst
            ) {
               const size_t size = src.size();
               if (!dst.empty()) {
                  for (auto& item : dst)
                     item.action.set(*copy, nullptr);
               }
               dst.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  auto& src_item = src[i];
                  auto& dst_item = dst[i];
                  dst_item.behavior_graph_path = src_item.behavior_graph_path;
                  dst_item.action.set(*copy, src_item.action);
                  dst_item.anam_subrecord      = src_item.anam_subrecord;
               }
            };
            _copy_candidacies(this->_as_action_root.masters, copy->_as_action_root.masters);
            _copy_candidacies(this->_as_action_root.active,  copy->_as_action_root.active);
         } else {
            //
            // We're duplicating this form. We should not preserve action root status: 
            // if two idles in the active file are fighting to be the root idle of the 
            // same action in the same graph, we can't know in advance which one will 
            // have a later file offset when the active file is eventually saved.
            // 
            // We can't *stop* frontends or outside systems from creating situations 
            // wherein two idles fight to be the same root, but we can at least avoid 
            // those situations being the default for common operations like cloning 
            // forms.
            // 
            // (Handling this here is messy. If the idle hierarchy were integrated into 
            // the backend as a first-class system, we could maintain that sort of data 
            // integrity better. That's something to do in Sustain Phase 1.)
            //
            bool do_not_preserve_hierarchy = false;
            if (auto* parent = this->_hierarchy.parent.get_form_stub()) {
               if (parent && parent->form_type == form_type::action)
                  do_not_preserve_hierarchy = true;
            }

            copy->_hierarchy.behavior_graph = this->_hierarchy.behavior_graph;
            if (!do_not_preserve_hierarchy) {
               copy->_hierarchy.parent.set(*copy, this->_hierarchy.parent);
               copy->_hierarchy.previous_sibling.set(*copy, this->_hierarchy.previous_sibling);
            }
         }
      }
   }
   void IdleAnimation::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      record.write_string_subrecord('DNAM', this->_hierarchy.behavior_graph.verbatim);
      record.write_string_subrecord('ENAM', this->animation_event);
      {
         const auto& active_file_candidacies = this->_as_action_root.active;
         const auto  size = active_file_candidacies.size();
         if (size > 1) {
            //
            // If we loaded a malformed IDLE record with multiple ANAM subrecords (or a 
            // malformed file with multiple IDLE records for the same form), and if the 
            // ANAMs collectively place the idle into multiple action roots, then we 
            // need to ensure we preserve that malformedness when saving. Otherwise, 
            // dealing with it during editing becomes a thousand times more complicated, 
            // and it's complicated enough as it is.
            //
            for (size_t i = 0; i < size - 1; ++i) {
               auto& item      = active_file_candidacies[i];
               auto& subrecord = record.open_next_subrecord('ANAM');
               subrecord.write(item.action);
               subrecord.write((uint32_t)0);
               subrecord.close();
            }
            auto& back = active_file_candidacies.back();
            if (back.action != this->_hierarchy.parent) { // avoid a redundant ANAM
               auto& subrecord = record.open_next_subrecord('ANAM');
               subrecord.write(back.action);
               subrecord.write((uint32_t)0);
               subrecord.close();
            }
         }
      }
      {
         auto& subrecord = record.open_next_subrecord('ANAM');
         subrecord.write(this->_hierarchy.parent);
         subrecord.write(this->_hierarchy.previous_sibling);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->data.loop_time_range.min);
         subrecord.write(this->data.loop_time_range.max);
         subrecord.write(this->data.flags);
         subrecord.write(this->data.anim_group_section);
         subrecord.write(this->data.replay_delay);
         subrecord.close();
      }
   }
   void IdleAnimation::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->conditions.clear(*this);

      this->animation_event.clear();
      {
         this->_hierarchy.behavior_graph = {};
         this->_hierarchy.parent.set(*this, nullptr);
         this->_hierarchy.previous_sibling.set(*this, nullptr);

         auto& list = this->_as_action_root.active;
         for (auto& item : list)
            item.action.set(*this, nullptr);
         list.clear();
      }

      this->data = {};
   }
   void IdleAnimation::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);

      this->_hierarchy.parent.clear_if(*this, other);
      this->_hierarchy.previous_sibling.clear_if(*this, other);
      
      // start of action-root jank
      if (other.form_type == form_type::action) {
         //
         // We can't sever an IDLE's action-root candidacy if that candidacy originates 
         // from a master file. HOWEVER, we generally shouldn't need to: we only truly 
         // delete a form stub if it's defined in the active file. Otherwise, we just 
         // flag it as deleted. If the form stub is defined in the active file, then it 
         // should never appear in the masters-side candidacies anyway: the stub doesn't 
         // exist at the time the masters are indexed; the file that defines it comes 
         // after those files.
         // 
         // In other words:
         // 
         //  - We basically only tell a form to sever references to another form as 
         //    preparation for that other form either being wholly deleted from memory, 
         //    or being flagged as "a deleted form." That's really the only notable 
         //    circumstance in which this function is called.
         // 
         //  - We only ever wholly delete a form from memory if it's defined in the 
         //    active file. (This is ignoring teardown of the entire load order, which 
         //    doesn't bother severing form-to-form references since we'll be deleting 
         //    everything anyway.)
         // 
         //  - Therefore, if the form being deleted is an AACT defined outside of the 
         //    active file, we can safely refuse to sever references to it without being 
         //    left with a dangling pointer.
         // 
         // This *feels* dangerous, though. Like, I genuinely hate this. The "sever any 
         // outbound references to this form" function is silently refusing to sever 
         // outbound references to some forms in some situations, with this refusal 
         // predicated on implementation details for wholly separate processes that 
         // happen to be the main use case for the function. Some amount of messiness is 
         // unavoidable: Bethesda's IDLE/ANAM handling fails to maintain data integrity 
         // for the global idle tree, and as a result, we have to maintain the integrity 
         // of a potentially degenerate cross-form data model; we have to be *correctly 
         // wrong*, and having this function refuse to honor its own contract is the 
         // most expedient way to do that; but this sucks. This is definitely a design 
         // issue that I'll need to see to during sustain.
         //
         if (other.get_owning_load_order().is_defined_in_active_file(other)) {
            for (auto& item : this->_as_action_root.masters) {
               assert(item.action != &other && "An IDLE form has an impossible action-root candidacy (seen in a master file, but refers to the active file) that we cannot sever.");
            }
         }
         {
            auto& list = this->_as_action_root.active;
            bool  any  = false;
            for (auto& item : list) {
               if (item.action == &other) {
                  item.action.set(*this, nullptr);
                  any = true;
               }
            }
            if (any)
               std::erase_if(list, [](auto& item) { return item.action == nullptr; });
         }
      }
      // end of action-root jank
   }
}