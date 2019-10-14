#include <iostream> // for testing
#include "esp/TESPlugin.h"
#include "forms/Quest.h"

const char* testPath = "C:/Program Files (x86)/Steam/steamapps/common/Skyrim/Data/Skyrim.esm";

int main() {
   TESPlugin skyrim;
   skyrim.load(testPath);
   std::cout << "Loaded Skyrim.esm." << std::endl;
   std::cout << "Author: " << skyrim.authorName << std::endl;
   std::cout << "Description: " << skyrim.description << std::endl;
   skyrim.forEachFormOfType(77, [](FormStub* stub) {
      std::cout << "[QUST:" << std::hex << stub->formID << "]" << std::endl;
      auto form_guard = stub->load();
      auto form = stub->form;
      if (form && form->formType == 77) {
         TESQuest* quest = (TESQuest*)form;
         std::cout << "[QUST:" << std::hex << stub->formID << "]" << quest->editorID.c_str() << " (" << quest->name.c_str() << ")" << std::endl;
      }
      return false;
   });
   return 0;
}