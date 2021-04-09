#include "_build_metatables.h"
#include "_all.h"

namespace editor_script {
   void build_all_wrapper_metatables(lua_State* L) {
      using namespace wrappers;
      //
      define_wrapper_metatable<wrapper_metatable>(L);
      define_wrapper_metatable<form>(L);
      #pragma region Common form components
         define_wrapper_metatable<papyrus_root>(L);
         papyrus_root::build_collection_metatables(L);
            define_wrapper_metatable<papyrus_script>(L);
            papyrus_script::build_collection_metatables(L);
               define_wrapper_metatable<papyrus_property>(L);
               papyrus_property::build_collection_metatables(L);
      #pragma endregion
      #pragma region Form types
         define_wrapper_metatable<formlist>(L);
            formlist::build_collection_metatables(L);
         define_wrapper_metatable<shout>(L);
            shout::build_collection_metatables(L);
            define_wrapper_metatable<shout_word>(L);
         define_wrapper_metatable<quest>(L);
            quest::build_collection_metatables(L);
            define_wrapper_metatable<quest_alias>(L);
            define_wrapper_metatable<quest_loc_alias>(L);
            define_wrapper_metatable<quest_ref_alias>(L);
         define_wrapper_metatable<topic>(L);
         define_wrapper_metatable<topic_info>(L);
            topic_info::build_collection_metatables(L);
            define_wrapper_metatable<topic_info_response>(L);
         define_wrapper_metatable<voicetype>(L);
         define_wrapper_metatable<word_of_power>(L);
      #pragma endregion
      #pragma region UI
         define_wrapper_metatable<ui::widget>(L);
            define_wrapper_metatable<ui::button>(L);
            define_wrapper_metatable<ui::formpicker>(L);
            define_wrapper_metatable<ui::progress_bar>(L);
            define_wrapper_metatable<ui::spinbox>(L);
            define_wrapper_metatable<ui::text>(L);
            define_wrapper_metatable<ui::textbox>(L);
            define_wrapper_metatable<ui::window>(L);
      #pragma endregion
   }
}