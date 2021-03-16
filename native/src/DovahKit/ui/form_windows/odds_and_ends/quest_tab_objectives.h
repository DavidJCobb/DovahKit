#pragma once
#include <cstdint>
#include <QAction>
#include <QWidget>
#include "ui_quest_tab_objectives.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/Quest.h"

class QuestTabObjectives : public QWidget {
   Q_OBJECT
   private:
      using loaded_t = dovah::loaded_forms::Quest;
   public:
      QuestTabObjectives(dovah::form_stub& s, loaded_t& q, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      void deactivate();
      void redrawTargetListSelectedItemConditions();
      void redrawTargetListSelectedItemText();
      //
   private:
      Ui::QuestTabObjectives ui;
      dovah::form_stub& stub;
      loaded_t&         form;

      struct {
         struct {
            QAction* insert = nullptr;
            QAction* remove = nullptr;
         } objective_list;
         struct {
            QAction* insert   = nullptr;
            QAction* remove   = nullptr;
            QAction* moveUp   = nullptr;
            QAction* moveDown = nullptr;
         } target_list;
      } context_menu_actions;
      
      loaded_t::Objective* _get_objective() const noexcept;
      loaded_t::Objective* _get_objective(int) const noexcept;
      loaded_t::Target* _get_target() const noexcept;
      loaded_t::Target* _get_target(int objective, int entry) const noexcept;

      void _select_objective(int) noexcept;
      void _select_target(int) noexcept;

      int _selected_objective_id() const noexcept;
      int _selected_target_index() const noexcept;

      void _redraw_objective_list();
      void _redraw_objective_settings();
      void _redraw_target_list();
      void _redraw_target_settings();

      bool _did_first_show = false;
      void showEvent(QShowEvent* event) override;
};
