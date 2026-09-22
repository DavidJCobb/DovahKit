;
; Currently, my customized install script relies on `MyAppName` being a valid folder name, 
; etc.. If you want a program name with spaces in it, you'll need to split this into two 
; variables, one of which is suitable for that purpose.
;
#define MyAppName      "DovahKit"
#define MyAppPublisher "DavidJCobb"
//#define MyAppURL "https://www.nexusmods.com/halothemasterchiefcollection/mods/192"
#define MyAppExeName   "DovahKit.exe"

; Unique identifier for this application. Installers rely on this to know whether they're 
; being asked to perform an in-place update; uninstallers rely on this to know what files 
; to remove. As such this should be unique to each separate application.
;
; To generate a new GUID, open the Inno Setup Compiler GUI, open the Tools menu, and pick 
; "Generate GUID."
#define MyAppId "{{3C9C46C5-1857-4898-B32D-49970ABEAD6F}"