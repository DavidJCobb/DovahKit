#include "./location.h"
#include <limits>
#include "dovah/core.h"
#include "dovah/utils/update_location_content.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "ui/utils/bind.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

// Includes for contents list-view columns
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/forms/components/extra_data/types/e/encounter_zone.h"
#include "dovah/forms/components/extra_data/types/l/location.h"
#include "dovah/forms/components/extra_data/types/l/location_ref_type.h"
#include "dovah/forms/ActorBase.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/EncounterZone.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/forms/Worldspace.h"

namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}
namespace {
   static constexpr bool _is_valid_zone(dovah::form_stub* zone) {
      return zone && zone->form_type == dovah::form_type::encounter_zone && (zone->formID != dovah::hardcoded_form_ids::NoZoneZone);
   }
   static dovah::form_stub* _location_of_zone(dovah::form_stub& zone) {
      auto loaded = zone.load().ptr_cast<dovah::loaded_forms::EncounterZone>();
      if (!loaded)
         return nullptr;
      auto* stub = loaded->location.get_form_stub();
      if (stub && stub->form_type == dovah::form_type::location)
         return stub;
      return nullptr;
   };
   
   static const dovah::loaded_forms::ObjectReference* _as_ref(const dovah::loaded_forms::Form& loaded) {
      return dynamic_cast<const dovah::loaded_forms::ObjectReference*>(&loaded);
   }
   static dovah::form_stub* _loc_ref_type_of(const dovah::loaded_forms::ObjectReference& loaded) {
      auto* extra = loaded.extra_data.get<extra_data_types::location_ref_type>();
      if (!extra)
         return nullptr;
      auto* stub = extra->form.get_form_stub();
      if (!stub || stub->form_type != dovah::form_type::location_ref_type)
         return nullptr;
      return stub;
   }
   static dovah::form_stub* _location_of_cell(dovah::form_stub& stub) {
      if (stub.form_type != dovah::form_type::cell)
         return nullptr;

      dovah::form_stub* parent_cell  = &stub;
      dovah::form_stub* parent_world = nullptr;
      {
         parent_cell = stub.get_parent_form();
         if (!parent_cell || parent_cell->form_type != dovah::form_type::cell)
            return nullptr;
         parent_world = parent_cell->get_parent_form();
         if (parent_world && parent_world->form_type != dovah::form_type::worldspace)
            parent_world = nullptr;
      }

      dovah::loaded_form_ptr<dovah::loaded_forms::Cell>       loaded_cell;
      dovah::loaded_form_ptr<dovah::loaded_forms::Worldspace> loaded_world;
      {
         loaded_cell = parent_cell->load().ptr_cast<dovah::loaded_forms::Cell>();
         if (!loaded_cell)
            return nullptr;
      }

      constexpr auto _is_valid_location = [](dovah::form_stub* stub) {
         return stub && stub->form_type == dovah::form_type::location;
      };

      //
      // Pull whatever Location the containing Encounter Zone was tagged with.
      //
      if (auto* extra = loaded_cell->extra_data.get<extra_data_types::encounter_zone>()) {
         auto* zone = extra->form.get_form_stub();
         if (_is_valid_zone(zone))
            if (auto* stub = _location_of_zone(*zone))
               return stub;
      }
      if (parent_world) {
         loaded_world = parent_world->load().ptr_cast<dovah::loaded_forms::Worldspace>();
         if (loaded_world) {
            auto* zone = loaded_world->encounter_zone.get_form_stub();
            if (_is_valid_zone(zone))
               if (auto* stub = _location_of_zone(*zone))
                  return stub;
         }
      }

      //
      // Pull whatever Location the Cell or World were tagged with.
      //
      if (auto* extra = loaded_cell->extra_data.get<extra_data_types::location>()) {
         auto* stub = extra->form.get_form_stub();
         if (_is_valid_location(stub))
            return stub;
      }
      if (loaded_world) {
         auto* stub = loaded_world->location.get_form_stub();
         if (_is_valid_location(stub))
            return stub;
      }

      return nullptr;
   }
   static dovah::form_stub* _containing_location_of_ref(const dovah::form_stub& ref) {
      auto* cell = ref.get_parent_form();;
      if (cell && cell->form_type == dovah::form_type::cell)
         return _location_of_cell(*cell);
      return nullptr;
   }
   static dovah::form_stub* _persist_location_of(const dovah::loaded_forms::ObjectReference& loaded) {
      auto* extra = loaded.extra_data.get<extra_data_types::location>();
      if (!extra)
         return nullptr;
      auto* stub = extra->form.get_form_stub();
      if (!stub || stub->form_type != dovah::form_type::location)
         return nullptr;
      return stub;
   }

   static bool _is_unique_actor_base(dovah::form_stub& stub) {
      auto loaded = stub.load().ptr_cast<dovah::loaded_forms::ActorBase>();
      if (!loaded)
         return false;
      return (loaded->actor_flags & dovah::loaded_forms::ActorBase::actor_flag::unique) != 0;
   }
}

FormDialogLocation::FormDialogLocation(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.parent->setAllowedFormType(dovah::form_type::location);
   {
      this->_filters.exclude_self = new DKFormPickerExcludeSingleFormFilter(this);
      this->ui.parent->setCustomFilter(this->_filters.exclude_self);
   }
   this->ui.musicType->setAllowedFormType(dovah::form_type::music_type);
   this->ui.unreportedCrimeFaction->setAllowedFormType(dovah::form_type::faction);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   {
      auto* widget = this->ui.locRefTypes;
      widget->setAllowDuplicates(false);
      widget->setNamelessRefDisplayMode(DKFormListPane::NamelessRefDisplayMode::BaseFormEditorID);
      widget->setReadOnly(true);
      widget->addExtraColumn(
         tr("Loc Ref Type"),
         [](const dovah::loaded_forms::Form& loaded) -> QString {
            if (auto* ref = _as_ref(loaded))
               if (auto* stub = _loc_ref_type_of(*ref))
                  return QString::fromStdString(stub->editorID);
            return {};
         }
      );
      widget->addExtraColumn(
         tr("Loc Ref Type ID"),
         [](const dovah::loaded_forms::Form& loaded) -> QString {
            if (auto* ref = _as_ref(loaded))
               if (auto* stub = _loc_ref_type_of(*ref))
                  return tr("(%1)").arg(editor_helpers::form_id_to_string(stub->formID));
            return {};
         }
      );
   }
   this->ui.cells->setAllowDuplicates(false);
   this->ui.cells->setReadOnly(true);
   {
      auto* widget = this->ui.actors;
      widget->setAllowDuplicates(false);
      widget->setNamelessRefDisplayMode(DKFormListPane::NamelessRefDisplayMode::BaseFormEditorID);
      widget->setReadOnly(true);
      widget->addExtraColumn(
         tr("Loc Ref Type"),
         [](const dovah::loaded_forms::Form& loaded) -> QString {
            if (auto* ref = _as_ref(loaded))
               if (auto* stub = _loc_ref_type_of(*ref))
                  return QString::fromStdString(stub->editorID);
            return {};
         }
      );
      widget->addExtraColumn(
         tr("In Location"),
         [](const dovah::form_stub& stub) -> QString {
            auto* loc = _containing_location_of_ref(stub);
            if (!loc)
               return {};
            return QString::fromStdString(loc->editorID);
         }
      );
      widget->addExtraColumn(
         tr("Persists in Location"),
         [](const dovah::loaded_forms::Form& loaded) -> QString {
            if (auto* ref = _as_ref(loaded))
               if (auto* stub = _persist_location_of(*ref))
                  return QString::fromStdString(stub->editorID);
            return {};
         }
      );
   }

   this->load(); // this creates the working copy.
}
void FormDialogLocation::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_filters.exclude_self->set_exclusion(&this->form->stub);

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.parent, working.parent_location, working);
   ui::bind(this->ui.marker, working.marker, working);
   ui::bind(this->ui.markerRadius, working.radius);
   ui::bind(this->ui.horseMarker, working.horse_marker, working);
   ui::bind(this->ui.musicType, working.music, working);
   ui::bind(this->ui.unreportedCrimeFaction, working.unreported_crime_faction, working);
   ui::bind(this->ui.color, working.color);
   this->ui.keywords->pullStubs(working.keywords.forms);

   this->_update_contents_views();
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      switch (stub->form_type) {
         case dovah::form_type::actor_base:
            _on_actor_base_changed(*stub);
            break;
         case dovah::form_type::actor:
            _on_actor_changed(*stub);
            break;
         case dovah::form_type::reference:
            _on_ref_changed(*stub);
            break;
         case dovah::form_type::cell:
            _on_cell_changed(*stub);
            break;
      }

   });
}
void FormDialogLocation::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   this->ui.keywords->commitStubs(working.keywords.forms, working);
}

void FormDialogLocation::_update_contents_views() {
   auto& location_stub = this->form->stub;

   this->ui.locRefTypes->clear();
   this->ui.cells->clear();
   this->ui.actors->clear();

   dovah::utils::update_location_content updater;
   updater.gather(location_stub);

   for (dovah::form_stub* stub : updater.content.special_refs) {
      this->ui.locRefTypes->addStub(stub);
   }
   {
      for (auto& item : updater.content.exterior_cell_lists) {
         for (dovah::form_stub* cell : item.cells)
            this->ui.cells->addStub(cell);
      }
      for (auto& pair : location_stub.inbound) {
         auto* using_stub = pair.second.other;
         if (using_stub->form_type != dovah::form_type::cell)
            continue;
         if (using_stub->is_exterior_cell())
            continue;

         auto loaded = using_stub->load().ptr_cast<dovah::loaded_forms::Cell>();
         if (!loaded)
            continue;
         auto* extra = loaded->extra_data.get<extra_data_types::location>();
         if (extra && extra->form.get_form_stub() == &location_stub) {
            this->ui.cells->addStub(using_stub);

            // We want to list all unique actors present in any cell tagged with this 
            // location, even if this location is not the actors' Persist Location (i.e. 
            // even if the Actor(Base) and Location do not use each other directly).
            dovah::form_stub_helpers::for_each_child_form(
               *using_stub,
               [this, &location_stub](dovah::form_stub& child) {
                  if (auto* base = dovah::form_stub_helpers::get_base_form(child))
                     if (_is_unique_actor_base(*base))
                        this->ui.actors->addStub(&child);
               }
            );

            continue;
         }
         //
         // Unsure whether the CK would also gather a cell for display in this list if 
         // its encounter zone is tagged to the location but the cell itself is not.
         //
      }
   }

   // In addition to listing all unique actors in any cell tagged with this Location, 
   // we do also want to list unique actors who use this Location as their Persist 
   // Location.
   for (dovah::form_stub* stub : updater.content.unique_actors) {
      this->ui.actors->addStub(stub);
   }
}
void FormDialogLocation::_on_actor_base_changed(dovah::form_stub& actor_base) {
   //
   // Handle the case of an ActorBase becoming, or ceasing to be, flagged 
   // as "unique."
   //

   const auto known_unique_actors = this->ui.actors->stubs();

   bool is_unique_actor = _is_unique_actor_base(actor_base);
   bool was_unique_actor = false;
   for (const auto* actor : known_unique_actors) {
      if (dovah::form_stub_helpers::get_base_form(*actor) == &actor_base) {
         was_unique_actor = true;
         break;
      }
   }

   if (is_unique_actor == was_unique_actor) {
      return;
   } else if (is_unique_actor) {
      //
      // Comb all of our cells, to see if any refs therein were previously 
      // non-unique actors but, by virtue of a change to their ActorBase, 
      // have become unique.
      //
      for (const auto* cell : this->ui.cells->stubs()) {
         dovah::form_stub_helpers::for_each_child_form(
            *cell,
            [this, &actor_base](dovah::form_stub& child) {
               if (child.form_type != dovah::form_type::actor)
                  return;
               if (dovah::form_stub_helpers::get_base_form(child) == &actor_base)
                  this->ui.actors->addStub(&child);
            }
         );
      }
   } else {
      //
      // Strip no-longer-unique actors from our unique actors list.
      //
      for (auto* actor : known_unique_actors)
         if (dovah::form_stub_helpers::get_base_form(*actor) == &actor_base)
            this->ui.actors->removeStub(actor);
   }
}
void FormDialogLocation::_on_actor_changed(dovah::form_stub& stub) {
   //
   // We want to handle the following changes:
   // 
   //  - Base form is changed from a unique actor to a non-unique actor
   //  - Base form is changed from a non-unique actor to a unique actor
   //  - Ref is placed in or persists in this Location, when previously it did neither
   //  - Ref is no longer placed in nor persisting in this Location
   //

   const auto known_unique_actors = this->ui.actors->stubs();

   auto* base = dovah::form_stub_helpers::get_base_form(stub);
   bool  is_unique_actor = _is_unique_actor_base(*base);
   bool  was_unique_actor = false;
   for (const auto* actor : known_unique_actors) {
      if (actor == &stub) {
         was_unique_actor = true;
         break;
      }
   }
   if (is_unique_actor != was_unique_actor) {
      if (is_unique_actor) {
         this->ui.actors->addStub(&stub);
      } else {
         this->ui.actors->removeStub(&stub);
      }
   }
   
   auto loaded = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
   if (!loaded) {
      return;
   }
   bool persists_here = _persist_location_of(*loaded)     == &this->form->stub;
   bool is_in_here    = _containing_location_of_ref(stub) == &this->form->stub;
   if (!persists_here && !is_in_here) {
      this->ui.actors->removeStub(&stub);
      this->ui.locRefTypes->removeStub(&stub);
      return;
   }

   auto* loc_ref_type = _loc_ref_type_of(*loaded);
   if (loc_ref_type) {
      this->ui.locRefTypes->addStub(&stub);
   } else {
      this->ui.locRefTypes->removeStub(&stub);
   }
}
void FormDialogLocation::_on_cell_changed(dovah::form_stub& cell) {
   bool cell_was_here = this->ui.cells->stubs().indexOf(&cell) >= 0;
   bool cell_is_here  = _location_of_cell(cell) == &this->form->stub;
   if (cell_was_here && cell_is_here)
      return;
   if (cell_is_here) {
      //
      // Cell has become part of this location.
      //
      this->ui.cells->addStub(&cell);
      //
      dovah::form_stub_helpers::for_each_child_form(
         cell,
         [this](dovah::form_stub& child) {
            if (auto* base = dovah::form_stub_helpers::get_base_form(child))
               if (_is_unique_actor_base(*base))
                  this->ui.actors->addStub(&child);
            auto loaded = child.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            if (loaded)
               if (_loc_ref_type_of(*loaded))
                  this->ui.locRefTypes->addStub(&child);
         }
      );
   } else {
      //
      // Cell was part of this location, but no longer is.
      //
      this->ui.cells->removeStub(&cell);
      //
      const auto known_special_refs = this->ui.locRefTypes->stubs();
      for (auto* ref : known_special_refs) {
         if (ref->get_parent_form() != &cell)
            continue;
         this->ui.locRefTypes->removeStub(ref);
      }
      //
      const auto known_unique_actors = this->ui.actors->stubs();
      for (auto* actor : known_unique_actors) {
         if (actor->get_parent_form() != &cell)
            continue;
         auto loaded = actor->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
         if (loaded && _persist_location_of(*loaded) == &this->form->stub)
            continue;
         this->ui.actors->removeStub(actor);
      }
   }
}
void FormDialogLocation::_on_ref_changed(dovah::form_stub& ref) {
   //
   // We want to handle the following changes:
   // 
   //  - Ref begins or ceases to have a LocRefType
   //  - Ref's Persist Location is changed to or from this Location
   //  - Ref is moved into or out of this Location
   //
   auto loaded = ref.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
   if (!loaded) {
      this->ui.locRefTypes->removeStub(&ref);
      return;
   }
   auto* loc_ref_type = _loc_ref_type_of(*loaded);
   if (!loc_ref_type) {
      this->ui.locRefTypes->removeStub(&ref);
      return;
   }
   if (_persist_location_of(*loaded) != &this->form->stub) {
      this->ui.locRefTypes->removeStub(&ref);
      return;
   }
   this->ui.locRefTypes->addStub(&ref);
}