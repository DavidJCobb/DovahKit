#include <iostream> // for testing
#include "esp/TESPlugin.h"
#include "forms/Quest.h"

const char* testPath = "C:/Program Files (x86)/Steam/steamapps/common/Skyrim/Data/Skyrim.esm";

int main() {
   TESPluginFile skyrim;
   skyrim.load(testPath);
   std::cout << "Loaded Skyrim.esm." << std::endl;
   std::cout << "Author: " << skyrim.authorName << std::endl;
   std::cout << "Description: " << skyrim.description << std::endl;
   skyrim.forEachFormOfType(77, [](FormStub* stub) {
      printf("[QUST:%08X]\n", stub->formID);
      auto form_guard = stub->load();
      auto form = stub->form;
      if (form && form->formType == 77) {
         TESQuest* quest = (TESQuest*)form;
         printf("[QUST:%08X]%s (%s)\n", stub->formID, quest->editorID.c_str(), quest->name.c_str());
      }
      return false;
   });
   return 0;
}