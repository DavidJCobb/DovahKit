#include <iostream> // for testing
#include <sys/timeb.h> // for benchmarks
#include "esp/TESPlugin.h"
#include "forms/Quest.h"

const char* testPath = "C:/Program Files (x86)/Steam/steamapps/common/Skyrim/Data/Skyrim.esm";

int main() {
   TESPluginFile skyrim;
   struct timeb bench_start;
   struct timeb bench_end;
   ftime(&bench_start);
   skyrim.load(testPath);
   ftime(&bench_end);
   std::cout << "Loaded Skyrim.esm." << std::endl;
   printf("Time taken: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
   std::cout << "Author: " << skyrim.authorName << std::endl;
   std::cout << "Description: " << skyrim.description << std::endl;
   skyrim.forEachFormOfType(77, [](FormStub* stub) {
      auto form_guard = stub->load();
      auto form = stub->form;
      if (form && form->formType == 77) {
         TESQuest* quest = (TESQuest*)form;
         const char* type = TESQuest::QuestTypeToString(quest->questType);
         if (!type)
            type = "<UNKNOWN>";
         printf("[QUST:%08X]%s (%s) is a %s quest\n", stub->formID, quest->editorID.c_str(), quest->name.c_str(), type);
         if (quest->scriptData.scripts.size()) {
            quest->scriptData.forEachScript([](PapyrusScriptData::Script* script) {
               printf(" - Script: %s with %d properties\n", script->name.c_str(), script->properties.size());
               return false;
            });
         }
      } else {
         printf("[QUST:%08X] could not be loaded.\n", stub->formID);
      }
      return false;
   });
   //
   FormStubHeap::get().dump();
   FormStubHeap::get().forceFreeAll();
   FormStubHeap::get().dump();
   //
   return 0;
}