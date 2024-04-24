#pragma once
#include "./base_list_size_too_large_to_serialize_error.h"
#include "./form_type_is_unimplemented.h"
#include "./length_prefixed_string_is_too_long_to_serialize.h"
#pragma region by form component
   #include "./by_form_component/destruction/too_many_stages.h"
   #include "./by_form_component/papyrus/too_many_perk_fragments.h"
   #include "./by_form_component/papyrus/too_many_properties_on_script.h"
   #include "./by_form_component/papyrus/too_many_scene_phase_fragments.h"
   #include "./by_form_component/papyrus/too_many_scripts.h"
#pragma endregion
#pragma region by form type
   #include "./by_form_type/landscape/heightmap_contains_too_steep_a_slope.h"
   #include "./by_form_type/quest/too_many_log_entry_papyrus_fragments.h"
   #include "./by_form_type/quest/too_many_scripted_aliases.h"
#pragma endregion