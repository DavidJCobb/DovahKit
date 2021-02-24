#pragma once
#include <cstdint>
#include <QWidget>
#include "ui_quest_tab_stages.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/Quest.h"

class QuestTabStages : public QWidget {
   Q_OBJECT
   private:
      using loaded_t = dovah::loaded_forms::Quest;
   public:
      QuestTabStages(dovah::form_stub& s, loaded_t& q, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      void deactivate();
      //
   private:
      Ui::QuestTabStages ui;
      dovah::form_stub&  stub;
      loaded_t&          form;
      
      loaded_t::Stage*    _get_stage() const noexcept;
      loaded_t::Stage*    _get_stage(int) const noexcept;
      loaded_t::LogEntry* _get_log_entry() const noexcept;
      loaded_t::LogEntry* _get_log_entry(int stage, int entry) const noexcept;

      int _selected_stage_index() const noexcept;
      int _selected_log_entry_index() const noexcept;
      
      void _modify_stage_flag(loaded_t::Stage::flags_t, bool) const noexcept;
      void _modify_log_entry_flag(loaded_t::LogEntry::flags_t, bool) const noexcept;

      void _redraw_stage_list(); // TODO
      void _redraw_stage_settings(); // TODO
      void _redraw_entry_list(); // TODO
      void _redraw_entry_settings(); // TODO
};
