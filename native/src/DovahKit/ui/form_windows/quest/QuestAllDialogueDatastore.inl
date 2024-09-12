#pragma once
#include "./QuestAllDialogueDatastore.h"

constexpr size_t QuestAllDialogueDatastore::Topic::index_of_info(const dovah::form_stub& stub) const {
   auto&  list = this->infos;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i)
      if (list[i]->stub == &stub)
         return i;
   return (size_t)-1;
}
