#pragma once
#include <cstdint>
#include <QComboBox>

namespace dovah::loaded_forms {
   class Quest;
}

extern void make_location_alias_combobox(dovah::loaded_forms::Quest&, QComboBox&);
extern void make_reference_alias_combobox(dovah::loaded_forms::Quest&, QComboBox&);
extern void set_combobox_to_alias(QComboBox&, uint32_t alias_id);

extern void make_event_data_comboboxes(dovah::loaded_forms::Quest&, QComboBox& event_code, QComboBox& event_data);
