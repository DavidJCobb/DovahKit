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
#include "dovah/forms/components/extra_data/location.h"
#include "dovah/forms/components/extra_data/location_ref_type.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/EncounterZone.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/forms/Worldspace.h"

namespace {
   static const dovah::loaded_forms::ObjectReference* _as_ref(const dovah::loaded_forms::Form& loaded) {
      return dynamic_cast<const dovah::loaded_forms::ObjectReference*>(&loaded);;
   }
   static dovah::form_stub* _loc_ref_type_of(const dovah::loaded_forms::ObjectReference& loaded) {
      auto* extra = (extra_data::location_ref_type*) loaded.extra_data.lookup_by_type(extra_data_type::location_ref_type);
      if (!extra)
         return nullptr;
      auto* stub = extra->form.get_form_stub();
      if (!stub || stub->form_type != dovah::form_type::location_ref_type)
         return nullptr;
      return stub;
   }
   static dovah::form_stub* _containing_location_of(const dovah::form_stub& stub) {
      if (!dovah::form_type_is_reference(stub.form_type))
         return nullptr;
      
      dovah::form_stub* parent_cell  = nullptr;
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
      constexpr auto _is_valid_zone = [](dovah::form_stub* zone) {
         return zone && zone->form_type == dovah::form_type::encounter_zone && (zone->formID != dovah::hardcoded_form_ids::NoZoneZone);
      };
      auto _location_of_zone = [](dovah::form_stub& zone) -> dovah::form_stub* {
         auto loaded = zone.load().ptr_cast<dovah::loaded_forms::EncounterZone>();
         if (!loaded)
            return nullptr;
         auto* stub = loaded->location.get_form_stub();
         if (_is_valid_location(stub))
            return stub;
         return nullptr;
      };

      //
      // Pull whatever Location the containing Encounter Zone was tagged with.
      //
      if (auto* extra = (extra_data::encounter_zone*)loaded_cell->extra_data.lookup_by_type(extra_data_type::encounter_zone)) {
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
      if (auto* extra = (extra_data::location*) loaded_cell->extra_data.lookup_by_type(extra_data_type::location)) {
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
   static dovah::form_stub* _persist_location_of(const dovah::loaded_forms::ObjectReference& loaded) {
      auto* extra = (extra_data::location*) loaded.extra_data.lookup_by_type(extra_data_type::location);
      if (!extra)
         return nullptr;
      auto* stub = extra->form.get_form_stub();
      if (!stub || stub->form_type != dovah::form_type::location)
         return nullptr;
      return stub;
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
   this->ui.cells->setReadOnly(true);
   {
      auto* widget = this->ui.actors;
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
            auto* loc = _containing_location_of(stub);
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
   this->ui.name->setText(editor.convert_localized_string(working.name));
   ui::bind(this->ui.parent, working.parent_location, working);
   ui::bind(this->ui.marker, working.marker, working);
   ui::bind(this->ui.markerRadius, working.radius);
   ui::bind(this->ui.horseMarker, working.horse_marker, working);
   ui::bind(this->ui.musicType, working.music, working);
   ui::bind(this->ui.unreportedCrimeFaction, working.unreported_crime_faction, working);
   ui::bind(this->ui.color, working.color);
   this->ui.keywords->pullStubs(working.keywords.forms);

   this->_update_contents_views();
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
   
   editor.assign_localized_string(working.name, this->ui.name->text());
   this->ui.keywords->commitStubs(working.keywords.forms, working);
}

#include "dovah/forms/components/extra_data/encounter_zone.h"
#include "dovah/forms/Cell.h"
namespace {
   namespace extra_data {
      using namespace dovah::loaded_forms::components::extra;
   }
   using extra_data_type = dovah::loaded_forms::components::extra_data_type;
}
void FormDialogLocation::_update_contents_views() {
   auto& location_stub = this->form->stub;

   dovah::utils::update_location_content updater;
   updater.gather(location_stub);

   {
      auto* widget = this->ui.locRefTypes;
      widget->clear();
      for (dovah::form_stub* stub : updater.content.special_refs)
         widget->addStub(stub);
   }
   {
      auto* widget = this->ui.cells;
      widget->clear();
      for (auto& item : updater.content.exterior_cell_lists) {
         for (dovah::form_stub* cell : item.cells)
            widget->addStub(cell);
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
         auto* extra = (extra_data::location*)loaded->extra_data.lookup_by_type(extra_data_type::location);
         if (extra && extra->form.get_form_stub() == &location_stub) {
            widget->addStub(using_stub);
            continue;
         }
         //
         // Unsure whether the CK would also gather a cell for display in this list if 
         // its encounter zone is tagged to the location but the cell itself is not.
         //
      }
   }
   {
      auto* widget = this->ui.actors;
      widget->clear();
      for (dovah::form_stub* stub : updater.content.unique_actors)
         widget->addStub(stub);
      //
      // NOTE: This needs to include unique actors in child/descendant locations as well.
      //
   }
}