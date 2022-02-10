#pragma once
#include "helpers/class_list.h"
//
#include "Form.h"
#pragma region Forms
   #include "Activator.h"
   #include "Actor.h"
   #include "Cell.h"
   #include "Color.h"
   #include "DefaultObjectManager.h"
   #include "DialogueBranch.h"
   #include "Faction.h"
   #include "FormList.h"
   #include "Landscape.h"
   #include "LandTexture.h"
   #include "Location.h"
   #include "Note.h"
   #include "ObjectReference.h"
   #include "Package.h"
   #include "Quest.h"
   #include "Shout.h"
   #include "TextureSet.h"
   #include "Topic.h"
   #include "TopicInfo.h"
   #include "Voicetype.h"
   #include "WordOfPower.h"
   #include "Worldspace.h"
#pragma endregion

namespace dovah {
   using all_loaded_form_types = cobb::class_list<
      loaded_forms::Form,
      //
      loaded_forms::Activator,
      loaded_forms::Actor,
      loaded_forms::Cell,
      loaded_forms::Color,
      loaded_forms::DefaultObjectManager,
      loaded_forms::DialogueBranch,
      loaded_forms::Faction,
      loaded_forms::FormList,
      loaded_forms::Landscape,
      loaded_forms::LandTexture,
      loaded_forms::Location,
      loaded_forms::Note,
      loaded_forms::ObjectReference,
      loaded_forms::Package,
      loaded_forms::Quest,
      loaded_forms::Shout,
      loaded_forms::TextureSet,
      loaded_forms::Topic,
      loaded_forms::TopicInfo,
      loaded_forms::Voicetype,
      loaded_forms::WordOfPower,
      loaded_forms::Worldspace//,
   >;
}
