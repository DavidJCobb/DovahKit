#pragma once
#include <filesystem>
#include <QDialog>
#include "ui_save_window.h"
namespace dovah {
   enum class game;
}

class ActiveFileSaveDialog : public QDialog {
   Q_OBJECT;
   public:
      ActiveFileSaveDialog(QWidget* parent = Q_NULLPTR);

   private:
      Ui::ActiveFileSaveDialog ui;

      std::filesystem::path _get_target_filename(); // returns empty on failure

      // Change what path the backend will save the new file to. This ensures that 
      // cross-game conversions go to the right place, and is also needed for if the 
      // user never actually loaded a file and is making a new file with no masters.
      void _force_current_editor_base_path(dovah::game);

      // When converting across games, verify that the same masters exist in the 
      // destination game's Data directory.
      bool _check_cross_game_masters_exist(dovah::game);

      // These functions either pop up error messages, or pop up confirmation prompts 
      // as appropriate. They return true if the save should proceed, or false if the 
      // save should abort.
      bool _enforce_form_id_ranges(bool allow_bees, bool allow_non_esl);
      bool _enforce_cross_game_form_loss_is_deliberate(dovah::game);
      bool _enforce_esl_interiors_are_deliberate();

      void commit();
};
