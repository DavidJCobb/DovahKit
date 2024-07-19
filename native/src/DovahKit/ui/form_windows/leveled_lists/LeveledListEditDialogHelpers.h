#pragma once
/*
   This file contains most of the code that powers the leveled list dialogs. 
   They're set up in separate Qt Designer files, so we can't use inheritance 
   to share the widget definitions; and this in turn complicates sharing the 
   setup and other code for the dialogs as a whole.

   Mark `LeveledListEditDialogHelpers` as a friend class and then call its 
   static functions, passing a reference to your dialog.
*/
#include <concepts>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QSpinBox>
#include "widgets/DKFormPicker.h"
#include "widgets/DKHeaderView.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

#include "dovah/core.h"
#include "dovah/utils/leveled_list_preview.h"
#include "widgets/widget-dialogs/DKLeveledListPreviewDialog.h"
#include "ui/models/forms/LeveledListModel.h"

namespace impl {
   template<typename Ui>
   concept exposes_flag_use_all = requires (const Ui & ui) {
      { ui.flagUseAll } -> std::convertible_to<QCheckBox*>;
   };

   template<typename Ui>
   concept exposes_flag_special_loot = requires (const Ui & ui) {
      { ui.flagSpecialLoot } -> std::convertible_to<QCheckBox*>;
   };

   template<typename Ui>
   concept has_chance_none_widgets = requires (const Ui& ui) {
      { ui.chanceNonePercentage } -> std::convertible_to<QSpinBox*>;
      { ui.chanceNoneGlobal } -> std::convertible_to<DKFormPicker*>;
   };
      
   template<typename Ui>
   concept has_coed_widgets = requires (const Ui& ui) {
      { ui.entryHealth } -> std::convertible_to<QDoubleSpinBox*>;
      { ui.entryOwnerTypeActorBase } -> std::convertible_to<QRadioButton*>;
      { ui.entryOwnerTypeFaction } -> std::convertible_to<QRadioButton*>;
      { ui.entryOwnerRank } -> std::convertible_to<QSpinBox*>;
      { ui.entryOwnerActorBase } -> std::convertible_to<DKFormPicker*>;
      { ui.entryOwnerFaction } -> std::convertible_to<DKFormPicker*>;
      { ui.entryOwnerGlobal } -> std::convertible_to<DKFormPicker*>;
   };
}

struct LeveledListEditDialogHelpers {
   public:
      template<typename Dialog>
      static void setup(Dialog& self) {
         using ui_type = std::decay_t<decltype(self.ui)>;

         LeveledListEditDialogHelpers::setup_chance_none(self);

         ui::set_range<uint16_t>(self.ui.entryCount);
         ui::set_range<uint16_t>(self.ui.entryLevel);
         if constexpr (impl::has_coed_widgets<ui_type>) {
            ui::set_unsigned_range<float>(self.ui.entryHealth);
            ui::set_range<uint16_t>(self.ui.entryOwnerRank);

            self.ui.entryOwnerActorBase->setAllowedFormType(dovah::form_type::actor_base);
            self.ui.entryOwnerFaction->setAllowedFormType(dovah::form_type::faction);
            self.ui.entryOwnerGlobal->setAllowedFormType(dovah::form_type::global);
         }
      
         LeveledListEditDialogHelpers::setup_flags(self);

         self._model = new LeveledListModel(&self);
         LeveledListEditDialogHelpers::setup_view_ui(self);
         if constexpr (!impl::has_coed_widgets<ui_type>) {
            self._model->setShowsContainerItemFields(false);
         }

         if constexpr (impl::has_coed_widgets<ui_type>) {
            QObject::connect(self.ui.entryOwnerTypeActorBase, &QRadioButton::toggled, &self, [&self](bool checked) {
               self.ui.entryOwnerGlobal->setEnabled(checked);
            });
            QObject::connect(self.ui.entryOwnerTypeFaction, &QRadioButton::toggled, &self, [&self](bool checked) {
               self.ui.entryOwnerRank->setEnabled(checked);
            });
         }

         LeveledListEditDialogHelpers::setup_preview_ui(self);
      }

      template<typename Dialog>
      static void load(Dialog& self) {
         auto& working = *self.form;

         ui::bind(self.ui.editorID, self.editor_id());

         bind_flags(self);
         bind_chance_none(self);

         {
            auto types = working.leveled_list_data.legal_form_types();
            self.ui.entryForm->setAllowedFormTypes(QList<dovah::form_type>(types.begin(), types.end()));
            self.ui.entryForm->setAllowNone(false);
         }
         self._model->importFrom(working.leveled_list_data);
      }

      template<typename Dialog>
      static void save(Dialog& self) {
         auto& working = *self.form;
         self._model->commitTo(working, working.leveled_list_data);
      }

      // Take values from the current UI and write them into the model, overwriting 
      // the currently selected leveled object.
      template<typename Dialog>
      static void write_ui_to_model(Dialog& self) {
         using ui_type = std::decay_t<decltype(self.ui)>;

         auto* view      = self.ui.view;
         auto* model     = self._model;
         auto* sel_model = view->selectionModel();

         auto rows = sel_model->selectedRows();
         if (rows.empty())
            return;
         auto row = rows[0].row();
         if (row < 0)
            return;

         auto* src = model->data(row);
         if (!src)
            return;

         LeveledListModel::LeveledObject dst = *src;
         dst.form  = self.ui.entryForm->formStub();
         dst.level = self.ui.entryLevel->value();
         dst.count = self.ui.entryCount->value();
         if constexpr (impl::has_coed_widgets<ui_type>) {
            dst.health = self.ui.entryHealth->value() / LeveledListModel::health_display_mult;
            if (self.ui.entryOwnerTypeActorBase->isChecked()) {
               dst.ownership.owner = self.ui.entryOwnerActorBase->formStub();
            } else {
               dst.ownership.owner = self.ui.entryOwnerFaction->formStub();
            }
            dst.ownership.global = self.ui.entryOwnerGlobal->formStub();
            dst.ownership.rank   = self.ui.entryOwnerRank->value();
         }

         auto qpmi = QPersistentModelIndex(rows[0]);
         model->setData(row, dst);
         //
         // In case the level was changed and the row was moved:
         //
         auto new_qmi = QModelIndex(qpmi);
         auto tl = new_qmi.siblingAtColumn(0);
         auto br = new_qmi.siblingAtColumn(self._model->columnCount() - 1);
         sel_model->select(
            QItemSelection{ tl, br },
            QItemSelectionModel::SelectionFlag::ClearAndSelect
         );
      }

      // Event filter. Used to allow us to delete a selected row when the Del key 
      // is pressed.
      template<typename Dialog>
      static bool event_filter(Dialog& self, QObject* watched, QEvent* event) {
         if (watched == self.ui.view) {
            if (event->type() == QEvent::Type::KeyPress) {
               auto* casted = (QKeyEvent*)event;
               if (casted->key() == Qt::Key::Key_Delete) {
                  auto* sel_model = self.ui.view->selectionModel();
                  auto  rows      = sel_model->selectedRows();
                  if (rows.empty())
                     return false;
                  self._model->removeRow(rows[0].row());
                  return true;
               }
            }
         }
         return false;
      }

   protected:
      template<typename Dialog>
      static void setup_chance_none(Dialog& self) {
         if constexpr (impl::has_chance_none_widgets<std::decay_t<decltype(self.ui)>>) {
            self.ui.chanceNonePercentage->setRange(0, 100);
            self.ui.chanceNoneGlobal->setAllowedFormType(dovah::form_type::global);
         }
      }

      // Sets up connections between the dialog's flag widgets, to enforce 
      // mutually-exclusive flags.
      template<typename Dialog>
      static void setup_flags(Dialog& self) {
         if constexpr (impl::exposes_flag_use_all<std::decay_t<decltype(self.ui)>>) {
            auto handler = [&self]() {
               bool allow_use_all = true;
               if (self.ui.flagCumulativeLevels->isChecked())
                  allow_use_all = false;
               else if (self.ui.flagRecalcPerCount->isChecked())
                  allow_use_all = false;

               self.ui.flagUseAll->setEnabled(allow_use_all);
            };
            QObject::connect(self.ui.flagCumulativeLevels, &QCheckBox::toggled, &self, handler);
            QObject::connect(self.ui.flagRecalcPerCount,   &QCheckBox::toggled, &self, handler);

            QObject::connect(self.ui.flagUseAll, &QCheckBox::toggled, &self, [&self](bool checked) {
               self.ui.flagCumulativeLevels->setEnabled(!checked);
               self.ui.flagRecalcPerCount->setEnabled(!checked);
            });
         }
      }

      template<typename Dialog>
      static void setup_view_ui(Dialog& self) {
         using ui_type = std::decay_t<decltype(self.ui)>;

         auto* view      = self.ui.view;
         auto* model     = self._model;
         view->setModel(model);
         auto* sel_model = view->selectionModel();

         {
            auto* header = new DKHeaderView(Qt::Horizontal, view);
            header->setFlexResizeEnabled(true);
            view->setHorizontalHeader(header);
            //
            auto metrics = QFontMetrics(view->font());
            header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
            header->setColumnFlex(LeveledListModel::Column::Level,  0, 0, metrics.boundingRect("99").width() * 1.5F + 4);
            header->setColumnFlex(LeveledListModel::Column::Count,  0, 0, metrics.boundingRect("9999").width() * 1.5F + 4);
            header->setColumnFlex(LeveledListModel::Column::Form,   2, 0);
            header->setColumnFlex(LeveledListModel::Column::Health, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
            header->setColumnFlex(LeveledListModel::Column::Owner,  1, 0);
            header->setSectionResizeMode(LeveledListModel::Column::Level,  QHeaderView::Interactive);
            header->setSectionResizeMode(LeveledListModel::Column::Count,  QHeaderView::Interactive);
            header->setSectionResizeMode(LeveledListModel::Column::Form,   QHeaderView::Interactive);
            header->setSectionResizeMode(LeveledListModel::Column::Health, QHeaderView::Interactive);
            header->setSectionResizeMode(LeveledListModel::Column::Owner,  QHeaderView::Interactive);
            header->setStretchLastSection(false);
         }
         if (auto* header = view->verticalHeader()) {
            header->setSectionResizeMode(QHeaderView::ResizeToContents);
            header->setVisible(false);
         }
         view->setSelectionBehavior(QAbstractItemView::SelectRows);
         view->setSelectionMode(QAbstractItemView::SingleSelection);
         view->setCornerButtonEnabled(false);
      
         QObject::connect(sel_model, &QItemSelectionModel::currentChanged, &self, [&self, model](const QModelIndex& current, const QModelIndex& previous) {
            auto blockers = std::array{
               QSignalBlocker(self.ui.entryForm),
               QSignalBlocker(self.ui.entryLevel),
               QSignalBlocker(self.ui.entryCount),
            };
            auto blockers_for_coed = std::array<QSignalBlocker, 7>{
               QSignalBlocker(nullptr),
               QSignalBlocker(nullptr),
               QSignalBlocker(nullptr),
               QSignalBlocker(nullptr),
               QSignalBlocker(nullptr),
               QSignalBlocker(nullptr),
               QSignalBlocker(nullptr),
            };
            if constexpr (impl::has_coed_widgets<ui_type>) {
               blockers_for_coed = {
                  QSignalBlocker(self.ui.entryHealth),
                  QSignalBlocker(self.ui.entryOwnerTypeActorBase),
                  QSignalBlocker(self.ui.entryOwnerTypeFaction),
                  QSignalBlocker(self.ui.entryOwnerActorBase),
                  QSignalBlocker(self.ui.entryOwnerFaction),
                  QSignalBlocker(self.ui.entryOwnerGlobal),
                  QSignalBlocker(self.ui.entryOwnerRank),
               };
            }

            const auto* data = model->data(current.row());
            bool enable = data != nullptr;

            self.ui.entryForm->setEnabled(enable);
            self.ui.entryLevel->setEnabled(enable);
            self.ui.entryCount->setEnabled(enable);
            if constexpr (impl::has_coed_widgets<ui_type>) {
               self.ui.entryHealth->setEnabled(enable);
               self.ui.entryOwnerGroupbox->setEnabled(enable);
            }
            if (!data) {
               return;
            }

            self.ui.entryForm->setFormStub(data->form);
            self.ui.entryLevel->setValue(data->level);
            self.ui.entryCount->setValue(data->count);

            if constexpr (impl::has_coed_widgets<ui_type>) {
               if (data->form) {
                  bool has_health = false;
                  bool has_owner = true;
                  switch (data->form->form_type) {
                     case dovah::form_type::leveled_item:
                     case dovah::form_type::leveled_character:
                     case dovah::form_type::leveled_spell:
                        has_owner = false;
                        break;
                     case dovah::form_type::armor:
                     case dovah::form_type::weapon:
                        has_health = true;
                        break;
                  }
                  self.ui.entryHealth->setEnabled(has_health);
                  self.ui.entryOwnerGroupbox->setEnabled(has_owner);
               } else {
                  self.ui.entryHealth->setEnabled(false);
                  self.ui.entryOwnerGroupbox->setEnabled(false);
               }

               self.ui.entryHealth->setValue(data->health * LeveledListModel::health_display_mult);
               {
                  auto  type = dovah::form_type::none;
                  auto* stub = data->ownership.owner;
                  if (stub)
                     type = stub->form_type;
                  //
                  switch (type) {
                     case dovah::form_type::actor_base:
                        self.ui.entryOwnerTypeActorBase->setChecked(true);
                        self.ui.entryOwnerActorBase->setFormStub(stub);
                        break;
                     case dovah::form_type::faction:
                        self.ui.entryOwnerTypeFaction->setChecked(true);
                        self.ui.entryOwnerFaction->setFormStub(stub);
                        break;
                     default:
                        self.ui.entryOwnerTypeActorBase->setChecked(true);
                        self.ui.entryOwnerActorBase->setFormStub(nullptr);
                        break;
                  }
               }
               self.ui.entryOwnerGlobal->setFormStub(data->ownership.global);
               self.ui.entryOwnerRank->setValue(data->ownership.rank);
            }
         });
         QObject::connect(self.ui.entryForm,  &DKFormPicker::formChanged,                  &self, &Dialog::_overwrite_selected_leveled_object);
         QObject::connect(self.ui.entryLevel, QOverload<int>::of(&QSpinBox::valueChanged), &self, &Dialog::_overwrite_selected_leveled_object);
         QObject::connect(self.ui.entryCount, QOverload<int>::of(&QSpinBox::valueChanged), &self, &Dialog::_overwrite_selected_leveled_object);
         if constexpr (impl::has_coed_widgets<ui_type>) {
            QObject::connect(self.ui.entryHealth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), &self, &Dialog::_overwrite_selected_leveled_object);
            QObject::connect(self.ui.entryOwnerTypeActorBase, &QRadioButton::toggled, &self, &Dialog::_overwrite_selected_leveled_object);
            QObject::connect(self.ui.entryOwnerTypeFaction, &QRadioButton::toggled, &self, &Dialog::_overwrite_selected_leveled_object);
            QObject::connect(self.ui.entryOwnerActorBase, &DKFormPicker::formChanged, &self, &Dialog::_overwrite_selected_leveled_object);
            QObject::connect(self.ui.entryOwnerFaction, &DKFormPicker::formChanged, &self, &Dialog::_overwrite_selected_leveled_object);
            QObject::connect(self.ui.entryOwnerGlobal, &DKFormPicker::formChanged, &self, &Dialog::_overwrite_selected_leveled_object);
            QObject::connect(self.ui.entryOwnerRank, QOverload<int>::of(&QSpinBox::valueChanged), &self, &Dialog::_overwrite_selected_leveled_object);
         }

         // force UI enable state updates:
         sel_model->currentChanged({}, {});

         {
            auto* menu = self._view_context = new QMenu(&self);

            auto* action_new = new QAction(QObject::tr("Add entry", "leveled list context menu"), menu);
            QObject::connect(action_new, &QAction::triggered, &self, [&self]() {
               auto at = self._model->rowCount();
               if (self._model->insertRows(at, 1)) {
                  auto  qmi       = self._model->index(at, 0, {});
                  auto* sel_model = self.ui.view->selectionModel();
                  //sel_model->setCurrentIndex(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect);

                  auto tl = qmi.siblingAtColumn(0);
                  auto br = qmi.siblingAtColumn(self._model->columnCount() - 1);
                  sel_model->select(
                     QItemSelection{ tl, br },
                     QItemSelectionModel::SelectionFlag::ClearAndSelect
                  );
               }
            });
            menu->addAction(action_new);

            auto* action_delete = new QAction(QObject::tr("Remove entry", "leveled list context menu"), menu);
            QObject::connect(action_delete, &QAction::triggered, &self, [&self]() {
               auto* sel_model = self.ui.view->selectionModel();
               auto  rows      = sel_model->selectedRows();
               if (rows.empty())
                  return;
               self._model->removeRow(rows[0].row());
            });
            menu->addAction(action_delete);

            QObject::connect(menu, &QMenu::aboutToShow, &self, [&self, action_delete]() {
               auto* sel_model = self.ui.view->selectionModel();
               auto  rows      = sel_model->selectedRows();
               if (rows.empty())
                  return;
               action_delete->setVisible(rows[0].isValid());
            });

            view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
            QObject::connect(view, &QWidget::customContextMenuRequested, &self, [&self](const QPoint& pos) {
               self._view_context->exec(self.ui.view->mapToGlobal(pos));
            });
         }

         // hook the Del key
         view->installEventFilter(&self);
      }

      template<typename Dialog>
      static void setup_preview_ui(Dialog& self) {
         using ui_type = std::decay_t<decltype(self.ui)>;

         ui::set_range<uint16_t>(self.ui.previewCount);
         ui::set_range<uint16_t>(self.ui.previewLevel);
         //
         self.ui.previewCount->setValue(1);
         self.ui.previewLevel->setValue(1);
         //
         QObject::connect(self.ui.buttonPreviewLLCalc, &QPushButton::clicked, &self, [&self]() {
            using backend_type = typename LeveledListModel::backend_type;

            auto& working = *self.form;

            // The backend has code to generate the results of a leveled list. This means that 
            // we need to actually have our data *in* the backend, though... so commit early 
            // to the working copy. It's fine -- it's just a working copy.
            self._model->commitTo(working, working.leveled_list_data);

            dovah::leveled_list_preview preview;
            preview.prepare_game_settings(self.formStub()->get_owning_load_order(), working.leveled_list_data);
            preview.input_count  = self.ui.previewCount->value();
            preview.input_level  = self.ui.previewLevel->value();
            preview.player_level = preview.input_level;
            if constexpr (impl::exposes_flag_special_loot<ui_type>) {
               preview.use_special_loot_formula = self.ui.flagSpecialLoot->isChecked();
            }
            auto results = preview.generate(working.leveled_list_data);

            auto* window = new DKLeveledListPreviewDialog(&self);
            window->setContents(results);
            window->show();
         });
      }

      // --------------------------------------------------------------------------------------------------------------

      template<typename Dialog>
      static void bind_chance_none(Dialog& self) {
         if constexpr (impl::has_chance_none_widgets<std::decay_t<decltype(self.ui)>>) {
            auto& working = *self.form;
            ui::bind(self.ui.chanceNoneGlobal,     working.leveled_list_data.chance_none.global, working);
            ui::bind(self.ui.chanceNonePercentage, working.leveled_list_data.chance_none.percentage);
         }
      }

      template<typename Dialog>
      static void bind_flags(Dialog& self) {
         using ui_type = std::decay_t<decltype(self.ui)>;

         using component_type = decltype(Dialog::loaded_form_type::leveled_list_data);
         using flag           = component_type::flag;

         auto& working = *self.form;
         auto& flags   = working.leveled_list_data.flags;

         ui::bind(self.ui.flagCumulativeLevels, flags, flag::calculate_from_all_levels_below_player);
         ui::bind(self.ui.flagRecalcPerCount,   flags, flag::calculate_for_each_item_in_count);
         if constexpr (impl::exposes_flag_use_all<ui_type>) {
            ui::bind(self.ui.flagUseAll, flags, flag::use_all);
         }
         if constexpr (impl::exposes_flag_special_loot<ui_type>) {
            ui::bind(self.ui.flagSpecialLoot, flags, flag::special_loot);
         }
      }
};