#pragma once

namespace dovah {
   class form_stub;
}
class QuestAllDialogueDatastore;

namespace SceneFormVisualEditor_impl {
   struct SceneContext {
      dovah::form_stub*           quest    = nullptr;
      dovah::form_stub*           scene    = nullptr;
      QuestAllDialogueDatastore*  dialogue = nullptr;
   };
}
