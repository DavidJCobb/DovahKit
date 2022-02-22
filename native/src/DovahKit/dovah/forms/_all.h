#pragma once
#include <concepts>
#include "helpers/class_list.h"
#include "../../../incomplete_code_warnings.h"
//
#include "Form.h"
#pragma region Forms
   #include "Activator.h"
   #include "Actor.h"
   #include "ActorBase.h"
   #include "Cell.h"
   #include "Color.h"
   #include "Container.h"
   #include "DefaultObjectManager.h"
   #include "DialogueBranch.h"
   #include "Faction.h"
   #include "Flora.h"
   #include "FormList.h"
   #include "Landscape.h"
   #include "LandTexture.h"
   #include "Light.h"
   #include "Location.h"
   #include "MagicEffect.h"
   #include "Note.h"
   #include "ObjectReference.h"
   #include "Package.h"
   #include "Quest.h"
   #include "Static.h"
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
      loaded_forms::Flora,
      loaded_forms::FormList,
      loaded_forms::Landscape,
      loaded_forms::LandTexture,
      loaded_forms::Light,
      loaded_forms::Location,
      loaded_forms::Note,
      loaded_forms::ObjectReference,
      loaded_forms::Package,
      loaded_forms::Quest,
      loaded_forms::Static,
      loaded_forms::Shout,
      loaded_forms::TextureSet,
      loaded_forms::Topic,
      loaded_forms::TopicInfo,
      loaded_forms::Voicetype,
      loaded_forms::WordOfPower,
      loaded_forms::Worldspace//,
   >;

   namespace impl {
      template<typename T> concept valid_loaded_form_class = requires {
         requires (std::is_same_v<T, loaded_forms::Form> || std::is_base_of_v<loaded_forms::Form, T>);
         { T::form_type } -> std::common_with<form_type_t>;
      };
      constexpr size_t first_invalid_form_class = all_loaded_form_types::index_of_matching([]<typename T>() { return !valid_loaded_form_class<T>; });
   }
   static_assert(
      impl::first_invalid_form_class == (size_t)-1,
      "All loaded-form classes must have a static \"form_type\" member of type form_type_t."
   );

   static_assert(
      incomplete_code_warnings::allow_compiling_despite_incomplete_forms || !all_loaded_form_types::has_matching([]<typename T>() { return std::is_base_of_v<loaded_forms::_IncompleteFormType, T>; }),
      "The backend for one or more form types is incomplete."
   );
}
